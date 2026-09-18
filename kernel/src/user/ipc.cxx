#include "ipc.hxx"
#include "containers/queue.h"
#include "mem/alloc.hxx"
#include "mem/memory.h"
#include "process.hxx"
#include "schedule.h"
#include "x86_64/cpu.h"
#include "x86_64/tls.h"
#include <fluke/defs/ipc.h>
#include <fluke/defs/limits.h>

#define TRANSFER_MODE_MAX 5
#define CTX_PTR CTX_SYS_A1
#define CTX_LEN CTX_SYS_A2
#define CTX_MODE CTX_SYS_A3

struct IPC_channel {
    enum State {
        EMPTY,
        LISTENING,
        CALLING,
    };

    Queue queue;
    State state;
};

struct IPC_handle final : Handle {
    result<isize> read(void* buffer, isize len) override;
    result<isize> write(const void* data, isize len) override;
    // result<isize> seek(isize offset, int whence) override;
    error_code ipc_call(Context) override;
    void close() override;

    error_code ipc_call(int transfer_mode);
};

struct IPCimpl : IPC {
    IPC_handle handle = {};
    i8 transfer_map[TRANSFER_MODE_MAX];
    u8 channel_count;
    bool closed_call = false, closed_listen = false;
    IPC_channel channels[];

    result<IPC_channel*> transfer_channel(int transfer_mode) {
        auto channel_id = transfer_map[transfer_mode];
        if (channel_id < 0)
            return error_code(EINVAL);
        return &channels[channel_id];
    }

    usize size() {
        return sizeof(*this) + channel_count * sizeof(channels[0]);
    }
};

static inline
IPCimpl&
ipc_cast(IPC_handle* handle)
{
    return *container_of(handle, IPCimpl, handle);
}

static inline
IPCimpl&
ipc_cast(IPC* ipc)
{
    return *static_cast<IPCimpl*>(ipc);
}

static result<int>
call_transfer_mode(Context caller)
{
    int call_mode = CTX_MODE(caller);

    /* Caller may not directly use 'system' class methods - these are reserved
     * for the kernel only. For read/write, use the appropriate syscall instead
     */
    if ((call_mode & IPC_CLASS_MASK) == 0)
        return error_code(EPERM);

    if (call_mode & IPC_CALL_TXSTR) {
        auto src = (void*)CTX_PTR(caller);
        usize size = CTX_LEN(caller);
        TRY_ERRC(check_user_range(src, size));
        return size <= PATH_MAX ? IPC_TRANSFER_SMALLSTR : IPC_TRANSFER_LARGESTR;
    }
    return IPC_TRANSFER_REGISTER;
}

__attribute__((noreturn)) static void
transfer_error(error_code shame, Thread* guilty, Thread* innocent)
{
    CTX_SYS_R0(&guilty->ctx) = -int(shame);
    CTX_SYS_R0(&innocent->ctx) = -EIPCIO;
    schedule_ready(guilty);
    schedule_ready(innocent);
    schedule_or_exit();
}

static void
copy_string(Thread* source, Thread* dest, usize count)
{
    int err = memcpy_user_user_catch_fault(
        dest,
        (void*)CTX_PTR(&dest->ctx),
        source,
        (const void*)CTX_PTR(&source->ctx),
        count
    );
    if (err == 0)
        return;
    else if (err == MEMCPY_FAULT_READ)
        transfer_error(error_code(EFAULT), source, dest);
    else
        transfer_error(error_code(EFAULT), dest, source);
}

__attribute__((noreturn)) static void
call_transfer(Thread* caller, Thread* listener, int transfer_mode)
{
    assert(caller->state == FLOATING);
    assert(listener->state == FLOATING);
    assert(listener->ipc_caller == nullptr);

    auto listener_callback = CTX_SYS_A4(&listener->ctx);
    CTX_SYS_A4(&listener->ctx) = CTX_SYS_A4(&caller->ctx);
    CTX_SYS_A5(&listener->ctx) = CTX_SYS_A5(&caller->ctx);

    switch (transfer_mode) {
    case IPC_TRANSFER_REGISTER:
        CTX_SYS_A1(&listener->ctx) = CTX_SYS_A1(&caller->ctx);
        CTX_SYS_A2(&listener->ctx) = CTX_SYS_A2(&caller->ctx);
        CTX_MODE(&listener->ctx) = CTX_MODE(&caller->ctx);
        break;

    case IPC_TRANSFER_SMALLSTR:
    case IPC_TRANSFER_LARGESTR:
        {
            auto len = CTX_LEN(&caller->ctx);
            auto l_len = CTX_LEN(&listener->ctx);
            if (len > l_len) {
                if (CTX_MODE(&caller->ctx) & IPC_CALL_FRAGMENT)
                    len = l_len;
                else
                    transfer_error(error_code(E2BIG), caller, listener);
            }
            CTX_LEN(&listener->ctx) = len;
            CTX_MODE(&listener->ctx) = CTX_MODE(&caller->ctx);
            copy_string(caller, listener, len);
        }
        break;

    case IPC_TRANSFER_WRITE:
        {
            auto len = std::min(CTX_LEN(&caller->ctx), CTX_LEN(&listener->ctx));
            CTX_LEN(&listener->ctx) = len;
            CTX_MODE(&listener->ctx) =
                IPC_SYSTEM_WRITE | IPC_CALL_TXSTR | IPC_CALL_FRAGMENT;
            CTX_MODE(&caller->ctx) = 0;
            copy_string(caller, listener, len);
        }
        break;

    case IPC_TRANSFER_READ:
        CTX_LEN(&listener->ctx) = CTX_LEN(&caller->ctx);
        CTX_MODE(&listener->ctx) = IPC_SYSTEM_READ | IPC_CALL_RXSTR;
        CTX_MODE(&caller->ctx) = IPC_CALL_RXSTR;
        break;

    default:
        panic("Invalid IPC transfer mode");
    }

    caller->state = CALLING;
    listener->ipc_caller = caller;
    CTX_SYS_PC(&listener->ctx) = listener_callback;
    schedule_ready(listener);
    schedule_or_exit();
}

error_code
IPC_handle::ipc_call(int transfer_mode)
{
    auto& self = ipc_cast(this);
    if (self.closed_listen)
        return error_code(EPIPE);
    assert(not self.closed_call);

    auto& channel = *TRY(self.transfer_channel(transfer_mode));

    Thread* caller = thread_cast(cpu_context_save());
    Thread* listener;

    CTX_SYS_A0(&caller->ctx) = transfer_mode;

    switch (channel.state) {
    using enum IPC_channel::State;

    case EMPTY:
        assert(channel.queue.head == nullptr);
        channel.state = CALLING;
        caller->push_into(&channel.queue);
        schedule_or_exit();

    case LISTENING:
        listener = Thread::pop_from(&channel.queue);
        assert(listener);
        if (not channel.queue.head)
            channel.state = EMPTY;
        call_transfer(caller, listener, transfer_mode);

    case CALLING:
        assert(channel.queue.head);
        caller->push_into(&channel.queue);
        schedule_or_exit();
    }
    panic("Invalid ipc queue state");
}

error_code
IPC::listen(Context ctx)
{
    auto& self = ipc_cast(this);
    if (self.closed_call)
        return error_code(EPIPE);
    assert(not self.closed_listen);

    auto channel_id = CTX_MODE(ctx);
    if (channel_id >= self.channel_count)
        return error_code(EINVAL);

    auto& channel = self.channels[channel_id];

    Thread* caller;
    Thread* listener = thread_cast(cpu_context_save());

    switch (channel.state) {
    using enum IPC_channel::State;

    case EMPTY:
        assert(channel.queue.head == nullptr);
        channel.state = LISTENING;
        listener->push_into(&channel.queue);
        schedule_or_exit();

    case LISTENING:
        assert(channel.queue.head);
        listener->push_into(&channel.queue);
        schedule_or_exit();

    case CALLING:
        caller = Thread::pop_from(&channel.queue);
        assert(caller);
        if (not channel.queue.head)
            channel.state = EMPTY;
        call_transfer(caller, listener, CTX_SYS_A0(&caller->ctx));
    }
    panic("Invalid ipc queue state");
}

static error_code
response_error(Thread* caller, error_code shame)
{
    CTX_SYS_R0(&caller->ctx) = -EIPCIO;
    schedule_ready(caller);
    return shame;
}

static result<void>::unit
respond_register(Thread* caller, Context ctx)
{
    CTX_SYS_R0(&caller->ctx) = CTX_SYS_A0(ctx);
    CTX_SYS_R1(&caller->ctx) = CTX_SYS_A1(ctx);
    schedule_ready(caller);
    return {};
}

result<void>
IPC::respond(Context rctx)
{
    Thread* caller = std::exchange(
        thread_cast(get_tls_current_thread())->ipc_caller,
        nullptr
    );
    if (not caller)
        return error_code(ENODEV);

    assert(caller->state == CALLING);
    caller->state = FLOATING;

    if (!(CTX_MODE(rctx) & IPC_CALL_RXSTR))
        return respond_register(caller, rctx);

    // Server responds with string message

    if (!(CTX_MODE(&caller->ctx) & IPC_CALL_RXSTR))
        // Caller wasn't expecting a string
        return response_error(caller, error_code(EINVAL));

    auto len = CTX_LEN(rctx);
    auto c_len = CTX_LEN(&caller->ctx);
    if (len > c_len)
        return response_error(caller, error_code(E2BIG));

    /* We've checked the obvious errors, now try to actually copy the
     * string. We'll be switching address spaces for the copy, so save the
     * responder's state
     */
    Thread* responder = thread_cast(cpu_context_save());
    copy_string(responder, caller, len);

    CTX_SYS_R0(&caller->ctx) = CTX_SYS_A0(&responder->ctx);
    CTX_SYS_R1(&caller->ctx) = len;
    schedule_ready(caller);

    CTX_SYS_R0(&responder->ctx) = 0;
    schedule_ready(responder);
    schedule_or_exit();
}

// External entry points

result<isize>
IPC_handle::read(void*, isize)
{
    return ipc_call(IPC_TRANSFER_READ);
}

result<isize>
IPC_handle::write(const void*, isize)
{
    return ipc_call(IPC_TRANSFER_WRITE);
}

error_code
IPC_handle::ipc_call(Context ctx)
{
    int transfer_mode = TRY(call_transfer_mode(ctx));
    return ipc_call(transfer_mode);
}

void
IPC_handle::close()
{
    auto& self = ipc_cast(this);
    self.closed_call = true;

    for (int i=0; i<self.channel_count; i++) {
        auto& channel = self.channels[i];
        switch (channel.state) {
        using enum IPC_channel::State;

        case EMPTY:
            assert(channel.queue.head == nullptr);
            break;

        case LISTENING:
            assert(channel.queue.head);
            schedule_wake_all(&channel.queue, -EPIPE);
            break;

        case CALLING:
            assert(channel.queue.head);
            schedule_wake_all(&channel.queue, -EBADF);
            break;
        }
    }

    if (self.closed_listen)
        kfree(&self, self.size());
}

Handle*
IPC::handle()
{
    return &ipc_cast(this).handle;
}

void
IPC::close(Tree& tree_root)
{
    auto& self = ipc_cast(this);
    self.closed_listen = true;

    for (int i=0; i<self.channel_count; i++) {
        auto& channel = self.channels[i];
        switch (channel.state) {
        using enum IPC_channel::State;

        case EMPTY:
            assert(channel.queue.head == nullptr);
            break;

        case LISTENING:
            assert(channel.queue.head);
            schedule_wake_all(&channel.queue, -EBADF);
            break;

        case CALLING:
            assert(channel.queue.head);
            schedule_wake_all(&channel.queue, -EPIPE);
            break;
        }
    }

    tree_root.remove(&tree);

    if (self.closed_call)
        kfree(&self, self.size());
}

result<IPC*>
IPC::create(i8 transfers[], usize ntransfers)
{
    if (ntransfers >= TRANSFER_MODE_MAX)
        return error_code(EINVAL);

    i8 transfer_map[TRANSFER_MODE_MAX];
    memset(transfer_map, -1, sizeof(transfer_map));

    i8 max_channel = 0;
    for (auto n = ntransfers; n --> 0;) {
        i8 channel = transfers[n];
        if (channel < 0)
            continue;
        if (channel >= TRANSFER_MODE_MAX)
            return error_code(EINVAL);
        if (channel >= max_channel)
            max_channel = channel + 1;
        transfer_map[n] = channel;
    }

    auto size = sizeof(IPCimpl) + max_channel * sizeof(IPC_channel);
    auto ipc_ptr = TRY_ALLOC(kalloc(size));
    auto ipc = new(ipc_ptr) IPCimpl{};

    ipc->channel_count = max_channel;
    ipc->closed_call = ipc->closed_listen = false;
    memcpy(ipc->transfer_map, transfer_map, sizeof(transfer_map));
    memset(ipc->channels, 0, max_channel * sizeof(IPC_channel));
    return ipc;
}

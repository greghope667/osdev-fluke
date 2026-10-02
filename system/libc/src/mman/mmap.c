#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <fluke/fluke.h>

void*
mmap(void* hint, size_t len, int prot, int mflags, int fd, off_t off)
{
    // TODO: only support anonymous private mappings for now
    assert(off == 0);
    assert(fd == -1);

    auto p = _fluke_virtual_map(hint, len, prot, mflags);
    if ((long)p >= 0) {
        return p;
    } else {
        errno = -(long)p;
        return MAP_FAILED;
    }
}

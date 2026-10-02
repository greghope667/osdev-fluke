#undef assert

void __assert_failed(const char*, const char*, int) __attribute__((noreturn));

#ifdef NDEBUG
#define assert(_) ((void)0)
#else
#define assert(expr) ((expr) ? ((void)0) : __assert_failed(__func__, __FILE__, __LINE__))
#endif

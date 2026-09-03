#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct user_shared_object {
    const void* object;
    char name[32];
};

static_assert(sizeof(struct user_shared_object) == 40);

void user_share_init();
const struct user_shared_object* user_share_get_objects();

#ifdef __cplusplus
} //extern C
#endif

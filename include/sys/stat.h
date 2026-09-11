#pragma once

#include <fluke/types.h>
#include <time.h>                       // IWYU pragma: export

typedef __blkcnt_t      blkcnt_t;
typedef __blksize_t     blksize_t;
typedef __dev_t         dev_t;
typedef __gid_t         gid_t;
typedef __ino_t         ino_t;
typedef __mode_t        mode_t;
typedef __nlink_t       nlink_t;
typedef __off_t         off_t;
typedef __uid_t         uid_t;

struct stat {
    dev_t       st_dev;     // Device id of owning device
    ino_t       st_ino;     // Serial (inode) number
    mode_t      st_mode;    // Mode
    nlink_t     st_nlink;   // Number of hard links
    uid_t       st_uid;     // User ID
    gid_t       st_gid;     // Group ID
    //dev_t       st_rdev;
    off_t       st_size;    // Size of regular file

    struct timespec st_atim;    // Access time
    struct timespec st_mtim;    // Modification time
    struct timespec st_ctim;    // File status change

    blksize_t   st_blksize; // Preferred IO block size
    blkcnt_t    st_blocks;  // Number of allocated blocks
};

// Mode_t file mode bits

#define S_IRWXU         0700
#define S_IRUSR         0400
#define S_IWUSR         0200
#define S_IXUSR         0100
#define S_IRWXG         0070
#define S_IRGRP         0040
#define S_IWGRP         0020
#define S_IXGRP         0010
#define S_IRWXO         0007
#define S_IROTH         0004
#define S_IWOTH         0002
#define S_IXOTH         0001
#define S_ISUID        04000
#define S_ISGID        02000


#ifdef __cplusplus
extern "C" {
#endif

int     mkdir(const char* __path, mode_t __permissions);

#ifdef __cplusplus
} // extern C
#endif

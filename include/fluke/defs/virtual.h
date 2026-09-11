#pragma once

#define PROT_NONE                   0
#define PROT_READ                   (1 << 0)
#define PROT_WRITE                  (1 << 1)
#define PROT_EXEC                   (1 << 2)

#define MAP_FIXED                   (1 << 0)
#define MAP_FIXED_NOREPLACE         (1 << 1)

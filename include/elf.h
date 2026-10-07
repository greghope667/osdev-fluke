#pragma once

// https://refspecs.linuxfoundation.org/elf/gabi4+/contents.html
// https://uclibc.org/docs/elf-64-gen.pdf

#include <stdint.h>

typedef uint64_t        Elf64_Addr;
typedef uint64_t        Elf64_Off;
typedef uint16_t        Elf64_Half;
typedef uint32_t        Elf64_Word;
typedef int32_t         Elf64_Sword;
typedef uint64_t        Elf64_Xword;
typedef int64_t         Elf64_Sxword;

/* Elf Header */

typedef struct {

    uint8_t     e_ident[16];    /* ELF identification */

#define EI_NIDENT           16

#define EI_MAG0             0
#define ELFMAG0             0x7f

#define EI_MAG1             1
#define ELFMAG1             'E'

#define EI_MAG2             2
#define ELFMAG2             'L'

#define EI_MAG3             3
#define ELFMAG3             'F'

/* File Class */
#define EI_CLASS            4

#define ELFCLASS32          1
#define ELFCLASS64          2

/* Data Encodings */
#define EI_DATA             5

#define ELFDATA2LSB         1       // Little endian
#define ELFDATA2MSB         2       // Big endian

/* File Version (must be EV_CURRENT) */
#define EI_VERSION          6

#define EV_CURRENT          1

/* OS ABI */
#define EI_OSABI            7

#define ELFOSABI_NONE       0       // System V ABI, i.e. generic
#define ELFOSABI_SYSV       ELFOSABI_NONE

#define EI_ABIVERSION       8       // ABI version
#define EI_PAD              9       // Start of padding bytes

    Elf64_Half  e_type;         /* Object file type */

#define ET_NONE             0       // No file type
#define ET_REL              1       // Relocatable object file
#define ET_EXEC             2       // Executable file
#define ET_DYN              3       // Shared object file
#define ET_CORE             4       // Core file

#define ET_LOOS             0xFE00  // Environment-specific use
#define ET_HIOS             0xFEFF
#define ET_LOPROC           0xFF00  // Processor-specific use
#define ET_HIPROC           0xFFFF

    Elf64_Half  e_machine;      /* Machine type */

#define EM_NONE             0
#define EM_X86_64           62

    Elf64_Word  e_version;      /* Object file version */

#define EV_CURRENT          1

    Elf64_Addr  e_entry;        /* Entry point address */
    Elf64_Off   e_phoff;        /* Program header offset */
    Elf64_Off   e_shoff;        /* Section header offset */
    Elf64_Word  e_flags;        /* Processor-specific flags */
    Elf64_Half  e_ehsize;       /* ELF header size */
    Elf64_Half  e_phentsize;    /* Size of program header entry */
    Elf64_Half  e_phnum;        /* Number of program header entries */
    Elf64_Half  e_shentsize;    /* Size of section header entry */
    Elf64_Half  e_shnum;        /* Number of section header entries */
    Elf64_Half  e_shstrndx;     /* Section name string table index */

} Elf64_Ehdr;


/* Program Headers */

typedef struct {

    Elf64_Word  p_type;         /* Type of segment */

#define PT_NULL             0       // Unused entry
#define PT_LOAD             1       // Loadable segment
#define PT_DYNAMIC          2       // Dynamic linking table
#define PT_INTERP           3       // Program interpreter path
#define PT_NOTE             4       // Note sections
#define PT_SHLIB            5       // [reserved]
#define PT_PHDR             6       // Program header table
#define PT_TLS              7       // Thread-local storage

#define PT_LOOS         0x60000000  // Environment specific use
#define PT_HIOS         0x6fffffff
#define PT_LOPROC       0x70000000  // Processor specific use
#define PT_HIPROC       0x7fffffff

    Elf64_Word  p_flags;        /* Segment attributes */

#define PF_X                1       // Execute permission
#define PF_W                2       // Write permission
#define PF_R                4       // Read permission

    Elf64_Off   p_offset;       /* Offset in file */
    Elf64_Addr  p_vaddr;        /* Virtual address in memory */
    Elf64_Addr  p_paddr;        /* Physical address (ignored for non-embedded) */
    Elf64_Xword p_filesz;       /* Size of segment in file */
    Elf64_Xword p_memsz;        /* Size of segment in memory */
    Elf64_Xword p_align;        /* Alignment of segment */

} Elf64_Phdr;

#ifndef V8CONTAINER_LIB_H
#define V8CONTAINER_LIB_H

#include <stdint.h>

/* Не забываем, что у нас GCC */
#define STRICT_MEMORY_SIZE __attribute__((packed))

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct __stFileHeader {
    uint32_t            next_page_addr;
    uint32_t            page_size;
    uint32_t            storage_ver;
    uint32_t            reserved;          // всегда 0x00000000 ?
} STRICT_MEMORY_SIZE stFileHeader;

typedef struct __stElemAddr {
    uint32_t            elem_header_addr;
    uint32_t            elem_data_addr;
    uint32_t            fffffff;           //всегда 0x7fffffff ?
} STRICT_MEMORY_SIZE stElemAddr;

typedef struct __stBlockHeader {
    char                EOL_0D;
    char                EOL_0A;
    char                data_size_hex[8];
    char                space1;
    char                page_size_hex[8];
    char                space2;
    char                next_page_addr_hex[8];
    char                space3;
    char                EOL2_0D;
    char                EOL2_0A;
} STRICT_MEMORY_SIZE stBlockHeader;

typedef struct __stElemHeaderBegin {
    uint64_t            date_creation;
    uint64_t            date_modification;
    uint32_t            res;               // всегда 0x000000?
} STRICT_MEMORY_SIZE stElemHeaderBegin;


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // V8CONTAINER_LIB_H

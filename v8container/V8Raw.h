#ifndef V8CONTAINER_LIB_V8RAW_H
#define V8CONTAINER_LIB_V8RAW_H

#include <fstream>
#include <stdint.h>

/* Не забываем, что у нас GCC */
#define STRICT_MEMORY_SIZE __attribute__((packed))

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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

#endif // DOXYGEN_SHOULD_SKIP_THIS

namespace V8Raw {

const size_t V8_DEFAULT_PAGE_SIZE = 512;
const uint32_t V8_FF_SIGNATURE = 0x7fffffff;

class AllocationError : public std::exception {
public:
    AllocationError() throw ();
    AllocationError(size_t SizeExpected) throw ();

    virtual ~AllocationError() throw ();

    virtual const char *what() const throw ();

private:
    size_t              m_MemorySizeExpected;
    std::string         m_message;
};

uint32_t
_httoi8(const char *value);

class MemoryOutputStream {
public:
    MemoryOutputStream(char **buffer, const stBlockHeader *BlockHeader);
    MemoryOutputStream(const MemoryOutputStream &src);

    void write(const char *data, size_t size) throw ();
private:
    char       *m_buffer;
    size_t      index;
};

class MemoryInputStream {
public:
    MemoryInputStream(const char *buffer) throw();

    void        read  (char *buffer, size_t bytes_to_read);
    size_t      gcount() const;
    void        seekg (size_t off, std::ios_base::seekdir way);
private:
    const char         *m_buffer;
    size_t              index;
    size_t              last_read;
};

template<class InputStreamType, class OutputStreamType>
int ReadBlockData(InputStreamType &f, const stBlockHeader *pStartBlockHeader, OutputStreamType &out, uint32_t *data_size)
{
    uint32_t page_size, next_page_addr;
    unsigned read_in_bytes, bytes_to_read;

    if (!pStartBlockHeader)
        return 0;

    stBlockHeader  Header = *pStartBlockHeader;

    *data_size = _httoi8(pStartBlockHeader->data_size_hex);

    const size_t buf_size = 1024; // TODO: Настраиваемый размер буфера
    char *pBlockData = new char [buf_size];

    read_in_bytes = 0;
    while (read_in_bytes < *data_size) {

        page_size = _httoi8(Header.page_size_hex);
        next_page_addr = _httoi8(Header.next_page_addr_hex);

        bytes_to_read = std::min(page_size, *data_size - read_in_bytes);

        size_t read_done = 0;

        while (read_done < bytes_to_read) {
            f.read(pBlockData, std::min(buf_size, bytes_to_read - read_done));
            size_t rd = f.gcount();
            out.write(pBlockData, rd);
            read_done += rd;
        }

        read_in_bytes += bytes_to_read;

        if (next_page_addr != V8_FF_SIGNATURE) { // есть следующая страница
            f.seekg(next_page_addr, std::ios_base::beg);
            f.read((char*)&Header, sizeof(Header));
        } else
            break;
    }

    delete [] pBlockData;

    return 0;
}


template<class StreamType>
int ReadBlockData(StreamType &f, const stBlockHeader *pStartBlockHeader, char **out, uint32_t *data_size)
{
    MemoryOutputStream mout(out, pStartBlockHeader);
    return ReadBlockData(f, pStartBlockHeader, mout, data_size);
}



}

#endif // V8CONTAINER_LIB_V8RAW_H

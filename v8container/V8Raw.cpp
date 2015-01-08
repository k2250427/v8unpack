#include "V8Raw.h"

namespace V8Raw {

MemoryOutputStream::MemoryOutputStream(char **buffer, const stBlockHeader *BlockHeader)
    : m_buffer(NULL), index(0)
{
    size_t data_size = _httoi8(BlockHeader->data_size_hex);
    m_buffer = new char[data_size];
    *buffer = m_buffer;
}

MemoryOutputStream::MemoryOutputStream(const MemoryOutputStream &src)
    : m_buffer(src.m_buffer), index(src.index)
{}

void MemoryOutputStream::write(const char *data, size_t size) throw ()
{
    while (size--)
        m_buffer[index++] = *(data++);
}

MemoryInputStream::MemoryInputStream(const char *buffer) throw ()
    : m_buffer(buffer), index(0), last_read(0)
{
}

void MemoryInputStream::read(char *buffer, size_t bytes_to_read)
{
    last_read = bytes_to_read;
    while (bytes_to_read--)
        *(buffer++) = m_buffer[index++];
}

size_t MemoryInputStream::gcount() const
{
    return last_read;
}

void MemoryInputStream::seekg(size_t off, std::ios_base::seekdir way)
{
    index = off;
}

AllocationError::AllocationError() throw ()
    : m_MemorySizeExpected(0), m_message("Not enough memory!")
{
}

AllocationError::AllocationError(size_t SizeExpected) throw ()
    : m_MemorySizeExpected(SizeExpected), m_message("Not enough memory to allocate: ")
{
    m_message += SizeExpected;
}

AllocationError::~AllocationError() throw ()
{}

const char *AllocationError::what() const throw ()
{
    return m_message.c_str();
}

uint32_t
_httoi8(const char *value)
{
    uint32_t result = 0;

    const char *s = value;
    unsigned char lower_s;

    int N = 8;

    while (N--) {
        lower_s = tolower(*s);
        if (lower_s >= '0' && lower_s <= '9') {
            result <<= 4;
            result += lower_s - '0';
        } else if (lower_s >= 'a' && lower_s <= 'f') {
            result <<= 4;
            result += lower_s - 'a' + 10;
        } else
            break;
        s++;
    }

    return result;
}

int ReadBlockData(char *pFileData, const stBlockHeader *pStartBlockHeader, char **out, uint32_t *data_size)
{
    MemoryInputStream m_in(pFileData);
    MemoryOutputStream m_out(out, pStartBlockHeader);
    return ReadBlockData(m_in, pStartBlockHeader, m_out, data_size);
}

}

#include "InflateStream.h"
#include <string.h>
#include <assert.h>

namespace V8Raw {

#define CHUNK 16386

InflateStream::InflateStream(std::basic_istream<char> &source) :
    source(source), output(new char[CHUNK]), input(new char[CHUNK]), available(0), last_read(0)
{
    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = (Bytef*)input;

    int ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK) {
        /* TODO: внятное исключение */
        throw std::exception();
    }

}

InflateStream::~InflateStream()
{
    delete [] output;
    delete [] input;

    (void)inflateEnd(&strm);
}

InflateStream &
InflateStream::read(char *buffer, size_t size)
{
    size_t remain = size;
    last_read = 0;
    while (remain) {

        if (remain <= available) {
            memcpy(buffer, output, remain * sizeof(char));
            available = 0;
            last_read += remain;
            break;
        }

        UpdateBuffer();

        if (available == 0) {
            /* включить признак конца файла */
            break;
        }

    }

    return *this;
}

size_t
InflateStream::gcount() const
{
    return last_read;
}

void
InflateStream::UpdateBuffer()
{
    if (strm.avail_in == 0) {
        strm.avail_in = source.read(input, CHUNK).gcount();
        if (source.bad()) {
            (void)inflateEnd(&strm);
            /* TODO: внятное исключение */
            throw std::exception();
        }
        if (strm.avail_in == 0)
            return;
    }

    int ret;

    available = 0;

    strm.avail_out = CHUNK;
    strm.next_out = (Bytef*)output;
    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);  // state not clobbered
    switch (ret) {
    case Z_NEED_DICT:
        ret = Z_DATA_ERROR;     // and fall through
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        /* TODO: внятное исключение */
        throw std::exception();
    }
    available = CHUNK - strm.avail_out;

}

}

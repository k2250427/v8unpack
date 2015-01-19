#include "DeflateStream.h"
#include <string.h>
#include <assert.h>

namespace V8Raw {

#define CHUNK 16386

#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif


DeflateStream::DeflateStream(std::basic_istream<char> &source) :
    source(source), output(new char[CHUNK]), input(new char[CHUNK]), available(0), last_read(0)
{
    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        /* TODO: внятное исключение */
        throw std::exception();
    }

}

DeflateStream::~DeflateStream()
{
    delete [] output;
    delete [] input;

    (void)deflateEnd(&strm);
}

DeflateStream &
DeflateStream::read(char *buffer, size_t size)
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
DeflateStream::gcount() const
{
    return last_read;
}

void
DeflateStream::UpdateBuffer()
{
    if (strm.avail_in == 0) {

        strm.avail_in = source.read(input, CHUNK).gcount();

        if (source.bad()) {
            //(void)deflateEnd(&strm);
            /* TODO: внятное исключение */
            throw std::exception();
        }

        if (strm.avail_in == 0)
            return;
    }

    int ret, flush;

    available = 0;

    flush = source.eof() ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = (Bytef*)input;

    strm.avail_out = CHUNK;
    strm.next_out = (Bytef*)output;
    ret = deflate(&strm, flush);    // no bad return value
    assert(ret != Z_STREAM_ERROR);  // state not clobbered

    available = CHUNK - strm.avail_out;

}

}

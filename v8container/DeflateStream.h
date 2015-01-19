#ifndef DEFLATESTREAM_H
#define DEFLATESTREAM_H

#include <iostream>
#include <zlib.h>

namespace V8Raw {

class DeflateStream
{
public:
    DeflateStream(std::basic_istream<char> &source);
    ~DeflateStream();

    DeflateStream &read(char *buffer, size_t size);
    size_t gcount() const;

protected:
    void UpdateBuffer();
private:
    std::basic_istream<char>       &source;
    char                           *output;
    char                           *input;
    size_t                          available;
    size_t                          last_read;
    z_stream                        strm;
};

}
#endif // DEFLATESTREAM_H

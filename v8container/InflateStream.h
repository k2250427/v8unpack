#ifndef INFLATESTREAM_H
#define INFLATESTREAM_H

#include <iostream>
#include "zlib.h"


namespace V8Raw {

class InflateStream
{
public:
    InflateStream(std::basic_istream<char> &source);
    virtual ~InflateStream();

    InflateStream &read(char *buffer, size_t size);
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

#endif // INFLATESTREAM_H

#include "v8container.h"
#include "V8Raw.h"
#include <iostream>
#include <fstream>
#include <vector>


#define DEFAULT_FOPEN_MODE "rw"

struct __stV8Container {
public:
    __stV8Container(const char *filename)
        :   f(filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out),
            modified(false)
    {}

private:
    mutable std::fstream                f;
    bool                                modified;
    std::vector<stElemAddr>             AddrTable;
};

struct __stV8File {
    V8Container        *parent;
    const char         *name;
    bool                is_folder;
};


V8Container *
V8Container_OpenFile(const char *filename)
{
    V8Container *C = new V8Container(filename);
    return C;
}



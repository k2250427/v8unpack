#include "v8container.h"
#include "V8Raw.h"
#include <iostream>
#include <fstream>
#include <vector>


#define DEFAULT_FOPEN_MODE "rw"

using std::string;
using std::vector;
using std::fstream;

struct __stV8Container {
public:
    __stV8Container() throw ()
        :
            modified(false),
            m_DefaultPageSize(V8Raw::V8_DEFAULT_PAGE_SIZE),
            FreeBlock(V8Raw::V8_FF_SIGNATURE), FileListRead(false), NeedPack(true)
    {}

    virtual ~__stV8Container();

    void                    SetDefaultPageSize(uint32_t PageSize) throw ();
    uint32_t                GetDefaultPageSize() const throw ();

    void                    Open(const char *filename);
    void                    Create(const char *filename);
    void                    Close();

    const vector<V8File>    GetFiles();
    void                    SetNeedPack(bool value) { NeedPack = value; }

protected:

    void                    ReadFiles();

private:

    size_t                  ReadBlockHeader(stBlockHeader *Header)
    {
        return f.read((char*)Header, sizeof(*Header)).gcount();
    }

    mutable fstream                     f;
    bool                                modified;
    vector<stElemAddr>                  AddrTable;
    uint32_t                            m_DefaultPageSize;
    uint32_t                            FreeBlock;
    bool                                FileListRead;
    vector<V8File>                      Files;
    bool                                NeedPack;
};

struct __stV8File {
public:

    __stV8File()
        : parent(NULL) {}

    __stV8File(const __stV8File &src)
        : parent(src.parent), name(src.name), is_folder(src.is_folder), is_packed(src.is_packed)
    {}

    __stV8File(V8Container *parent, const char *data, uint32_t size)
        : parent(parent)
    {
        header = *((stElemHeaderBegin*)data);
        name.reserve(size);

        // TODO: Надо ли разбирать уникод?
        const char *c = data + sizeof(stElemHeaderBegin);
        while (*c) {
            name += *c;
            c += 2;
        }
    }

    V8Container        *parent;
    std::string         name;
    bool                is_folder;
    bool                is_packed;

    stElemHeaderBegin   header;
};

__stV8Container::~__stV8Container()
{
}

void
__stV8Container::ReadFiles()
{
    vector<stElemAddr>::const_iterator cit;
    for (cit = AddrTable.begin(); cit != AddrTable.end(); cit++) {

        char *data = NULL;
        uint32_t data_size = 0;

        f.seekg(cit->elem_header_addr);

        stBlockHeader BlockHeader;

        ReadBlockHeader(&BlockHeader);
        V8Raw::ReadBlockData(f, &BlockHeader, &data, &data_size);

        V8File file(this, data, data_size);
        Files.push_back(file);

        delete [] data;
    }

    FileListRead = true;
}

const vector<V8File>
__stV8Container::GetFiles()
{
    if (!FileListRead)
        ReadFiles();

    return Files;
}

void __stV8Container::Close()
{
    if (f.is_open())
        f.close();

    AddrTable.clear();
    Files.clear();

    modified = false;
}

void
__stV8Container::Open(const char *filename)
{
    Close();

    f.open(filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

    stFileHeader FileHeader;
    stBlockHeader BlockHeader;

    f.read((char*)&FileHeader, sizeof(FileHeader));   // TODO: throw
    f.read((char*)&BlockHeader, sizeof(BlockHeader)); // TODO: throw

    m_DefaultPageSize = FileHeader.page_size;
    FreeBlock = FileHeader.next_page_addr;

    stElemAddr *pElemsAddrs = NULL, *CurElemAddr;
    uint32_t ElemsAddrsSize;
    V8Raw::ReadBlockData(f, &BlockHeader, (char**)&pElemsAddrs, &ElemsAddrsSize);
    unsigned ElemsNum = ElemsAddrsSize / sizeof(stElemAddr), i;

    AddrTable.reserve(ElemsNum);
    Files.reserve(ElemsNum);

    for (i = 0, CurElemAddr = pElemsAddrs; i < ElemsNum; ++i, ++CurElemAddr) {
        AddrTable.push_back(*CurElemAddr);
    }

    delete [] pElemsAddrs;

    FileListRead = false;
}

void
__stV8Container::Create(const char *filename)
{
    Close();

    f.open(filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

    stFileHeader FileHeader;

    FileHeader.next_page_addr = V8Raw::V8_FF_SIGNATURE;
    FileHeader.page_size = m_DefaultPageSize;
    FileHeader.storage_ver = 0;
    FileHeader.reserved = 0;

    f.write((char *)&FileHeader, sizeof(FileHeader)).flush();

    stBlockHeader BlockHeader;

    V8Raw::PrepareBlockHeader(&BlockHeader, 0, m_DefaultPageSize, V8Raw::V8_FF_SIGNATURE);

    f.write((char *)&BlockHeader, sizeof(BlockHeader)).flush();

    unsigned i = m_DefaultPageSize;
    while (i--)
        f << '\0';

    FileListRead = true;
}

void
__stV8Container::SetDefaultPageSize(uint32_t PageSize) throw ()
{
    m_DefaultPageSize = PageSize;
}

uint32_t
__stV8Container::GetDefaultPageSize() const throw ()
{
    return m_DefaultPageSize;
}


V8Container *
V8Container_OpenFile(const char *filename)
{
    V8Container *C = new V8Container();
    C->Open(filename);

    return C;
}

V8Container *
V8Container_CreateFile(const char *filename)
{
    V8Container *C = new V8Container();
    C->Create(filename);

    return C;
}


void
V8Container_CloseFile(V8Container *Container)
{
    delete Container;
}

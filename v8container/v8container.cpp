#include "v8container.h"
#include "V8Raw.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "InflateStream.h"
#include "DeflateStream.h"


#define DEFAULT_FOPEN_MODE "rw"

using std::string;
using std::vector;
using std::fstream;

#define READ_LOCK    {}
#define READ_UNLOCK  {}
#define WRITE_LOCK   {}
#define WRITE_UNLOCK {}

struct AddresedFile;



static uint32_t
AddEmptyBlocks(std::fstream &f, unsigned count, unsigned page_size)
{
    std::fstream::pos_type ppos = f.tellp();

    f.seekp(0, std::ios_base::end);

    uint32_t first = f.tellp();
    uint32_t next = first;

    while (count--) {

        stBlockHeader Header;

        if (count)
            next += sizeof(Header) + page_size;
        else
            next = V8Raw::V8_FF_SIGNATURE;

        V8Raw::PrepareBlockHeader(&Header, 0, page_size, next);

        f.write((char*)&Header, sizeof(Header));
        for (unsigned i = 0; i < page_size; ++i)
            f << '\0';
    }

    f.seekp(ppos);
    return first;
}

static uint32_t
AddEmptySize(std::fstream &f, unsigned needed_size, unsigned page_size)
{
    unsigned needed = needed_size / page_size;
    if (needed_size % page_size)
        ++needed;
    return AddEmptyBlocks(f, needed, page_size);
}


/*! ReplaceBlockData
    перезаписывет существующие данные
    возвращает адрес первого освободившегося блока или FF-сигнатуру, если все старые блоки использованы
    в потоке out указатели G и P должны быть установлены на начало заголовка первого блока
*/

template<class InputStream>
uint32_t
ReplaceBlockData(std::fstream &f, InputStream &in, unsigned BlockDataSize)
{
    unsigned data_remains = BlockDataSize;

    while (data_remains) {

        stBlockHeader Header;

        std::fstream::pos_type ppos = f.tellp();
        f.read((char*)&Header, sizeof(Header));
        f.seekp(ppos);

        uint32_t next_addr = V8Raw::_httoi8(Header.next_page_addr_hex);
        uint32_t page_size = V8Raw::_httoi8(Header.page_size_hex);

        /* ASSERT (page_size != 0) */

        char *buffer = new char[page_size];

        if (page_size >= data_remains) {
            /* последний блок */

            in.read(buffer, data_remains);

            V8Raw::PrepareBlockHeader(&Header, data_remains, page_size, V8Raw::V8_FF_SIGNATURE);

            f.write((char*)&Header, sizeof(Header));
            f.write(buffer, data_remains);

            delete [] buffer;

            return next_addr;

        } else {

            if (next_addr == V8Raw::V8_FF_SIGNATURE) {

                /* Необходимо добавить достаточное количество */
                /* TODO: брать в расчёт свободные блоки */
                uint32_t new_pages_size = V8Raw::V8_DEFAULT_PAGE_SIZE; /* TODO: передавать размер новых страниц через параметр */

                next_addr = AddEmptySize(f, data_remains, new_pages_size);

            }

            in.read(buffer, page_size);

            V8Raw::PrepareBlockHeader(&Header, page_size, page_size, next_addr);

            f.write((char*)&Header, sizeof(Header));
            f.write(buffer, page_size);

            data_remains -= page_size;

        }

        delete [] buffer;

    }

    return V8Raw::V8_FF_SIGNATURE;
}



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

    int                     FindFile(const char *name) const;
    V8File                 *GetFile(int index) const;
    void                    SetNeedPack(bool value) { NeedPack = value; }

    void                    WriteFile(const char *filename);
    void                    RemoveFile(V8File &file);
    vector<uint32_t>        GetBlockChain(const AddresedFile &afile) const;

    void                    Flush();

protected:

    void                    ReadFiles();
    void                    UpdateRootTable();

private:

    size_t                  ReadBlockHeader(stBlockHeader *Header)
    {
        return f.read((char*)Header, sizeof(*Header)).gcount();
    }

    mutable fstream                     f;
    bool                                modified;
//    vector<stElemAddr>                  AddrTable;
    uint32_t                            m_DefaultPageSize;
    uint32_t                            FreeBlock;
    bool                                FileListRead;
//    vector<V8File>                      Files;
    vector<AddresedFile>                Files;
    bool                                NeedPack;
};

struct __stV8File {
public:

    __stV8File()
        : parent(NULL) {}

    __stV8File(const __stV8File &src)
        :
            parent(src.parent), name(src.name),
            is_folder(src.is_folder), is_packed(src.is_packed),
            header(src.header)
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
    __stV8File(const V8Container *parent, const char *filename)
        : parent(parent), name(filename), is_folder(false), is_packed(false)
    {
        header.date_creation = 0;     /* TODO: Now() / stat */
        header.date_modification = 0; /* TODO: Now() / stat */
        header.res = 0;
    }

    bool operator == (const __stV8File &src) const
    {
        return (name == src.name) && (parent == src.parent);
    }

    unsigned MakeHeader(char **data)
    {
        unsigned data_size = sizeof(header) + (name.size() + 2)*2;
        *data = new char[data_size];
        *((stElemHeaderBegin*)(*data)) = header;
        char *c = &(*data)[sizeof(header)];
        for (size_t i = 0; i < name.size(); ++i) {
            /* TODO: уникод? */
            *c++ = name[i];
            *c++ = 0;
        }
        for (int i = 0; i < 4; ++i)
            *c++ = 0;

        return data_size;
    }

    const V8Container  *parent;
    std::string         name;
    bool                is_folder;
    bool                is_packed;

    stElemHeaderBegin   header;
};


struct AddresedFile {

    boost::shared_ptr<__stV8File>       file;
    stElemAddr                          addr;

    AddresedFile(__stV8File *src)
        : file(src)
    {
        addr.elem_data_addr = V8Raw::V8_FF_SIGNATURE;
        addr.elem_header_addr = V8Raw::V8_FF_SIGNATURE;
        addr.fffffff = V8Raw::V8_FF_SIGNATURE;
    }

    bool operator == (const __stV8File &b) const
    {
        return *file == b;
    }

    bool operator == (const AddresedFile &b) const
    {
        return *file == *b.file;
    }

    bool is_new() const
    {
        return addr.elem_header_addr == V8Raw::V8_FF_SIGNATURE;
    }

};


__stV8Container::~__stV8Container()
{
}

void __stV8Container::Close()
{
    WRITE_LOCK ;

    if (f.is_open())
        f.close();

    Files.clear();

    modified = false;

    WRITE_UNLOCK ;
}

void
__stV8Container::UpdateRootTable()
{

    f.seekp(sizeof(stFileHeader));
    f.seekg(f.tellp());

    vector<stElemAddr> table;
    table.reserve(Files.size());

    vector<AddresedFile>::const_iterator cit;
    for (cit = Files.begin(); cit != Files.end(); ++cit) {
        table.push_back(cit->addr);
    }

    V8Raw::MemoryInputStream m_in((char*)table.data());
    ReplaceBlockData(f, m_in, sizeof(stElemAddr) * table.size());
}

void
__stV8Container::Open(const char *filename)
{
    Close();

    READ_LOCK ;

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

    Files.reserve(ElemsNum);

    for (i = 0, CurElemAddr = pElemsAddrs; i < ElemsNum; ++i, ++CurElemAddr) {

        char *data = NULL;
        uint32_t data_size = 0;

        f.seekg(CurElemAddr->elem_header_addr);

        stBlockHeader BlockHeader;

        ReadBlockHeader(&BlockHeader);
        V8Raw::ReadBlockData(f, &BlockHeader, &data, &data_size);

        AddresedFile afile(new __stV8File(this, data, data_size));
        afile.addr = *CurElemAddr;

        Files.push_back(afile);

        delete [] data;
    }

    delete [] pElemsAddrs;

    FileListRead = false;

    READ_UNLOCK ;
}

void
__stV8Container::Create(const char *filename)
{
    Close();

    WRITE_LOCK ;

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

    WRITE_UNLOCK ;
}

int
__stV8Container::FindFile(const char *name) const
{
    vector<AddresedFile>::const_iterator cit = std::find(
                                    Files.begin(),
                                    Files.end(),
                                    __stV8File(this, name)
    );

    if (cit == Files.end())
        return -1;

    return cit - Files.begin();

}

vector<uint32_t>
__stV8Container::GetBlockChain(const AddresedFile &afile) const
{
    vector<uint32_t> R;

    if (afile.addr.elem_data_addr == V8Raw::V8_FF_SIGNATURE) {
        /* новый файл, нет цепочки блоков */
        return R;
    }

    return R;
}

void
__stV8Container::WriteFile(const char *filename)
{
    WRITE_LOCK ;

    int index = FindFile(filename);
    if (index == -1) {
        index = Files.size();
        Files.push_back(new __stV8File(this, filename));
    }

    AddresedFile &afile = Files[index];
    char *header;
    unsigned header_size = afile.file->MakeHeader(&header);

    std::ifstream fin(filename, std::ios_base::binary);

    if (!fin) {
        /* Ошибка доступа к файлу */
        throw std::exception();
    }

    fin.seekg(0, std::ios_base::end);
    size_t data_size = fin.tellg();
    fin.seekg(0);

    if (afile.is_new()) {
        afile.addr.elem_header_addr = AddEmptySize(f, header_size, m_DefaultPageSize);
        afile.addr.elem_data_addr = AddEmptySize(f, data_size, m_DefaultPageSize);
    }

    f.seekg(afile.addr.elem_header_addr);
    f.seekp(f.tellg());

    {
        V8Raw::MemoryInputStream m_in(header);
        ReplaceBlockData(f, m_in, header_size);
    }

    delete [] header;


    #define BUFFER_SIZE 4096

    f.seekg(afile.addr.elem_data_addr);
    f.seekp(f.tellg());

    if (NeedPack) {

        V8Raw::DeflateStream def(fin);
        ReplaceBlockData(f, def, data_size);

    } else {
        ReplaceBlockData(f, fin, data_size);
    }

    UpdateRootTable();

    WRITE_UNLOCK ;
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

V8File *
__stV8Container::GetFile(int index) const
{
    return Files.at(index).file.get();
}

void
__stV8Container::RemoveFile(V8File &file)
{
    vector<AddresedFile>::iterator fit = std::find(Files.begin(), Files.end(), file);
    if (fit == Files.end()) {
        /* TODO: throw */
        return;
    }

    AddresedFile &afile = *fit;

    if (afile.is_new()) {
        Files.erase(fit);
        return;
    }

    /* TODO: помѣтить свободныя страницы */

    Files.erase(fit);

    UpdateRootTable();
}

void
__stV8Container::Flush()
{
}

V8Container *
V8Container_OpenFile(const char *filename, bool inflated)
{
    V8Container *C = new V8Container();
    C->Open(filename);
    C->SetNeedPack(inflated);

    return C;
}

V8Container *
V8Container_CreateFile(const char *filename, bool inflated)
{
    V8Container *C = new V8Container();
    C->Create(filename);
    C->SetNeedPack(inflated);

    return C;
}


void
V8Container_CloseFile(V8Container *Container)
{
    Container->Flush();
    delete Container;
}


V8File *
V8Container_AddFile(V8Container *Container, const char *filename)
{
    Container->WriteFile(filename);
    int index = Container->FindFile(filename);
    V8File *res = Container->GetFile(index);
    return res;
}

void
V8Container_RemoveFile(V8Container *Container, V8File *File)
{
    Container->RemoveFile(*File);
}


void
V8Container_SetDefaultPageSize(V8Container *Container, unsigned PageSize)
{
    Container->SetDefaultPageSize(PageSize);
}

void
V8Container_Flush(V8Container *Container)
{
    Container->Flush();
}

V8File *
V8Container_FindFile(const V8Container *Container, const char *name)
{
    int index = Container->FindFile(name);
    return Container->GetFile(index);
}

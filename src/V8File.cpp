/////////////////////////////////////////////////////////////////////////////
//
//
//	Author:			disa_da
//	E-mail:			disa_da2@mail.ru
//
//
/////////////////////////////////////////////////////////////////////////////

/**
    2014-2015       dmpas       sergey(dot)batanov(at)dmpas(dot)ru
 */

// V8File.cpp: implementation of the CV8File class.
//
//////////////////////////////////////////////////////////////////////


#include "V8File.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "v8container/V8Raw.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CV8File::CV8File()
{
    IsDataPacked = true;
}


CV8File::CV8File(const CV8File &src)
    : FileHeader(src.FileHeader), IsDataPacked(src.IsDataPacked)
{
    ElemsAddrs.assign(src.ElemsAddrs.begin(), src.ElemsAddrs.end());
    Elems.assign(src.Elems.begin(), src.Elems.end());
}

/*
CV8File::CV8File(char *pFileData, bool boolInflate)
{
    LoadFile(pFileData, boolInflate);
}
*/
CV8File::~CV8File()
{
}

CV8Elem::CV8Elem(const CV8Elem &src)
    : pHeader(src.pHeader), HeaderSize(src.HeaderSize),
      pData(src.pData), DataSize(src.DataSize),
      UnpackedData(src.UnpackedData), IsV8File(src.IsV8File),
      NeedUnpack(src.NeedUnpack)
{ }

CV8Elem::CV8Elem()
    : pHeader(NULL), HeaderSize(0),
      pData(NULL), DataSize(0),
      IsV8File(false),
      NeedUnpack(false)
{
}

CV8Elem::~CV8Elem()
{
    // TODO: Добавить удаление данных
}

void CV8Elem::Free()
{
    if (pHeader) {
        delete [] pHeader;
        pHeader = NULL;
    }

    if (pData) {
        delete [] pData;
        pData = NULL;
    }
}

int CV8File::Inflate(const std::string &in_filename, const std::string &out_filename)
{
    int ret;

    boost::filesystem::path inf(in_filename);
    boost::filesystem::ifstream in_file(inf);

    if (!in_file)
        return V8UNPACK_INFLATE_IN_FILE_NOT_FOUND;

    boost::filesystem::path ouf(out_filename);
    boost::filesystem::ofstream out_file(ouf);

    if (!out_file)
        return V8UNPACK_INFLATE_OUT_FILE_NOT_CREATED;

    ret = Inflate(in_file, out_file);

    if (ret == Z_DATA_ERROR)
        return V8UNPACK_INFLATE_DATAERROR;
    if (ret)
        return V8UNPACK_INFLATE_ERROR;

    return 0;
}

int CV8File::Deflate(const std::string &in_filename, const std::string &out_filename)
{

    int ret;

    boost::filesystem::path inf(in_filename);
    boost::filesystem::ifstream in_file(inf, std::ios_base::binary);

    if (!in_file)
        return V8UNPACK_DEFLATE_IN_FILE_NOT_FOUND;

    boost::filesystem::path ouf(out_filename);
    boost::filesystem::ofstream out_file(ouf);

    if (!out_file)
        return V8UNPACK_DEFLATE_OUT_FILE_NOT_CREATED;

    ret = Deflate(in_file, out_file);

    if (ret)
        return V8UNPACK_DEFLATE_ERROR;

    return 0;
}

int CV8File::Deflate(std::basic_ifstream<char> &source, std::basic_ofstream<char> &dest)
{

    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate deflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
        return ret;

    // compress until end of file
    do {
        strm.avail_in = source.read(reinterpret_cast<char *>(in), CHUNK).gcount();
        if (source.bad()) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }

        flush = source.eof() ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        // run deflate() on input until output buffer not full, finish
        //   compression if all of source has been read in
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    // no bad return value
            assert(ret != Z_STREAM_ERROR);  // state not clobbered
            have = CHUNK - strm.avail_out;

            dest.write(reinterpret_cast<char *>(out), have);

            if (dest.bad()) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     // all input will be used

        // done when last data in file processed
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        // stream will be complete

    // clean up and return
    (void)deflateEnd(&strm);
    return Z_OK;

}
int CV8File::Inflate(std::basic_ifstream<char> &source, std::basic_ofstream<char> &dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK)
        return ret;

    do {
        strm.avail_in = source.read(reinterpret_cast<char *>(in), CHUNK).gcount();
        if (source.bad()) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;

        strm.next_in = in;

        // run inflate() on input until output buffer not full
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  // state not clobbered
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     // and fall through
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            dest.write(reinterpret_cast<char *>(out), have);
            if (dest.bad()) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        // done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // clean up and return
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


int CV8File::Inflate(const char* in_buf, char** out_buf, ULONG in_len, ULONG* out_len)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    unsigned long out_buf_len = in_len + CHUNK;
    *out_buf = static_cast<char*> (realloc(*out_buf, out_buf_len));
    *out_len = 0;


    // allocate inflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK)
        return ret;

    strm.avail_in = in_len;
    strm.next_in = (unsigned char *)(in_buf);

    // run inflate() on input until output buffer not full
    do {
        strm.avail_out = CHUNK;
        strm.next_out = out;
        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);  // state not clobbered
        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return ret;
        }
        have = CHUNK - strm.avail_out;
        if (*out_len + have > out_buf_len) {
            //if (have < sizeof
            out_buf_len = out_buf_len + sizeof(out);
            *out_buf = static_cast<char*> (realloc(*out_buf, out_buf_len));
            if (!out_buf) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        }
        memcpy((*out_buf + *out_len), out, have);
        *out_len += have;
    } while (strm.avail_out == 0);

    // done when inflate() says it's done

    // clean up and return
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int CV8File::Deflate(const char* in_buf, char** out_buf, ULONG in_len, ULONG* out_len)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    unsigned long out_buf_len = in_len + CHUNK;
    *out_buf = static_cast<char*> (realloc(*out_buf, out_buf_len));
    *out_len = 0;

    // allocate deflate state
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

    if (ret != Z_OK)
        return ret;


    flush = Z_FINISH;
    strm.next_in = (unsigned char *)(in_buf);
    strm.avail_in = in_len;

    // run deflate() on input until output buffer not full, finish
    //   compression if all of source has been read in
    do {
        strm.avail_out = sizeof(out);
        strm.next_out = out;
        ret = deflate(&strm, flush);    // no bad return value
        assert(ret != Z_STREAM_ERROR);  // state not clobbered
        have = sizeof(out) - strm.avail_out;
        if (*out_len + have > out_buf_len) {
            //if (have < sizeof
            out_buf_len = out_buf_len + sizeof(out);
            *out_buf = static_cast<char*> (realloc(*out_buf, out_buf_len));
            if (!out_buf) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        }
        memcpy((*out_buf + *out_len), out, have);
        *out_len += have;
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);     // all input will be used

    assert(ret == Z_STREAM_END);        // stream will be complete


    // clean up and return
    (void)deflateEnd(&strm);
    return Z_OK;

}

int CV8File::UnpackToDirectoryNoLoad(const std::string &directory, std::basic_ifstream<char> &file, bool boolInflate, bool UnpackWhenNeed)
{
    int ret = 0;

    boost::filesystem::path p_dir(directory);

    if (!boost::filesystem::exists(p_dir)) {
        if (!boost::filesystem::create_directory(directory)) {
            std::cerr << "UnpackToFolder. Error in creating directory!" << std::endl;
            return ret;
        }
    }

	stFileHeader	            FileHeader;
	std::vector<stElemAddr>     ElemsAddrs;
	std::vector<CV8Elem>        Elems;
	bool			            IsDataPacked;

	file.read((char*)&FileHeader, sizeof(FileHeader));

    stBlockHeader BlockHeader;
    stBlockHeader *pBlockHeader = &BlockHeader;

    file.read((char*)&BlockHeader, sizeof(BlockHeader));

    uint32_t ElemsAddrsSize;
    stElemAddr *pElemsAddrs = NULL;
    V8Raw::ReadBlockData(file, pBlockHeader, (char**)&pElemsAddrs, &ElemsAddrsSize);

    unsigned int ElemsNum = ElemsAddrsSize / sizeof(stElemAddr);

    Elems.clear();

    for (UINT i = 0; i < ElemsNum; i++) {

        if (pElemsAddrs[i].fffffff != V8Raw::V8_FF_SIGNATURE) {
            ElemsNum = i;
            break;
        }

        file.seekg(pElemsAddrs[i].elem_header_addr, std::ios_base::beg);
        file.read((char*)&BlockHeader, sizeof(BlockHeader));

        if (pBlockHeader->EOL_0D != 0x0d ||
                pBlockHeader->EOL_0A != 0x0a ||
                pBlockHeader->space1 != 0x20 ||
                pBlockHeader->space2 != 0x20 ||
                pBlockHeader->space3 != 0x20 ||
                pBlockHeader->EOL2_0D != 0x0d ||
                pBlockHeader->EOL2_0A != 0x0a) {

            ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
            break;
        }

        CV8Elem elem;
        V8Raw::ReadBlockData(file, pBlockHeader, (char**)&elem.pHeader, &elem.HeaderSize);

        char ElemName[512];
        UINT ElemNameLen;

        GetElemName(elem, ElemName, &ElemNameLen);

        boost::filesystem::path elem_path(p_dir / ElemName);

        boost::filesystem::ofstream o_tmp(p_dir / ".v8unpack.tmp", std::ios_base::binary);

        //080228 Блока данных может не быть, тогда адрес блока данных равен 0x7fffffff
        if (pElemsAddrs[i].elem_data_addr != V8Raw::V8_FF_SIGNATURE) {

            file.seekg(pElemsAddrs[i].elem_data_addr, std::ios_base::beg);
            file.read((char*)&BlockHeader, sizeof(BlockHeader));

            V8Raw::ReadBlockData(file, pBlockHeader, o_tmp, &elem.DataSize);
        } else {
            // TODO: Зачем это нужно??
            V8Raw::ReadBlockData(file, NULL, o_tmp, &elem.DataSize);
        }

        o_tmp.close();

        boost::filesystem::ifstream i_tmp(p_dir / ".v8unpack.tmp", std::ios_base::binary);

        elem.UnpackedData.IsDataPacked = false;

        if (boolInflate && IsDataPacked) {

            boost::filesystem::ofstream o_inf(p_dir / ".v8unpack.inf", std::ios_base::binary);
            ret = Inflate(i_tmp, o_inf);
            o_inf.close();

            boost::filesystem::ifstream i_inf(p_dir / ".v8unpack.inf", std::ios_base::binary);

            if (ret)
                IsDataPacked = false;
            else {

                elem.NeedUnpack = false; // отложенная распаковка не нужна
                if (IsV8File(i_inf)) {

                    ret = elem.UnpackedData.UnpackToDirectoryNoLoad(elem_path.string(), i_inf, 0, boolInflate);
                    if (ret)
                        break;

                } else {
                    boost::filesystem::ofstream out(elem_path, std::ios_base::binary);

                    i_inf.seekg(0, std::ios_base::beg);
                    i_inf.clear();

                    while (i_inf) {

                        const int buf_size = 1024;
                        char buf[buf_size];
                        size_t rd = i_inf.read(buf, buf_size).gcount();

                        if (rd)
                            out.write(buf, rd);
                        else
                            break;
                    }
                }
                ret = 0;
            }

        } else {

            i_tmp.seekg(0, std::ios_base::beg);
            i_tmp.clear();

            boost::filesystem::ofstream out(elem_path, std::ios_base::binary);
            while (!i_tmp.eof()) {

                const int buf_size = 1024;
                char buf[buf_size];
                size_t rd = i_tmp.read(buf, buf_size).gcount();

                if (rd)
                    out.write(buf, rd);
                else
                    break;
            }
            ret = 0;
        }

        elem.Free();

    } // for i = ..ElemsNum

    delete [] pElemsAddrs;

    if (boost::filesystem::exists(p_dir / ".v8unpack.inf"))
        boost::filesystem::remove(p_dir / ".v8unpack.inf");

    if (boost::filesystem::exists(p_dir / ".v8unpack.tmp"))
        boost::filesystem::remove(p_dir / ".v8unpack.tmp");

    return ret;
}

int CV8File::UnpackToFolder(const std::string &filename_in, const std::string &dirname, char *UnpackElemWithName, bool print_progress)
{
    int ret = 0;

    boost::filesystem::path filepath(filename_in);
    boost::filesystem::ifstream file_in(filepath, std::ios_base::binary);

    boost::filesystem::path dir_path(dirname);

    if (!boost::filesystem::exists(dir_path)) {
        if (!boost::filesystem::create_directory(dir_path)) {
            std::cerr << "UnpackToFolder. Error in creating directory!" << std::endl;
            return ret;
        }
    }

    if (!file_in) {
        std::cerr << "UnpackToFolder. Input file not found!" << std::endl;
        return -1;
    }

    stFileHeader FileHeader;
    stBlockHeader BlockHeader;

    size_t rd;

    rd = file_in.read((char*)&FileHeader, sizeof(FileHeader)).gcount();

    if (rd < sizeof(FileHeader)) {
        std::cerr << "UnpackToFolder. This is not V8 file!" << std::endl;
        return ret;
    }

    {   // Сохраняем заголовок
        boost::filesystem::ofstream file_out(dir_path / "FileHeader");
        file_out.write((char*)&FileHeader, sizeof(FileHeader));
    }

    file_in.read((char*)&BlockHeader, sizeof(BlockHeader));
    unsigned ElemsAddrsSize;

    stElemAddr *pElemsAddrs = NULL;
    V8Raw::ReadBlockData(file_in, &BlockHeader, (char**)&pElemsAddrs, &ElemsAddrsSize);
    unsigned ElemsNum = ElemsAddrsSize / sizeof(stElemAddr);

    stElemAddr *pElem = pElemsAddrs;
    for (unsigned i = ElemsNum; i; i--, pElem++) {

        if (pElem->fffffff != V8Raw::V8_FF_SIGNATURE) {
            break;
        }

        file_in.seekg(pElem->elem_header_addr, std::ios_base::beg);
        file_in.read((char*)&BlockHeader, sizeof(BlockHeader));

        if (BlockHeader.EOL_0D != 0x0d ||
                BlockHeader.EOL_0A != 0x0a ||
                BlockHeader.space1 != 0x20 ||
                BlockHeader.space2 != 0x20 ||
                BlockHeader.space3 != 0x20 ||
                BlockHeader.EOL2_0D != 0x0d ||
                BlockHeader.EOL2_0A != 0x0a) {

            ret = V8UNPACK_HEADER_ELEM_NOT_CORRECT;
            break;
        }

        CV8Elem elem;
        V8Raw::ReadBlockData(file_in, &BlockHeader, &elem.pHeader, &elem.HeaderSize);

        char *ElemName = new char[elem.HeaderSize - sizeof(stElemHeaderBegin)];
        unsigned ElemNameLen;

        GetElemName(elem, ElemName, &ElemNameLen);

        std::string sElemName(ElemName);

        delete [] ElemName;

        {
            boost::filesystem::ofstream file_out(dir_path / (sElemName + ".header"));
            file_out.write((char*)elem.pHeader, elem.HeaderSize);
            delete [] elem.pHeader;
        }

        if (pElem->elem_data_addr != V8Raw::V8_FF_SIGNATURE) {

            file_in.seekg(pElem->elem_data_addr, std::ios_base::beg);
            file_in.read((char*)&BlockHeader, sizeof(BlockHeader));
            V8Raw::ReadBlockData(file_in, &BlockHeader, &elem.pData, &elem.DataSize);

            {
                boost::filesystem::ofstream file_out(dir_path / (sElemName + ".data"));
                file_out.write((char*)elem.pData, elem.DataSize);
                delete [] elem.pData;
           }

        }

    } // for Elems

    std::cout << "Unpack Ok" << std::endl;
    return 0;
}

bool CV8File::IsV8File(std::basic_ifstream<char> &file)
{
    stFileHeader FileHeader;
    stBlockHeader BlockHeader;

    stBlockHeader *pBlockHeader = &BlockHeader;

    std::ifstream::pos_type offset = file.tellg();

    file.read((char*)&FileHeader, sizeof(FileHeader));
    file.read((char*)&BlockHeader, sizeof(BlockHeader));

    file.seekg(offset);
    file.clear();

    if (pBlockHeader->EOL_0D != 0x0d ||
            pBlockHeader->EOL_0A != 0x0a ||
            pBlockHeader->space1 != 0x20 ||
            pBlockHeader->space2 != 0x20 ||
            pBlockHeader->space3 != 0x20 ||
            pBlockHeader->EOL2_0D != 0x0d ||
            pBlockHeader->EOL2_0A != 0x0a) {

        return false;
    }

    return true;
}

int CV8File::PackFromFolder(const std::string &dirname, const std::string &filename_out)
{

    std::string filename;

    boost::filesystem::path p_curdir(dirname);

    filename = dirname;
    filename += "/FileHeader";

    boost::filesystem::path path_header(filename);

    boost::filesystem::ofstream file_out(filename_out, std::ios_base::binary);
    if (!file_out) {
        std::cerr << "PackFromFolder. Error in creating file!" << std::endl;
        return -1;
    }


    boost::filesystem::ifstream file_in(path_header, std::ios_base::binary);
    if (!file_in) {
        std::cerr << "PackFromFolder. No input FileHeader!" << std::endl;
        return -1;
    }

    stFileHeader FileHeader;

    file_in.read((char *)&FileHeader, sizeof(FileHeader));
    file_in.close();

    boost::filesystem::directory_iterator d_end;

    Elems.clear();

    unsigned ElemsNum = 0;
    // Высчитываем количество файлов
    {
        boost::filesystem::directory_iterator dit(p_curdir);

        if (dit == d_end) {
            std::cerr << "Build error. Directory `" << p_curdir << "` is empty.";
            return -1;
        }

        for (; dit != d_end; ++dit) {

            boost::filesystem::path current_file(dit->path());

            if (current_file.extension().string() == ".header")
                ++ElemsNum;

        }
    }

    uint32_t cur_block_addr = sizeof(stFileHeader) + sizeof(stBlockHeader);
    stElemAddr *pTOC;
    pTOC = new stElemAddr[ElemsNum];
    if (sizeof(stElemAddr) * ElemsNum < V8Raw::V8_DEFAULT_PAGE_SIZE)
        cur_block_addr += V8Raw::V8_DEFAULT_PAGE_SIZE;
    else
        cur_block_addr += sizeof(stElemAddr) * ElemsNum;

    //Резервируем место в начале файла под заголовок и TOC
    for(unsigned i=0; i < cur_block_addr; i++) {
        file_out << '\0';
    }

    unsigned ElemNum = 0;
    boost::filesystem::directory_iterator it(p_curdir);
    for (; it != d_end; it++) {
        boost::filesystem::path current_file(it->path());
        CV8Elem elem;
        if (current_file.extension().string() == ".header") {

            filename = dirname;
            filename += current_file.filename().string();

            CV8Elem elem;

            {
                boost::filesystem::ifstream file_in(current_file, std::ios_base::binary);
                file_in.seekg(0, std::ios_base::end);

                elem.HeaderSize = file_in.tellg();
                elem.pHeader = new char[elem.HeaderSize];

                file_in.seekg(0, std::ios_base::beg);
                file_in.read((char *)elem.pHeader, elem.HeaderSize);
            }

            boost::filesystem::path data_path = current_file.replace_extension("data");
            {
                boost::filesystem::ifstream file_in(data_path, std::ios_base::binary);

                if (file_in) {
                    file_in.seekg(0, std::ios_base::end);
                    elem.DataSize = file_in.tellg();
                    file_in.seekg(0, std::ios_base::beg);
                    elem.pData = new char[elem.DataSize];

                    file_in.read((char *)elem.pData, elem.DataSize);
                }
            }

            V8Raw::SaveBlockData(file_out, elem.pHeader, elem.HeaderSize, elem.HeaderSize);
            if (elem.pData)
                V8Raw::SaveBlockData(file_out, elem.pData, elem.DataSize);

        } else
            continue;

        //Добавляем элемент в TOC
        pTOC[ElemNum].elem_header_addr = cur_block_addr;
        cur_block_addr += sizeof(stBlockHeader) + elem.HeaderSize;
        pTOC[ElemNum].elem_data_addr = cur_block_addr;
        cur_block_addr += sizeof(stBlockHeader);
        if (elem.DataSize > V8Raw::V8_DEFAULT_PAGE_SIZE)
            cur_block_addr += elem.DataSize;
        else
            cur_block_addr += V8Raw::V8_DEFAULT_PAGE_SIZE;

        pTOC[ElemNum].fffffff = V8Raw::V8_FF_SIGNATURE;

        delete[] elem.pData;
        elem.pData = NULL;
        delete[] elem.pHeader;
        elem.pHeader = NULL;
        elem.IsV8File = false;
        elem.HeaderSize = 0;
        elem.DataSize = 0;

       ++ElemNum;
    } // for it

    //Записывем заголовок файла
    file_out.seekp(0, std::ios_base::beg);

    // записываем заголовок
    file_out.write(reinterpret_cast<char*>(&FileHeader), sizeof(FileHeader));

    //Записываем блок TOC
    V8Raw::SaveBlockData(file_out, (const char*) pTOC, sizeof(stElemAddr) * ElemsNum);

    delete [] pTOC;

    return 0;
}

int CV8File::Parse(const std::string &filename_in, const std::string &dirname, int level)
{
    int ret = 0;

    boost::filesystem::ifstream file_in(filename_in, std::ios_base::binary);

    if (!file_in) {
        std::cerr << "UnpackToFolder. Input file not found!" << std::endl;
        return -1;
    }

    file_in.seekg(0, std::ios_base::end);
    ULONG FileDataSize = file_in.tellg();
    file_in.seekg(0, std::ios_base::beg);

    ret = UnpackToDirectoryNoLoad(dirname, file_in, FileDataSize);

    std::cout << "LoadFile: ok" << std::endl;

    return ret;
}

int CV8File::GetElemName(const CV8Elem &Elem, char *ElemName, UINT *ElemNameLen)
{
    *ElemNameLen = (Elem.HeaderSize - sizeof(stElemHeaderBegin)) / 2;
    for (UINT j = 0; j < *ElemNameLen * 2; j+=2)
        ElemName[j/2] = Elem.pHeader[sizeof(stElemHeaderBegin) + j];

    return 0;
}


int CV8File::SetElemName(CV8Elem &Elem, const char *ElemName, UINT ElemNameLen)
{
    UINT stElemHeaderBeginSize = sizeof(stElemHeaderBegin);

    for (UINT j = 0; j <ElemNameLen * 2; j+=2, stElemHeaderBeginSize+=2) {
        Elem.pHeader[stElemHeaderBeginSize] = ElemName[j/2];
        Elem.pHeader[stElemHeaderBeginSize + 1] = 0;
    }

    return 0;
}



int CV8File::LoadFileFromFolder(const std::string &dirname)
{

    std::string filename;

    boost::filesystem::ifstream file_in;

    FileHeader.next_page_addr = V8Raw::V8_FF_SIGNATURE;
    FileHeader.page_size = V8Raw::V8_DEFAULT_PAGE_SIZE;
    FileHeader.storage_ver = 0;
    FileHeader.reserved = 0;

    Elems.clear();

    boost::filesystem::directory_iterator d_end;
    boost::filesystem::directory_iterator dit(dirname);

    for (; dit != d_end; ++dit) {
        boost::filesystem::path current_file(dit->path());
        if (current_file.filename().string().at(0) == '.')
            continue;

        CV8Elem elem;
        std::string name = current_file.filename().string();

        elem.HeaderSize = sizeof(stElemHeaderBegin) + (name.size() + 2) * 2;
        elem.pHeader = new char[elem.HeaderSize];

        memset(elem.pHeader, 0, elem.HeaderSize);

        SetElemName(elem, name.c_str(), name.size());

        if (boost::filesystem::is_directory(current_file)) {

            elem.IsV8File = true;

            std::string new_dirname(dirname);
            new_dirname += "/";
            new_dirname += name;

            elem.UnpackedData.LoadFileFromFolder(new_dirname);

        } else {
            elem.IsV8File = false;

            elem.DataSize = boost::filesystem::file_size(current_file);
            elem.pData = new char[elem.DataSize];

            boost::filesystem::ifstream file_in(current_file, std::ios_base::binary);
            file_in.read(reinterpret_cast<char *>(elem.pData), elem.DataSize);
        }

        Elems.push_back(elem);

    } // for directory_iterator

    return 0;

}

int CV8File::BuildCfFile(const std::string &in_dirname, const std::string &out_filename)
{
    //filename can't be empty
    if (!in_dirname.size()) {
        fputs("Argument error - Set of `in_dirname' argument \n", stderr);
        std::cerr << "Argument error - Set of `in_dirname' argument" << std::endl;
        return SHOW_USAGE;
    }

    if (!out_filename.size()) {
        std::cerr << "Argument error - Set of `out_filename' argument" << std::endl;
        return SHOW_USAGE;
    }

    UINT ElemsNum = 0;
    {
        boost::filesystem::directory_iterator d_end;
        boost::filesystem::directory_iterator dit(in_dirname);

        if (dit == d_end) {
            std::cerr << "Build error. Directory `" << in_dirname << "` is empty.";
            return -1;
        }

        for (; dit != d_end; ++dit) {

            boost::filesystem::path current_file(dit->path());
            std::string name = current_file.filename().string();

            if (name.at(0) == '.')
                continue;

            ++ElemsNum;
        }
    }


    //Предварительные расчеты длины заголовка таблицы содержимого TOC файла
    FileHeader.next_page_addr = V8Raw::V8_FF_SIGNATURE;
    FileHeader.page_size = V8Raw::V8_DEFAULT_PAGE_SIZE;
    FileHeader.storage_ver = 0;
    FileHeader.reserved = 0;
    DWORD cur_block_addr = sizeof(stFileHeader) + sizeof(stBlockHeader);
    stElemAddr *pTOC;
    pTOC = new stElemAddr[ElemsNum];
    if (sizeof(stElemAddr) * ElemsNum < V8Raw::V8_DEFAULT_PAGE_SIZE)
        cur_block_addr += V8Raw::V8_DEFAULT_PAGE_SIZE;
    else
        cur_block_addr += sizeof(stElemAddr) * ElemsNum;

    boost::filesystem::ofstream file_out(out_filename, std::ios_base::binary);
    //Открываем выходной файл контейнер на запись
    if (!file_out) {
        std::cout << "SaveFile. Error in creating file!" << std::endl;
        return -1;
    }

    //Резервируем место в начале файла под заголовок и TOC
    for(unsigned i=0; i < cur_block_addr; i++) {
        file_out << '\0';
    }

    UINT one_percent = ElemsNum / 50;
    if (one_percent) {
        std::cout << "Progress (50 points): " << std::flush;
    }


    std::string new_dirname;

    UINT ElemNum = 0;

    boost::filesystem::directory_iterator d_end;
    boost::filesystem::directory_iterator dit(in_dirname);
    for (; dit != d_end; ++dit) {

        boost::filesystem::path current_file(dit->path());
        std::string name = current_file.filename().string();

        if (name.at(0) == '.')
            continue;


        //Progress bar ->
        {
            if (ElemNum && one_percent && ElemNum%one_percent == 0) {
                if (ElemNum % (one_percent*10) == 0)
                    std::cout << "|" << std::flush;
                else
                    std::cout << ".";
            }
        }//<- Progress bar

        CV8Elem pElem;

        pElem.HeaderSize = sizeof(stElemHeaderBegin) + (name.size() + 2) * 2;
        pElem.pHeader = new char[pElem.HeaderSize];

        memset(pElem.pHeader, 0, pElem.HeaderSize);

        SetElemName(pElem, name.c_str(), name.size());
        if (boost::filesystem::is_directory(current_file)) {

            pElem.IsV8File = true;

            std::string new_dirname(in_dirname);
            new_dirname += "/";
            new_dirname += name;

            pElem.UnpackedData.LoadFileFromFolder(new_dirname);

        } else {

            pElem.IsV8File = false;

            pElem.DataSize = boost::filesystem::file_size(current_file);
            pElem.pData = new char[pElem.DataSize];

            boost::filesystem::path p_filename(in_dirname);
            p_filename /= name;

            boost::filesystem::ifstream file_in(p_filename, std::ios_base::binary);
            file_in.read(reinterpret_cast<char*>(pElem.pData), pElem.DataSize);
        }
        //Сжимаем данные
        PackElem(pElem);
        //Добавляем элемент в TOC
        pTOC[ElemNum].elem_header_addr = cur_block_addr;
        cur_block_addr += sizeof(stBlockHeader) + pElem.HeaderSize;
        pTOC[ElemNum].elem_data_addr = cur_block_addr;
        cur_block_addr += sizeof(stBlockHeader);
        if (pElem.DataSize > V8Raw::V8_DEFAULT_PAGE_SIZE)
            cur_block_addr += pElem.DataSize;
        else
            cur_block_addr += V8Raw::V8_DEFAULT_PAGE_SIZE;
        pTOC[ElemNum].fffffff = V8Raw::V8_FF_SIGNATURE;
        //Записываем элемент в файл
        V8Raw::SaveBlockData(file_out, pElem.pHeader, pElem.HeaderSize, pElem.HeaderSize);
        V8Raw::SaveBlockData(file_out, pElem.pData, pElem.DataSize);
        //Освобождаем память
        delete[] pElem.pData;
        pElem.pData = NULL;
        delete[] pElem.pHeader;
        pElem.pHeader = NULL;
        pElem.IsV8File = false;
        pElem.HeaderSize = 0;
        pElem.DataSize = 0;
        ElemNum++;
    }

    //Записывем заголовок файла
    file_out.seekp(0, std::ios_base::beg);
    file_out.write(reinterpret_cast<const char*>(&FileHeader), sizeof(FileHeader));

    //Записываем блок TOC
    V8Raw::SaveBlockData(file_out, (const char*) pTOC, sizeof(stElemAddr) * ElemsNum);

    delete [] pTOC;

    std::cout << std::endl << "Build OK!";

    return 0;
}

int CV8File::PackElem(CV8Elem &pElem)
{
    char *DeflateBuffer = NULL;
    ULONG DeflateSize = 0;

    char *DataBuffer = NULL;
    ULONG DataBufferSize = 0;

    int ret = 0;
    if (!pElem.IsV8File) {
        ret = Deflate(pElem.pData, &DeflateBuffer, pElem.DataSize, &DeflateSize);
        if (ret)
            return ret;

        delete[] pElem.pData;
        pElem.pData = new char[DeflateSize];
        pElem.DataSize = DeflateSize;
        memcpy(pElem.pData, DeflateBuffer, DeflateSize);
    } else {
        pElem.UnpackedData.GetData(&DataBuffer, &DataBufferSize);

        ret = Deflate(DataBuffer, &DeflateBuffer, DataBufferSize, &DeflateSize);
        if (ret)
            return ret;

        //pElem.UnpackedData = CV8File();
        pElem.IsV8File = false;

        pElem.pData = new char[DeflateSize];
        pElem.DataSize = DeflateSize;
        memcpy(pElem.pData, DeflateBuffer, DeflateSize);

    }

    if (DeflateBuffer)
        free(DeflateBuffer);

    if (DataBuffer)
        free(DataBuffer);

    return 0;
}

int CV8File::GetData(char **DataBuffer, ULONG *DataBufferSize) const
{

    UINT ElemsNum = Elems.size();

    ULONG NeedDataBufferSize = 0;
    NeedDataBufferSize += sizeof(stFileHeader);

    // заголовок блока и данные блока - адреса элементов с учетом минимальной страницы 512 байт
    NeedDataBufferSize += sizeof(stBlockHeader) + std::max(sizeof(stElemAddr) * ElemsNum, V8Raw::V8_DEFAULT_PAGE_SIZE);

    std::vector<CV8Elem>::const_iterator elem;
    //for(ElemNum = 0; ElemNum < ElemsNum; ElemNum++)
    for (elem = Elems.begin(); elem != Elems.end(); ++elem) {

        // заголовок блока и данные блока - заголовок элемента
        NeedDataBufferSize += sizeof(stBlockHeader)  + elem->HeaderSize;

        // заголовок блока и данные блока - данные элемента с учетом минимальной страницы 512 байт
        NeedDataBufferSize += sizeof(stBlockHeader)  + std::max((size_t)elem->DataSize, V8Raw::V8_DEFAULT_PAGE_SIZE);
    }


    // Создаем и заполняем данные по адресам элементов
    stElemAddr *pTempElemsAddrs = new stElemAddr[ElemsNum], *pCurrentTempElem;
    pCurrentTempElem = pTempElemsAddrs;

    DWORD cur_block_addr = sizeof(stFileHeader) + sizeof(stBlockHeader);
    if (sizeof(stElemAddr) * ElemsNum < V8Raw::V8_DEFAULT_PAGE_SIZE)
        cur_block_addr += V8Raw::V8_DEFAULT_PAGE_SIZE;
    else
        cur_block_addr += sizeof(stElemAddr) * ElemsNum;

    for (elem = Elems.begin(); elem !=  Elems.end(); ++elem) {

        pCurrentTempElem->elem_header_addr = cur_block_addr;
        cur_block_addr += sizeof(stBlockHeader) + elem->HeaderSize;

        pCurrentTempElem->elem_data_addr = cur_block_addr;
        cur_block_addr += sizeof(stBlockHeader);

        if (elem->DataSize > V8Raw::V8_DEFAULT_PAGE_SIZE)
            cur_block_addr += elem->DataSize;
        else
            cur_block_addr += V8Raw::V8_DEFAULT_PAGE_SIZE;

        pCurrentTempElem->fffffff = V8Raw::V8_FF_SIGNATURE;
        ++pCurrentTempElem;
    }


    *DataBuffer = static_cast<char*> (realloc(*DataBuffer, NeedDataBufferSize));


    char *cur_pos = *DataBuffer;


    // записываем заголовок
    memcpy(cur_pos, (char*) &FileHeader, sizeof(stFileHeader));
    cur_pos += sizeof(stFileHeader);

    // записываем адреса элементов
    SaveBlockDataToBuffer(&cur_pos, (char*) pTempElemsAddrs, sizeof(stElemAddr) * ElemsNum);

    // записываем элементы (заголовок и данные)
    for (elem = Elems.begin(); elem != Elems.end(); ++elem) {
        SaveBlockDataToBuffer(&cur_pos, elem->pHeader, elem->HeaderSize, elem->HeaderSize);
        SaveBlockDataToBuffer(&cur_pos, elem->pData, elem->DataSize);
    }

    //fclose(file_out);

    if (pTempElemsAddrs)
        delete[] pTempElemsAddrs;

    *DataBufferSize = NeedDataBufferSize;

    return 0;

}


int CV8File::SaveBlockDataToBuffer(char **cur_pos, const char *pBlockData, UINT BlockDataSize, UINT PageSize) const
{

    if (PageSize < BlockDataSize)
        PageSize = BlockDataSize;

    stBlockHeader CurBlockHeader;

    CurBlockHeader.EOL_0D = 0xd;
    CurBlockHeader.EOL_0A = 0xa;
    CurBlockHeader.EOL2_0D = 0xd;
    CurBlockHeader.EOL2_0A = 0xa;

    CurBlockHeader.space1 = 0;
    CurBlockHeader.space2 = 0;
    CurBlockHeader.space3 = 0;

    char buf[20];

    sprintf(buf, "%08x", BlockDataSize);
    strncpy(CurBlockHeader.data_size_hex, buf, 8);

    sprintf(buf, "%08x", PageSize);
    strncpy(CurBlockHeader.page_size_hex, buf, 8);

    sprintf(buf, "%08x", V8Raw::V8_FF_SIGNATURE);
    strncpy(CurBlockHeader.next_page_addr_hex, buf, 8);

    CurBlockHeader.space1 = ' ';
    CurBlockHeader.space2 = ' ';
    CurBlockHeader.space3 = ' ';


    memcpy(*cur_pos, (char*)&CurBlockHeader, sizeof(stBlockHeader));
    *cur_pos += sizeof(stBlockHeader);


    memcpy(*cur_pos, pBlockData, BlockDataSize);
    *cur_pos += BlockDataSize;

    for(UINT i = 0; i < PageSize - BlockDataSize; i++) {
        **cur_pos = 0;
        ++*cur_pos;
    }

    return 0;
}

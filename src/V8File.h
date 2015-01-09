/////////////////////////////////////////////////////////////////////////////
//
//
//	Author:			disa_da
//	E-mail:			disa_da2@mail.ru
//
//
/////////////////////////////////////////////////////////////////////////////

/**
    2014-2015       dmpas           sergey(dot)batanov(at)dmpas(dot)ru
 */

// V8File.h: interface for the CV8File class.
//
//////////////////////////////////////////////////////////////////////

/*#if !defined(AFX_V8FILE_H__935D5C2B_70FA_45F2_BDF2_A0274A8FD60C__INCLUDED_)
#define AFX_V8FILE_H__935D5C2B_70FA_45F2_BDF2_A0274A8FD60C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000*/

#include <assert.h>
#include <string>
#include "zlib.h"
#include <fstream>

#include <stdint.h>
#include <vector>
#include <boost/shared_array.hpp>

#include <v8container/V8Raw.h>

typedef uint32_t UINT;
typedef uint32_t DWORD;
typedef uint32_t ULONG;
typedef uint64_t ULONGLONG;


#define CHUNK 16384
#ifndef DEF_MEM_LEVEL
#  if MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  else
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif


#define V8UNPACK_ERROR -50
#define V8UNPACK_NOT_V8_FILE (V8UNPACK_ERROR-1)
#define V8UNPACK_HEADER_ELEM_NOT_CORRECT (V8UNPACK_ERROR-2)


#define V8UNPACK_INFLATE_ERROR (V8UNPACK_ERROR-20)
#define V8UNPACK_INFLATE_IN_FILE_NOT_FOUND (V8UNPACK_INFLATE_ERROR-1)
#define V8UNPACK_INFLATE_OUT_FILE_NOT_CREATED (V8UNPACK_INFLATE_ERROR-2)
#define V8UNPACK_INFLATE_DATAERROR (V8UNPACK_INFLATE_ERROR-3)

#define V8UNPACK_DEFLATE_ERROR (V8UNPACK_ERROR-30)
#define V8UNPACK_DEFLATE_IN_FILE_NOT_FOUND (V8UNPACK_ERROR-1)
#define V8UNPACK_DEFLATE_OUT_FILE_NOT_CREATED (V8UNPACK_ERROR-2)

#define SHOW_USAGE -22

class CV8Elem;

class CV8File
{
public:
	int SaveBlockDataToBuffer(char** Buffer, const char* pBlockData, UINT BlockDataSize, UINT PageSize = 512) const;
	int GetData(char **DataBufer, ULONG *DataBuferSize) const;
	int SaveFile(const std::string &filename) __attribute__((__deprecated__));
	int SetElemName(CV8Elem &Elem, const char *ElemName, UINT ElemNameLen);
	int LoadFileFromFolder(const std::string &dirname);
	int GetElemName(const CV8Elem &Elem, char* ElemName, UINT *ElemNameLen) const;
	int Parse(const std::string &filename, const std::string &dirname, int level = 0);

	static bool IsV8File(std::basic_ifstream<char> &file);

	int BuildCfFile(const std::string &dirname, const std::string &filename);


	static int Deflate(std::basic_ifstream<char> &source, std::basic_ofstream<char> &dest);
	static int Inflate(std::basic_ifstream<char> &source, std::basic_ofstream<char> &dest);

	static int Deflate(const std::string &in_filename, const std::string &out_filename);
	static int Inflate(const std::string &in_filename, const std::string &out_filename);

	static int Deflate(const char* in_buf, char** out_buf, ULONG in_len, ULONG* out_len);
	static int Inflate(const char* in_buf, char** out_buf, ULONG in_len, ULONG* out_len);

	int UnpackToDirectoryNoLoad(const std::string &directory, std::basic_ifstream<char> &file, ULONG FileData, bool boolInflate = true, bool UnpackWhenNeed = false);

	int UnpackToFolder(const std::string &filename, const std::string &dirname, char *block_name = NULL, bool print_progress = false);

	int PackFromFolder(const std::string &dirname, const std::string &filename);

	int SaveBlockData(std::basic_ofstream<char> &file_out, const char *pBlockData, UINT BlockDataSize, UINT PageSize = 512);

	static int PackElem(CV8Elem &pElem);


	CV8File();

	virtual ~CV8File();

	CV8File(const CV8File &src);

private:
	stFileHeader	            FileHeader;
	std::vector<stElemAddr>     ElemsAddrs;

	std::vector<CV8Elem>        Elems;
	bool			            IsDataPacked;
};

class CV8Elem
{
public:

	CV8Elem(const CV8Elem &src);
	CV8Elem();
	~CV8Elem();

	char	           *pHeader; // TODO: Утечка памяти
	UINT	            HeaderSize;
	char	           *pData; // TODO: Утечка памяти
	UINT	            DataSize;
	CV8File             UnpackedData;
	bool	            IsV8File;
	bool	            NeedUnpack;

};

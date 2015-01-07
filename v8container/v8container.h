/*! \file v8container.h */


#ifndef V8CONTAINER_LIB_H
#define V8CONTAINER_LIB_H

#include <stdint.h>

/* Не забываем, что у нас GCC */
#define STRICT_MEMORY_SIZE __attribute__((packed))

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef DOXYGEN_SHOULD_SKIP_THIS
typedef struct __stFileHeader {
    uint32_t            next_page_addr;
    uint32_t            page_size;
    uint32_t            storage_ver;
    uint32_t            reserved;          // всегда 0x00000000 ?
} STRICT_MEMORY_SIZE stFileHeader;

typedef struct __stElemAddr {
    uint32_t            elem_header_addr;
    uint32_t            elem_data_addr;
    uint32_t            fffffff;           //всегда 0x7fffffff ?
} STRICT_MEMORY_SIZE stElemAddr;

typedef struct __stBlockHeader {
    char                EOL_0D;
    char                EOL_0A;
    char                data_size_hex[8];
    char                space1;
    char                page_size_hex[8];
    char                space2;
    char                next_page_addr_hex[8];
    char                space3;
    char                EOL2_0D;
    char                EOL2_0A;
} STRICT_MEMORY_SIZE stBlockHeader;

typedef struct __stElemHeaderBegin {
    uint64_t            date_creation;
    uint64_t            date_modification;
    uint32_t            res;               // всегда 0x000000?
} STRICT_MEMORY_SIZE stElemHeaderBegin;

#endif // DOXYGEN_SHOULD_SKIP_THIS

typedef struct __stV8Container V8Container;
typedef struct __stV8File V8File;

/*! \defgroup V8Container Процедуры для работы с 8-файлом
  @{
 */
/*! Открывает 8-файл для чтения-записи */
V8Container *
V8Container_OpenFile(
                        const   char                   *filename /*!< Путь к файлу */
                    );

/*! Открывает вложенный 8-файл для чтения-записи */
V8Container *
V8Container_OpenV8File(
                        const   char                   *V8File /*!< 8-файл, вложенный в другой файл */
                       );

/*! Создаёт новый пустой 8-файл и открывает для чтения-записи */
V8Container *
V8Container_CreateFile(
                        const   char                   *filename /*!< Путь к файлу*/
                    );

/*! Закрывает файл и освобождает выделенную память

После закрытия указатель на V8Container и указатели на связанные с ним V8File становятся неверными
и дальнейшее их использование приведёт к неопределённому поведению.
 */
void
V8Container_CloseFile(
                                V8Container            *Container /*!< Открытый 8-файл */
                        );

/*! Определяет количество файлов в корне 8-файла */
int
V8Container_GetFilesCount(
                                V8Container            *Container /*!< Открытый 8-файл */
                        );

/*! Получает список файлов, содержащихся в 8-файле */
const V8File *
V8Container_GetFilesList(
                                V8Container            *Container
                        );

/*! Добавляет внешний файл в 8-файл */
const V8File *
V8Container_AddFile(
                                V8Container            *Container, /*!< 8-файл, открытый для записи */
                        const   char                   *filename   /*!< Путь к файлу */
                    );

/*! Удаляет из 8-файла вложенный файл

После удаления File становится неправильным указателем
и дальнейшее использование приводёт к неопределённому поведению.
*/
void
V8Container_RemoveFile(
                                V8Container            *Container, /*!< 8-файл, открытый для записи */
                        const   V8File                 *File       /*!< Вложенный файл */
                        );


/*! Извлекает из 8-файла вложенный файл в сыром виде (без распаковки) */
void
V8Container_ExtractFileRaw(
                                V8Container            *Container, /*!< 8-файл, открытый для чтения */
                        const   V8File                 *File,      /*!< Вложенный файл */
                        const   char                   *filename   /*!< Путь к выходному файлу */
                        );

/*! Извлекает из 8-файла вложенный файл, при необходимости распаковывая его */
void
V8Container_ExtractFile(
                                V8Container            *Container, /*!< 8-файл, открытый для чтения */
                        const   V8File                 *File,      /*!< Вложенный файл */
                        const   char                   *filename   /*!< Путь к выходному файлу */
                        );

/*!
 }@
*/

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // V8CONTAINER_LIB_H

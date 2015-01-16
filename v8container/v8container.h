/*! \file v8container.h */


#ifndef V8CONTAINER_LIB_H
#define V8CONTAINER_LIB_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef struct __stV8Container V8Container;
typedef struct __stV8File V8File;

/*! \defgroup V8Container Процедуры для работы с 8-файлом
  @{
 */
/*! Открывает 8-файл для чтения-записи */
V8Container *
V8Container_OpenFile(
                        const   char                   *filename, /*!< Путь к файлу */
                                bool                    inflated  /*!< Данные внутри файла запакованы */
                    );

/*! Открывает вложенный 8-файл для чтения-записи */
V8Container *
V8Container_OpenV8File(
                        const   V8File                 *v8File /*!< 8-файл, вложенный в другой файл */
                       );

/*! Создаёт новый пустой 8-файл и открывает для чтения-записи */
V8Container *
V8Container_CreateFile(
                        const   char                   *filename, /*!< Путь к файлу*/
                                bool                    inflated  /*!< Данные внутри файла запакованы */
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

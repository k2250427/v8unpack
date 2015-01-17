#include <stdio.h>
#include <stdlib.h>

#include "v8container/v8container.h"

int main()
{
    const char filename[] = "test.v8c";

    V8Container *C = V8Container_CreateFile(filename, false);

    V8Container_SetDefaultPageSize(C, 4096);

    V8File *vf_main_c       = V8Container_AddFile(C, "main.c");
    V8File *vf_testlib_cbp  = V8Container_AddFile(C, "testlibc.cbp");

    (void)vf_testlib_cbp;

    V8Container_CloseFile(C);

    C = V8Container_OpenFile(filename, false);

    vf_main_c               = V8Container_FindFile(C, "main.c");

    V8Container_RemoveFile(C,  vf_main_c);

    vf_main_c               = V8Container_AddFile(C, "main.c");

    V8Container_CloseFile(C);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include "v8container/v8container.h"

int main()
{
    const char filename[] = "test.v8c";

    V8Container *C = V8Container_CreateFile(filename, false);

    V8Container_AddFile(C, "main.c");

    V8Container_CloseFile(C);

    C = V8Container_OpenFile(filename, false);

    return 0;
}

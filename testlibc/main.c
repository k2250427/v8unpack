#include <stdio.h>
#include <stdlib.h>

#include "v8container/v8container.h"

int main()
{
    const char filename[] = "./test.v8c";

    V8Container *C = V8Container_CreateFile(filename);

    V8Container_CloseFile(C);

    return 0;
}

#include "dalloc.h"

#include <stdio.h>

int
main(int argc, char** argv)
{
    void* freed_memory = DMALLOC(2, int);
    freed_memory = DMALLOC(4, int);

    freed_memory = DREALLOC(freed_memory, 3, int);

    void* leaked_memory = DCALLOC(10, char);

    DFREE(freed_memory);

    DFREE(freed_memory);

    char* report = DALLOC_REPORT();

    printf("%s\n", report);

    DALLOC_CLEANUP();
}
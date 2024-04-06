#pragma once

#include <stdlib.h>

#ifndef DEBUG

#define DMALLOC(nitems, type) malloc(nitems * sizeof(type))

#define DCALLOC(nitems, type) calloc(nitems, sizeof(type))

#define DREALLOC(ptr, nitems, type) realloc(ptr, nitems * sizeof(type))

#define DFREE(ptr) free(ptr)

#define DALLOC_CLEANUP()

#define DALLOC_REPORT()                                                        \
    "Compile with DEBUG defined to get an allocation debug report."

#else

#define DALLOC_INFO(n_items, type)                                             \
    (DAllocInfo)                                                               \
    {                                                                          \
        .filename = __FILE_NAME__, .lineno = __LINE__, .nitems = n_items,      \
        .size = sizeof(type), .typename = #type,                               \
    }

#define DFREE_INFO()                                                           \
    (DFreeInfo)                                                                \
    {                                                                          \
        .filename = __FILE_NAME__, .lineno = __LINE__,                         \
    }

#define DMALLOC(nitems, type) dmalloc(DALLOC_INFO(nitems, type))

#define DCALLOC(nitems, type) dcalloc(DALLOC_INFO(nitems, type))

#define DREALLOC(ptr, nitems, type) drealloc(ptr, DALLOC_INFO(nitems, type))

#define DFREE(ptr) dfree(ptr, DFREE_INFO())

#define DALLOC_CLEANUP() dalloc_cleanup()

#define DALLOC_REPORT() dalloc_report()

typedef struct
{
    const char* filename;
    int lineno;
    size_t nitems;
    size_t size;
    const char* typename;
} DAllocInfo;

typedef struct
{
    const char* filename;
    int lineno;
} DFreeInfo;

void*
dmalloc(DAllocInfo info);

void*
dcalloc(DAllocInfo info);

void*
drealloc(void* ptr, DAllocInfo info);

void
dfree(void* ptr, DFreeInfo info);

void
dalloc_cleanup();

char*
dalloc_report();

#endif
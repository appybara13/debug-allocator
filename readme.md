# Debug Alloc

A stdlib replacement that reports leaked memory, failed allocations, and incorrect frees.

## Build

Add as a git submodule and link with the `dalloc` target.

## Use

Example usage follows:

```c
#include "dalloc.h"

char* malloc_example = DMALLOC(16, char);
int* calloc_example = DCALLOC(16, int);
int* realloc_example = DREALLOC(calloc_example, 20, int);

DFREE(malloc_example);
DFREE(calloc_example);
DFREE(realloc_example);

printf("%s\n", DALLOC_REPORT());

DALLOC_CLEANUP();

```

Note that all three allocation macros take in a number of items and a type. For the sake of brevity, the macros includes `sizeof(<type>)`.

In a release build, the macros expand into the respective stdlib functions, so there is no release performance impact 
from using this library.

In a debug build, allocations are tracked and `DALLOC_REPORT()` will return a string containing any memory issues. It will also 
protect against double frees and clean up leaks.

The library will not work as expected if functions are mismatched. 
For example, if memory is allocated with `DMALLOC()` but freed with `free()` a leak will be reported.

A example report, from the test included in this repo, follows:

```
46 bytes allocated in total, with 18 leaked.
+==============================================================================+
|          Issue |    File | Line | Type | Item Count | Type Size | Total Size |
+------------------------------------------------------------------------------+
|   LEAKED ALLOC | tests.c |    8 |  int |          2 |         4 |          8 |
|   LEAKED ALLOC | tests.c |   13 | char |         10 |         1 |         10 |
|    FAILED FREE | tests.c |   17 |      |          0 |         0 |          0 |
+==============================================================================+
```

## Justification

This library works at a higher level than Valgrind - it only tracks allocations made with the included macros.

This has a number of disadvantages, including that it won't track any other allocator calls.

It also has one big advantage: *it doesn't track any other allocator calls*. Valgrind tracks *all* memory allocations, including 
third party libraries. Simply initializing SDL and Vulkan can take several seconds when running through Valgrind. 
Using this library, you can check only the memory that you allocate.


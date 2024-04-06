#include "dalloc.h"

#ifdef DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t allocs_count = 0;
DAllocInfo* allocs = NULL;
void** alloc_ptrs = NULL;

size_t freed_allocs_count = 0;
DAllocInfo* freed_allocs = NULL;

size_t failed_allocs_count = 0;
DAllocInfo* failed_allocs = NULL;

size_t failed_frees_count = 0;
DFreeInfo* failed_frees = NULL;

size_t report_str_len = 0;
char* report_str = NULL;

void
report_alloc(void* ptr, DAllocInfo info);

bool
report_free(void* ptr, DFreeInfo info);

void
append_to_report(const char* str);

void*
dmalloc(DAllocInfo info)
{
    void* ptr = malloc(info.nitems * info.size);
    report_alloc(ptr, info);
    return ptr;
}

void*
dcalloc(DAllocInfo info)
{
    if (!info.nitems || !info.size)
        return NULL;
    void* ptr = calloc(info.nitems, info.size);
    report_alloc(ptr, info);
    return ptr;
}

void*
drealloc(void* ptr, DAllocInfo info)
{
    DFreeInfo ptr_free_info = { .filename = info.filename,
                                .lineno = info.lineno };

    if (!info.nitems || !info.size) {
        dfree(ptr, ptr_free_info);
        return NULL;
    }

    void* new_ptr = realloc(ptr, info.nitems * info.size);
    if (new_ptr) {
        report_free(ptr, ptr_free_info);
    }
    report_alloc(new_ptr, info);
    return new_ptr;
}

void
dfree(void* ptr, DFreeInfo info)
{
    if (report_free(ptr, info))
        free(ptr);
}

void
report_alloc(void* ptr, DAllocInfo info)
{
    if (!ptr) {
        failed_allocs = realloc(failed_allocs,
                                (failed_allocs_count + 1) * sizeof(DAllocInfo));
        failed_allocs[failed_allocs_count] = info;
        failed_allocs_count += 1;
        return;
    }

    alloc_ptrs = realloc(alloc_ptrs, (allocs_count + 1) * sizeof(void*));
    allocs = realloc(allocs, (allocs_count + 1) * sizeof(DAllocInfo));

    alloc_ptrs[allocs_count] = ptr;
    allocs[allocs_count] = info;

    allocs_count += 1;
}

bool
report_free(void* ptr, DFreeInfo info)
{
    if (!ptr)
        return false;

    size_t alloc_idx = 0;
    while (alloc_idx < allocs_count && alloc_ptrs[alloc_idx] != ptr)
        alloc_idx += 1;

    if (alloc_idx < allocs_count) {
        freed_allocs =
          realloc(freed_allocs, (freed_allocs_count + 1) * sizeof(DAllocInfo));
        freed_allocs[freed_allocs_count] = allocs[alloc_idx];
        freed_allocs_count += 1;

        DAllocInfo* new_allocs =
          malloc((allocs_count - 1) * sizeof(DAllocInfo));
        memcpy(new_allocs, allocs, alloc_idx * sizeof(DAllocInfo));
        memcpy(new_allocs + alloc_idx,
               allocs + alloc_idx + 1,
               (allocs_count - alloc_idx - 1) * sizeof(DAllocInfo));
        free(allocs);
        allocs = new_allocs;

        void** new_alloc_ptrs = malloc((allocs_count - 1) * sizeof(void*));
        memcpy(new_alloc_ptrs, alloc_ptrs, alloc_idx * sizeof(void*));
        memcpy(new_alloc_ptrs + alloc_idx,
               alloc_ptrs + alloc_idx + 1,
               (allocs_count - alloc_idx - 1) * sizeof(void*));
        free(alloc_ptrs);
        alloc_ptrs = new_alloc_ptrs;

        allocs_count -= 1;

        return true;
    }

    failed_frees =
      realloc(failed_frees, (failed_frees_count + 1) * sizeof(DFreeInfo));
    failed_frees[failed_frees_count] = info;
    failed_frees_count += 1;

    return false;
}

char*
dalloc_report()
{
    if (report_str)
        return report_str;

    const char* filename_title = "File";
    const char* lineno_title = "Line";
    const char* nitems_title = "Item Count";
    const char* size_title = "Type Size";
    const char* combined_size_title = "Total Size";
    const char* typename_title = "Type";

    size_t largest_filename_len = strlen(filename_title);
    size_t most_lineno_digits = strlen(lineno_title);
    size_t most_nitems_digits = strlen(nitems_title);
    size_t most_size_digits = strlen(size_title);
    size_t most_combined_size_digits = strlen(combined_size_title);
    size_t largest_typename_len = strlen(typename_title);

    bool issue_found =
      allocs_count > 0 || failed_allocs_count > 0 || failed_frees_count > 0;

    long long unsigned int total_allocated_bytes = 0;
    long long unsigned int total_leaked_bytes = 0;

    for (size_t a = 0; a < freed_allocs_count; a++) {
        total_allocated_bytes += freed_allocs[a].nitems * freed_allocs[a].size;
    }

    for (size_t a = 0; a < allocs_count; a++) {
        size_t filename_len = strlen(allocs[a].filename);
        size_t lineno_digits = snprintf(NULL, 0, "%d", allocs[a].lineno);
        size_t nitems_digits = snprintf(NULL, 0, "%zu", allocs[a].nitems);
        size_t size_digits = snprintf(NULL, 0, "%zu", allocs[a].size);
        size_t combined_size_digits =
          snprintf(NULL, 0, "%zu", allocs[a].nitems * allocs[a].size);
        size_t typename_len = strlen(allocs[a].typename);

        if (largest_filename_len < filename_len)
            largest_filename_len = filename_len;
        if (most_lineno_digits < lineno_digits)
            most_lineno_digits = lineno_digits;
        if (most_nitems_digits < nitems_digits)
            most_nitems_digits = nitems_digits;
        if (most_size_digits < size_digits)
            most_size_digits = size_digits;
        if (most_combined_size_digits < combined_size_digits)
            most_combined_size_digits = combined_size_digits;
        if (largest_typename_len < typename_len)
            largest_typename_len = typename_len;

        total_allocated_bytes += allocs[a].nitems * allocs[a].size;
        total_leaked_bytes += allocs[a].nitems * allocs[a].size;
    }

    for (size_t a = 0; a < failed_allocs_count; a++) {
        size_t filename_len = strlen(failed_allocs[a].filename);
        size_t lineno_digits = snprintf(NULL, 0, "%d", failed_allocs[a].lineno);
        size_t nitems_digits =
          snprintf(NULL, 0, "%zu", failed_allocs[a].nitems);
        size_t size_digits = snprintf(NULL, 0, "%zu", failed_allocs[a].size);
        size_t combined_size_digits = snprintf(
          NULL, 0, "%zu", failed_allocs[a].nitems * failed_allocs[a].size);
        size_t typename_len = strlen(failed_allocs[a].typename);

        if (largest_filename_len < filename_len)
            largest_filename_len = filename_len;
        if (most_lineno_digits < lineno_digits)
            most_lineno_digits = lineno_digits;
        if (most_nitems_digits < nitems_digits)
            most_nitems_digits = nitems_digits;
        if (most_size_digits < size_digits)
            most_size_digits = size_digits;
        if (most_combined_size_digits < combined_size_digits)
            most_combined_size_digits = combined_size_digits;
        if (largest_typename_len < typename_len)
            largest_typename_len = typename_len;
    }

    for (size_t a = 0; a < failed_frees_count; a++) {
        size_t filename_len = strlen(failed_frees[a].filename);
        size_t lineno_digits = snprintf(NULL, 0, "%d", failed_frees[a].lineno);

        if (largest_filename_len < filename_len)
            largest_filename_len = filename_len;
        if (most_lineno_digits < lineno_digits)
            most_lineno_digits = lineno_digits;
    }

    int summary_str_len =
      snprintf(0,
               0,
               "%llu bytes allocated in total, with %llu leaked.",
               total_allocated_bytes,
               total_leaked_bytes);

    char* summary_str = malloc(summary_str_len + 1);
    snprintf(summary_str,
             summary_str_len + 1,
             "%llu bytes allocated in total, with %llu leaked.",
             total_allocated_bytes,
             total_leaked_bytes);

    append_to_report(summary_str);

    free(summary_str);

    if (!issue_found)
        return report_str;

    int title_fmt_str_len = snprintf(
      0,
      0,
      "| %%14s | %%%zus | %%%zus | %%%zus | %%%zus | %%%zus | %%%zus |",
      largest_filename_len,
      most_lineno_digits,
      largest_typename_len,
      most_nitems_digits,
      most_size_digits,
      most_combined_size_digits);

    char* title_fmt_str = malloc(title_fmt_str_len + 1);
    snprintf(title_fmt_str,
             title_fmt_str_len + 1,
             "| %%14s | %%%zus | %%%zus | %%%zus | %%%zus | %%%zus | %%%zus |",
             largest_filename_len,
             most_lineno_digits,
             largest_typename_len,
             most_nitems_digits,
             most_size_digits,
             most_combined_size_digits);

    int row_fmt_str_len = snprintf(
      0,
      0,
      "| %%14s | %%%zus | %%%zud | %%%zus | %%%zud | %%%zud | %%%zud |",
      largest_filename_len,
      most_lineno_digits,
      largest_typename_len,
      most_nitems_digits,
      most_size_digits,
      most_combined_size_digits);

    char* row_fmt_str = malloc(row_fmt_str_len + 1);
    snprintf(row_fmt_str,
             row_fmt_str_len + 1,
             "| %%14s | %%%zus | %%%zud | %%%zus | %%%zud | %%%zud | %%%zud |",
             largest_filename_len,
             most_lineno_digits,
             largest_typename_len,
             most_nitems_digits,
             most_size_digits,
             most_combined_size_digits);

    int title_str_len = snprintf(0,
                                 0,
                                 title_fmt_str,
                                 "Issue",
                                 filename_title,
                                 lineno_title,
                                 typename_title,
                                 nitems_title,
                                 size_title,
                                 combined_size_title);
    char* title_str = malloc(title_str_len + 1);
    snprintf(title_str,
             title_str_len + 1,
             title_fmt_str,
             "Issue",
             filename_title,
             lineno_title,
             typename_title,
             nitems_title,
             size_title,
             combined_size_title);

    append_to_report("\n+");
    for (int c = 1; c < title_str_len - 1; c++) {
        append_to_report("=");
    }
    append_to_report("+\n");
    append_to_report(title_str);
    append_to_report("\n+");
    for (int c = 1; c < title_str_len - 1; c++) {
        append_to_report("-");
    }
    append_to_report("+\n");

    int row_str_len = snprintf(0, 0, row_fmt_str, "", "", 0, "", 0, 0, 0);
    char* row_str = malloc(row_str_len + 1);

    for (size_t a = 0; a < allocs_count; a++) {
        snprintf(row_str,
                 row_str_len + 1,
                 row_fmt_str,
                 "LEAKED ALLOC",
                 allocs[a].filename,
                 allocs[a].lineno,
                 allocs[a].typename,
                 allocs[a].nitems,
                 allocs[a].size,
                 allocs[a].size * allocs[a].nitems);

        append_to_report(row_str);
        append_to_report("\n");
    }

    for (size_t a = 0; a < failed_allocs_count; a++) {
        snprintf(row_str,
                 row_str_len + 1,
                 row_fmt_str,
                 "FAILED ALLOC",
                 failed_allocs[a].filename,
                 failed_allocs[a].lineno,
                 failed_allocs[a].typename,
                 failed_allocs[a].nitems,
                 failed_allocs[a].size,
                 failed_allocs[a].size * failed_allocs[a].nitems);

        append_to_report(row_str);
        append_to_report("\n");
    }

    for (size_t a = 0; a < failed_frees_count; a++) {
        snprintf(row_str,
                 row_str_len + 1,
                 row_fmt_str,
                 "FAILED FREE",
                 failed_frees[a].filename,
                 failed_frees[a].lineno,
                 "",
                 0,
                 0,
                 0);

        append_to_report(row_str);
        append_to_report("\n");
    }

    append_to_report("+");
    for (int c = 1; c < title_str_len - 1; c++) {
        append_to_report("=");
    }
    append_to_report("+");

    free(title_str);
    free(row_str);
    free(title_fmt_str);
    free(row_fmt_str);

    for (int a = 0; a < allocs_count; a++) {
        bool duplicate = false;
        for (int b = 0; b < a && !duplicate; b++) {
            duplicate = alloc_ptrs[a] == alloc_ptrs[b];
        }

        if (duplicate)
            continue;

        free(alloc_ptrs[a]);
    }

    return report_str;
}

void
append_to_report(const char* str)
{
    size_t len = strlen(str);

    report_str = realloc(report_str, len + report_str_len + 1);
    memcpy(report_str + report_str_len, str, len);
    report_str[len + report_str_len] = 0;

    report_str_len += len;
}

void
dalloc_cleanup()
{
    allocs_count = 0;
    freed_allocs_count = 0;
    failed_allocs_count = 0;
    failed_frees_count = 0;
    report_str_len = 0;
    free(allocs);
    free(alloc_ptrs);
    free(freed_allocs);
    free(failed_allocs);
    free(failed_frees);
    free(report_str);
}

#endif
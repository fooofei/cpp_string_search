
#ifndef CPP_STRING_SEARCH_STRING_SEARCH_H
#define CPP_STRING_SEARCH_STRING_SEARCH_H

size_t
boyermoore_horspool_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size);

size_t
sunday_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size);

size_t
rabin_karp_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size);

size_t
zzl_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size);

size_t
kmp_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size);

#endif //CPP_STRING_SEARCH_STRING_SEARCH_H

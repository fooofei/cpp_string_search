

#include <cstring> // std::memchr
#include <limits.h> // UCHAR_MAX
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <vector>

#include "../include/string_search/string_search.h"

//
// horspool 跟 sunday 的区别在于不匹配时，选取哪个字符做跳转:
//     horspool: 选取当前匹配的最后一个字符
//     sunday: 选取当前匹配的最后字符的下一个字符
//
//

// if find, return the off in text, if not find, return the text_size.

size_t
boyermoore_horspool_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size)
{

    const unsigned char* begin = (const unsigned char*)text;
    const unsigned char* end = begin + text_size;
    const unsigned char* begin_pattern = (const unsigned char*)pattern;
    const unsigned char* end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size)) {
        return text_size;
    }

    if (pattern_size == 1) {
        const unsigned char* r = (const unsigned char*)std::memchr(begin, *begin_pattern, text_size);
        return r ? size_t(r - begin) : text_size;
    }

    std::vector<size_t> shift_table(UCHAR_MAX + 1, pattern_size);
    size_t pattern_size_minus_1 = pattern_size - 1;
    for (size_t a = 0; a < pattern_size_minus_1; ++a) {
        shift_table[begin_pattern[a]] = pattern_size_minus_1 - a;
    }

    unsigned char pattern_end_char = *(end_pattern - 1);
    pattern_size -= 1;

    for (const unsigned char* ptr = begin + pattern_size; ptr < end;) {
        if (*ptr == pattern_end_char
            && 0 == std::memcmp(begin_pattern, ptr - pattern_size, pattern_size)) {
            return ptr - begin - pattern_size;
        }
        size_t shift = shift_table[*ptr];
        //assert(shift);
        ptr += shift;
    }
    return text_size;
}

// shift table 规则：
// 一个字符在 patt 中没出现过，则默认为 pattLen + 1
// 一个字符在 patt 中出现过，则计算为 到右边界到距离 + 1
// ptr 单个字符一旦匹配，则向前匹配全部，
// 一旦不匹配，就按照 shift table 跳转
// if find, return the off in text, if not find, return the text_size.
size_t
sunday_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size)
{

    const unsigned char* begin = (const unsigned char*)text;
    const unsigned char* end = begin + text_size;
    const unsigned char* begin_pattern = (const unsigned char*)pattern;
    const unsigned char* end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size)) {
        return text_size;
    }

    if (pattern_size == 1) {
        const unsigned char* r = (const unsigned char*)std::memchr(begin, *begin_pattern, text_size);
        return r ? size_t(r - begin) : text_size;
    }

    std::vector<size_t> shift_table(UCHAR_MAX + 1, pattern_size + 1);

    for (size_t a = 0; a < pattern_size; ++a) {
        shift_table[begin_pattern[a]] = pattern_size - a;
    }

    unsigned char pattern_end_char = *(end_pattern - 1);
    for (const unsigned char* ptr = begin + pattern_size; ptr <= end;) {
        if (*(ptr - 1) == pattern_end_char
            && 0 == std::memcmp(begin_pattern, ptr - pattern_size, pattern_size)) {
            return ptr - begin - pattern_size;
        }
        if (ptr == end) {
            break;
        }
        size_t shift = shift_table[*ptr];
        //assert(shift);
        ptr += shift;
    }
    return text_size;
}

// http://www.geeksforgeeks.org/searching-for-patterns-set-3-rabin-karp-algorithm/

inline int
hash_characters(const int d, const int q, int value, unsigned char c)
{
    return (d * value + c) % q;
}

size_t
rabin_karp_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size)
{
    const unsigned char* begin = (const unsigned char*)text;
    const unsigned char* end = begin + text_size;
    const unsigned char* begin_pattern = (const unsigned char*)pattern;
    const unsigned char* end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size)) {
        return text_size;
    }

    int hash_remove_base = 1; // for remove one characters from hash
    int d = 256; // d is the number of characters in input alphabet
    int q = 101; // a prime number

    // value of h would be pow(d, pattern_size-1)%q
    for (size_t i = 0; i < pattern_size - 1; ++i) {
        hash_remove_base = (hash_remove_base * d) % q;
    }

    int hash_pattern = 0;
    int hash_text = 0;

    for (const unsigned char *p1 = begin_pattern,
                             *p2 = begin;
         p1 < end_pattern; ++p1, ++p2) {
        hash_pattern = hash_characters(d, q, hash_pattern, *p1);
        hash_text = hash_characters(d, q, hash_text, *p2);
    }

    for (const unsigned char* ptr = begin + pattern_size; ptr <= end; ++ptr) {
        if (hash_pattern == hash_text && 0 == std::memcmp(begin_pattern, ptr - pattern_size, pattern_size)) {
            return ptr - begin - pattern_size;
        }

        if (ptr == end) {
            break;
        }

        hash_text = hash_characters(d, q,
            hash_text - (*(ptr - pattern_size)) * hash_remove_base,
            *ptr);

        for (; hash_text < 0;) {
            hash_text = hash_text + q;
        }
    }
    return text_size;
}

size_t
zzl_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size)
{
    typedef const uint8_t* const_pointer_t;
    const_pointer_t begin = (const_pointer_t)text;
    const_pointer_t end = begin + text_size;
    const_pointer_t begin_pattern = (const_pointer_t)pattern;
    const_pointer_t end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size)) {
        return text_size;
    }

    std::vector<const_pointer_t> array_next;
    for (const_pointer_t p = begin, e = end - pattern_size + 1; p < e; ++p) {
        if (*p == *begin_pattern) {
            array_next.push_back(p);
        }
    }

    for (size_t i = 0; i < array_next.size(); ++i) {
        const_pointer_t p = array_next[i];
        if (std::equal(begin_pattern, end_pattern, p)) {
            return p - begin;
        }
    }

    return text_size;
}

/* return 0 if success */
int build_prefix_table(const void* pattern, size_t pattern_size, std::vector<size_t>& table)
{
    table.clear();

    if (!(pattern && pattern_size)) {
        return -1;
    }
    table.resize(pattern_size);
    typedef const uint8_t* const_pointer_t;
    const_pointer_t first = (const uint8_t*)pattern;

    // first of table permanently 0
    if (table.size() > 0) {
        table[0] = 0;
    }
    if (table.size() > 1) {
        for (size_t off = 1; off < table.size(); off += 1) {
            table[off] = 0;
            size_t off1 = off;
            for (;;) {
                off1 = table[off1 - 1];
                off1 += 1;

                const_pointer_t f1 = first;
                const_pointer_t l1 = f1 + off1;
                const_pointer_t l2 = first + off + 1;
                const_pointer_t f2 = l2 - (size_t)std::distance(f1, l1);
                if (std::equal(f1, l1, f2)) {
                    table[off] = off1;
                    break;
                }
                if (off1 < 2) {
                    break;
                }
            }
        }
    }
    return 0;
}

size_t
kmp_memmem(const void* text, size_t text_size,
    const void* pattern, size_t pattern_size)
{
    if (!(text && text_size)) {
        return text_size;
    }
    if (!(pattern && pattern_size)) {
        return pattern_size;
    }

    std::vector<size_t> table;

    if (0 != build_prefix_table(pattern, pattern_size, table)) {
        return text_size;
    }

    const uint8_t* f1 = (const uint8_t*)text;
    const uint8_t* l1 = f1 + text_size;
    const uint8_t* f2 = (const uint8_t*)pattern;
    const uint8_t* off_ptr_text;
    size_t off_pattern;
    for (off_pattern = 0, off_ptr_text = f1; off_ptr_text < l1;) {
        if (f2[off_pattern] == *off_ptr_text) {
            off_pattern += 1;
            off_ptr_text += 1;
            if (off_pattern == pattern_size) {
                return (size_t)std::distance(f1, off_ptr_text) - pattern_size;
            }
        } else if (off_pattern > 0) {
            off_pattern = table[off_pattern - 1];
        } else {
            off_ptr_text += 1;
        }
    }

    return text_size;
}

// Sunday 算法的 C 语言实现
// 当参数中不包含字符串长度的时候，这种写法需要时刻判断是否结束了，比较复杂
int Sunday(const char* str, const char* patt)
{
    if (str == NULL || patt == NULL) {
        return -1;
    }
    if (*patt == 0) {
        return 0;
    }

    int pattLen = strlen(patt);

    int* shift = (int*)calloc(0x100, sizeof(int));

    for (int i = 0; i < 0x100; i++) {
        shift[i] = pattLen + 1;
    }
    for (int i = 0; i < pattLen; i++) {
        uint8_t u8 = patt[i];
        shift[u8] = pattLen - i;
    }

    // 没有多余的变量记录 str 长度，导致总是依赖迭代判断 '\0'
    // 看到下面的代码很长
    char pattEnd = patt[pattLen - 1];
    const char* p = str;
    for (int i = pattLen - 1; *p != 0 && i > 0; i--, p++) {
    }
    if (*p == 0) {
        return -1;
    }
    p += 1;

    for (;;) {
        if (*(p - 1) == pattEnd && 0 == memcmp(p - pattLen, patt, pattLen)) {
            free(shift);
            return p - pattLen - str;
        }
        if (*p == 0) {
            break;
        }
        int step = shift[(uint8_t)(*p)] - 1;
        for (int i = 0; i < step && *p != 0; i++) {
            p++;
        }
        if (*p == 0) {
            break;
        }
        p++;
    }

    free(shift);
    shift = NULL;

    return -1;
}

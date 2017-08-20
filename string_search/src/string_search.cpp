

#include <stdio.h>
#include <vector>
#include <map>

#include "../include/string_search/string_search.h"

//
// horspool 跟 sunday 的区别在于不匹配时，选取哪个字符做跳转:
//     horspool: 选取当前匹配的最后一个字符
//     sunday: 选取当前匹配的最后字符的下一个字符
//
//

// if find, return the off in text, if not find, return the text_size.

size_t boyermoore_horspool_memmem(const void * text, size_t text_size,
                                  const void * pattern, size_t pattern_size)
{

    const unsigned char * begin = (const unsigned char *)text;
    const unsigned char * end = begin + text_size;
    const unsigned char * begin_pattern = (const unsigned char *)pattern;
    const unsigned char * end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size))
    {
        return text_size;
    }

    if (pattern_size == 1)
    {
        const unsigned char * r = (const unsigned char *)std::memchr(begin,*begin_pattern,text_size);
        return r ? size_t(r - begin) : text_size;
    }

    std::vector<size_t> shift_table(UCHAR_MAX+1,pattern_size);
    size_t pattern_size_minus_1 = pattern_size-1;
    for (size_t a=0 ; a<pattern_size_minus_1 ; ++a)
    {
        shift_table[begin_pattern[a]]= pattern_size_minus_1 - a;
    }

    unsigned char pattern_end_char = *(end_pattern-1);
    pattern_size -= 1;

    for (const unsigned char * ptr= begin + pattern_size ; ptr < end ; )
    {
        if (*ptr == pattern_end_char 
            && 0 == std::memcmp(begin_pattern,ptr-pattern_size,pattern_size))
        {
            return ptr-begin-pattern_size;
        }
        size_t shift = shift_table[*ptr];
        //assert(shift);
        ptr += shift;
    }
    return text_size;
}


// if find, return the off in text, if not find, return the text_size.

size_t sunday_memmem(const void * text, size_t text_size,
                                  const void * pattern, size_t pattern_size)
{

    const unsigned char * begin = (const unsigned char *)text;
    const unsigned char * end = begin + text_size;
    const unsigned char * begin_pattern = (const unsigned char *)pattern;
    const unsigned char * end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size))
    {
        return text_size;
    }

    if (pattern_size == 1)
    {
        const unsigned char * r = (const unsigned char *)std::memchr(begin,*begin_pattern,text_size);
        return r ? size_t(r - begin) : text_size;
    }

    std::vector<size_t> shift_table(UCHAR_MAX+1,pattern_size+1);

    for (size_t a = 0; a<pattern_size;++a)
    {
        shift_table[begin_pattern[a]]=pattern_size-a;
    }
    
    unsigned char pattern_end_char = *(end_pattern-1);
    for (const unsigned char * ptr= begin + pattern_size ; ptr < end ; )
    {
        if (*(ptr-1) == pattern_end_char 
            && 0 == std::memcmp(begin_pattern,ptr-pattern_size,pattern_size))
        {
            return ptr-begin-pattern_size;
        }
        size_t shift = shift_table[*ptr];
        //assert(shift);
        ptr += shift;
    }
    return text_size;
}

// http://www.geeksforgeeks.org/searching-for-patterns-set-3-rabin-karp-algorithm/

inline int hash_characters(const int d, const int q, int  value, unsigned char c)
{
    return (d*value + c) % q;
}

size_t rabin_karp_memmem(const void * text, size_t text_size,
                         const void * pattern, size_t pattern_size)
{
    const unsigned char * begin = (const unsigned char *)text;
    const unsigned char * end = begin + text_size;
    const unsigned char * begin_pattern = (const unsigned char *)pattern;
    const unsigned char * end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size))
    {
        return text_size;
    }

    int hash_remove_base = 1; // for remove one characters from hash
    int d = 256; // d is the number of characters in input alphabet
    int q = 101; // a prime number

    // value of h would be pow(d, pattern_size-1)%q
    for (size_t i=0;i<pattern_size-1;++i)
    {
        hash_remove_base = (hash_remove_base*d)%q;
    }

    int hash_pattern = 0;
    int hash_text = 0;

    for (const unsigned char * p1 = begin_pattern,
        * p2 = begin
        ;p1<end_pattern;++p1,++p2)
    {
        hash_pattern = hash_characters(d,q,hash_pattern,*p1);
        hash_text = hash_characters(d,q,hash_text,*p2);
    }


    for (const unsigned char * ptr= begin + pattern_size ; ptr < end ; ++ptr)
    {
        if (hash_pattern == hash_text &&
            0 == std::memcmp(begin_pattern,ptr-pattern_size,pattern_size)
            )
        {
            return ptr-begin-pattern_size;
        }

        hash_text = hash_characters(d,q,
            hash_text-(*(ptr-pattern_size))*hash_remove_base,
            *ptr);

        while(hash_text<0)
        {
            hash_text = hash_text + q;
        }
    }
    return text_size;
}

size_t zzl_memmem(const void * text, size_t text_size,
                  const void * pattern, size_t pattern_size)
{
    const unsigned char * begin = (const unsigned char *)text;
    const unsigned char * end = begin + text_size;
    const unsigned char * begin_pattern = (const unsigned char *)pattern;
    const unsigned char * end_pattern = begin_pattern + pattern_size;

    if (!(begin && begin_pattern && text_size && pattern_size && pattern_size <= text_size))
    {
        return text_size;
    }

    std::vector<const unsigned char *> array_next;
    for (const unsigned char * p = begin , *e = end-pattern_size;p<e;++p)
    {
        if (*p == *begin_pattern)
        {
            array_next.push_back(p);
        }
    }

    for (size_t i=0;i<array_next.size();++i)
    {
        const unsigned char * p = array_next[i];
        if (0 == std::memcmp(p,begin_pattern,pattern_size))
        {
            return p-begin;
        }
    }

    return text_size;
}

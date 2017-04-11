

#include <stdio.h>
#include <vector>
#include <map>
#include <windows.h>
#include <assert.h>

#include "wumanber_search.h"
#include "unit_test_macro.h"



// if find, return the off in text, if not find, return the text_size.
typedef size_t (*pfn_memmem)(const void * text, size_t text_size,
                     const void * pattern, size_t pattern_size);

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
        assert(shift);
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
        assert(shift);
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


#include <iostream>
#include <fstream>
#include <string>


void test_single_pattern(pfn_memmem func_memmem, const char * func_name)
{
    std::vector<std::pair<std::string, std::string> > texts;


    texts.push_back(std::make_pair("substring searching","search"));
    texts.push_back(std::make_pair("CPM_annual_conference_announce","announce"));
    texts.push_back(std::make_pair("tartarget","target"));
    texts.push_back(std::make_pair("",""));
    texts.push_back(std::make_pair("source","rced"));
    texts.push_back(std::make_pair("source","a"));
    texts.push_back(std::make_pair("abcdefghijklmnopqrst","aaaa"));
    texts.push_back(std::make_pair("abcdefghijklmnopqrst","ghij"));
    texts.push_back(std::make_pair("abcdefghijklmnopqrst","abcdefg"));
    texts.push_back(std::make_pair("abcdefghijklmnopqrst","pqrst"));
    texts.push_back(std::make_pair("abcdefghijkl我擦mnopqrst","我擦"));


    for (size_t i=0;i<texts.size();++i)
    {
        const std::string & s1 = texts[i].first;
        const std::string & s2 = texts[i].second;

        size_t off = func_memmem(s1.c_str(),s1.size(),s2.c_str(),s2.size());
        if (off < s1.size())
        {
            EXPECT(0==memcmp(s1.c_str()+off,s2.c_str(),s2.size()));
        }
        printf("[%2d]%s ->[%d]at %d ->%s\n",i,s1.c_str(),s1.size(),off,s2.c_str());
    }
    printf("%s : 100%% pass\n",func_name);
}

void test_hash()
{
    size_t H = 0x10000;
    const unsigned char p[] ="ab";
    DWORD h1= ((DWORD)(H-1) & ( (*(p)<<8) | (*(p+1)))) ;

    DWORD h2 = *((const unsigned short *)p);

    printf("hash1 %d, hash2 %d\n",h1,h2);
}


int main(int argc, const TCHAR ** argv)
{
    test_single_pattern(boyermoore_horspool_memmem, "boyermoore_horspool");
    printf("\n");
    test_single_pattern(sunday_memmem, "sunday");
    printf("\n");
    test_single_pattern(rabin_karp_memmem, "rabin_karp");

    printf("\n");
    //test_wumanber();
   
    return 0;
}
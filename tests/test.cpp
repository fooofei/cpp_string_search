

#include <map>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>// std::memset ...
#include <string>
#include "string_search/string_search.h"
#include "string_search/wumanber_search.h"
#include "string_search/ac_search.h"
#include "unit_test_macro.h"


#ifndef _countof
#define _countof(x) (sizeof((x))/sizeof((x[0])))
#endif

static
size_t distance_pointer(const void * b, const void * e)
{
    return ((const unsigned char *)e-(const unsigned char *)b);
}


struct result_t
{
    // match which pattern(index), which content(std::string) at where(pos).
    typedef std::map<size_t, std::pair<size_t, std::string> > result_type;
    result_type check_r;
    result_type hit_r;

    void fprintf_result(FILE * f)
    {
        size_t i = 0;
        for (result_type::const_iterator it = check_r.begin(); it != check_r.end();++it,++i)
        {
            fprintf(f,"    [%lu]at(%lu),pattern_index(%lu),pattern(%s)\n",i,it->second.first,it->first,it->second.second.c_str());
        }
    }
    void clear()
    {
        hit_r.clear();
    }
};

static void callback_hit_pattern_strings(void * context,
                                        bool * is_continue,
                                        size_t off,
                                        size_t index, const void * begin, const void * end)
{
    struct result_t * results = (struct result_t *)context;
    std::string s((const char *)begin,distance_pointer(begin,end));
    results->hit_r[index] = std::make_pair(off,s);
    *is_continue = true;
}

void readlines_from_file(const char * name, std::vector<std::string> & container)
{
    std::ifstream f(name,std::ios::in | std::ios::binary);

    for (std::string line; f.peek() != EOF;)
    {
        std::getline(f, line);
        if (!line.empty())
        {
            container.push_back(line);
        }
    }

}


// how to print utf-8 on Windows is a question, not through the encoding convert.

template<typename search_t>
void test_search(const char * name, search_t & search)
{
    int hr;
    std::vector<std::string> patterns;
    std::vector<std::string> texts;
    size_t index = size_t(-1);


    readlines_from_file("test_wumanber.txt", patterns);
    readlines_from_file("test_text.txt", texts);

    // only add this


    std::vector<struct result_t> results;
    results.resize(texts.size());

   
    for (size_t patt_index = 0; patt_index < patterns.size(); ++patt_index)
    {
        const std::string & patt = patterns[patt_index];
        index = size_t(-1);
        search.push_pattern(patt.c_str(), patt.c_str() + patt.size(), &index);

        EXPECT(index == patt_index);
        for (size_t j = 0; j < results.size(); ++j)
        {
            const std::string & t = texts[j];
            size_t off = t.find(patt);
            if (off < t.size())
            {
                results[j].check_r[patt_index] = std::make_pair(off, patt);
            }
        }
    }

    hr = search.init();
    EXPECT(hr == S_OK);

    for (size_t i = 0; i < results.size(); ++i)
    {
        const std::string & t = texts[i];
        result_t & r = results[i];
        r.clear();
        hr = search.search(t.c_str(), t.c_str() + t.size(), callback_hit_pattern_strings, &r);
        EXPECT(hr == S_OK);
        EXPECT(r.check_r == r.hit_r);
        printf("%s::[%lu]text(%s) hits count(%lu)", name, i, t.c_str(),r.hit_r.size());
        printf("\n");
        r.fprintf_result(stdout);
        printf("\n");

    }

    printf("pass %s\n",name);
}

void test_wumanber_ac()
{
    wumanber_search_t wumanber;
    ac_search_t ac;
    test_search("wumanber", wumanber);
    test_search("ac", ac);
}




void test_single_pattern(size_t (*func_memmem)(const void * , size_t ,const void * , size_t ),
                         const char * func_name)
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
            EXPECT(0==std::memcmp(s1.c_str()+off,s2.c_str(),s2.size()));
        }
        printf("[%2lu]%s ->[%lu]at %lu ->%s\n",i,s1.c_str(),s1.size(),off,s2.c_str());
    }
    printf("%s : 100%% pass\n",func_name);
}


static
uint32_t _hash1(const void * p)
{
    const unsigned char * pb = (const unsigned char *)p;
    return ( (*(pb-2)<<8) | (*(pb-1)));
}
static
uint32_t _hash2(const void * p)
{
    const uint8_t * pb=(uint8_t *)p;

    union{
        struct{
            uint8_t v0;
            uint8_t v1;
            uint8_t v2;
            uint8_t v3;
        };
        uint32_t reserve;
    }v;
    memset(&v,0,sizeof(v));
    v.v0=*(pb-1);
    v.v1=*(pb-2);
    return v.reserve;
}

void test_hash2()
{
    const char * p = "ahfiafjsdofajfoaewfjoafas;djviasdfasfoeieowefjoiejoe";

    const char * p_end = p+strlen(p);

    for(const char * off=p+2;off<p_end;++off){
        uint32_t h1 = _hash1(off);
        uint32_t h2 = _hash2(off);
        EXPECT(h1== h2);
    }

}


int main()
{
    test_single_pattern(boyermoore_horspool_memmem, "boyermoore_horspool");
    printf("\n");
    test_single_pattern(sunday_memmem, "sunday");
    printf("\n");
    test_single_pattern(rabin_karp_memmem, "rabin_karp");
    printf("\n");
    test_single_pattern(zzl_memmem,"zzl");
    printf("\n");

    test_wumanber_ac();

    test_hash2();
    printf("pass all\n");
    return 0;
}

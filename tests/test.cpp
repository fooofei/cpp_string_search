


#include "string_search/wumanber_search.h"
#include "unit_test_macro.h"


#include <fstream>

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
    typedef std::map<size_t, std::string> result_type;
    result_type check_r;
    result_type hit_r;

    void fprintf_result(FILE * f)
    {
        for (result_type::const_iterator it = check_r.begin(); it != check_r.end();++it)
        {
            fprintf(f,"[%lu]%s ",it->first,it->second.c_str());
        }
    }
};

static void callback_hit_pattern_strings(void * context,
                                        bool * is_continue,
                                        size_t off,
                                        size_t index, const void * begin, const void * end)
{
    struct result_t * results = (struct result_t *)context;
    std::string s((const char *)begin,distance_pointer(begin,end));
    results->hit_r[index] = s;
    *is_continue = true;
}

void test_wumanber()
{
    std::ifstream f("test_wumanber.txt");
    int hr;
    std::vector<std::string> patterns;
    wumanber_search_t wumanber;
    size_t index=0;


    // only add this
    std::string texts[]=
            {
                    "这是一个非法网站www.game8118.com/，请冻结。",
                    "this is an invalid url:www.game8118.com/, please lock it.",
                    "www.game8118.com/1",
                    "www.soku.com/测试以某个pattern开头",
                    "测试以某个pattern结束www.sosocxw.com/",
                    "测试中文搜索abcdefg",
                    "abcdefg测试中文搜索",
                    "测试pattern重复.1000tuan.com/",
                    ".1000tuan.com/测试pattern重复",
                    "测试多个pattern,.51buy.com/,测试多个pattern.ftuan.com/",
            };

    std::vector<struct result_t> results;
    results.resize(_countof(texts));

    for (std::string line;f.peek() != EOF;)
    {
        std::getline(f,line);
        if (!line.empty())
        {
            patterns.push_back(line);
        }
    }

    for (size_t i = 0; i< patterns.size() ; ++i)
    {
        const std::string & s = patterns[i];
        wumanber.push_pattern(s.c_str(),s.c_str()+s.size(),&index);

        for (size_t i=0;i< results.size();++i)
        {
            const std::string & t= texts[i];
            if (t.find(s) < t.size())
            {
                results[i].check_r[index] =s;
            }
        }
    }

    hr = wumanber.init();
    //assert(hr == S_OK);
    EXPECT(hr == S_OK);

    for (size_t i=0; i< results.size(); ++i)
    {
        const std::string & t = texts[i];
        hr = wumanber.search(t.c_str(),t.c_str()+t.size(),callback_hit_pattern_strings,&results[i]);
        EXPECT(hr == S_OK);
        EXPECT(results[i].check_r == results[i].hit_r);
        printf("[%2lu]%s ->hit at->",i,t.c_str());
        results[i].fprintf_result(stdout);
        printf("\n");
    }

    printf("wumanber : 100%% pass\n");
}


#include <iostream>
#include <fstream>
#include <string>
#include "string_search/string_search.h"

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
            EXPECT(0==memcmp(s1.c_str()+off,s2.c_str(),s2.size()));
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

    test_wumanber();

    test_hash2();
    printf("pass all\n");
    return 0;
}

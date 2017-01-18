
#include "stdafx.h"
#include "wumanber_search.h"
#include <assert.h>

size_t wumanber_search_t::table_size(size_t pattern_size, size_t pattern_min_size)
{
    size_t primes[] = {1003, 10007, 100003, 1000003, 10000019, 100000007, 0};

    size_t threshold = 10 * pattern_min_size;
    for ( size_t * p = primes; *p ; ++p)
    {
        if (*p > pattern_size && *p / pattern_size > threshold)
        {
            return *p;
        }
    }
    return primes[_countof(primes)-1];
}

DWORD wumanber_search_t::_hash(size_t B, size_t H, const void * p)
{
    const unsigned char * pb = (const unsigned char *)p;
    switch (B)
    {
    case 2:
        return ((DWORD)(H-1) & ( (*(pb-2)<<8) | (*(pb-1)))) ; // 这里也可以变
    case 3: 
        return ((DWORD)(H-1) & (((*(pb-3))<<16) | (*(pb-2))<<8) | (*(pb-1)));
    default: return 0;
    }
}

HRESULT wumanber_search_t::push_pattern(const void * begin, const void * end, size_t * index)
{
    if(!(begin && end && end>begin)) return E_INVALIDARG;

    if (index)
    {
        *index = patterns_pointer_.size();
    }
    patterns_pointer_.push_back(std::make_pair(begin,end));

    const unsigned char * b = (const unsigned char *)begin;
    const unsigned char * e = (const unsigned char *)end;
    size_t l = (e-b);

    if (B<=l) // 过短的字符就放入其他的表中，不然非常影响跳转的性能
    {
        min_pattern_size_ = (std::min<size_t>)(min_pattern_size_,l);
    }
    return S_OK;
}

size_t wumanber_search_t::pattern_count() const 
{
    return patterns_pointer_.size();
}

HRESULT wumanber_search_t::init()
{
    size_t count = pattern_count();

    if(!(count>0)) return E_INVALIDARG;

    assert(B <= min_pattern_size_);

    size_t H =  table_size(count,min_pattern_size_); //1<<12; // 4096

    shfit_table_.resize(H,min_pattern_size_ - B +1);
    hash_table_.resize(H,count);
    hash_chain_table_.resize(count);
    prefix_table_.resize(count);


    for (size_t i =0; i< count;++i)
    {
        const unsigned char * begin = (const unsigned char *)patterns_pointer_[i].first;
        const void * end = patterns_pointer_[i].second;
        DWORD h ;
        size_t l = distance_pointer(begin,end);
        if (B<=l)
        {
            for (size_t j = min_pattern_size_; j >= B; --j)
            {
                h = _hash(B,shfit_table_.size(),begin+j);
                shfit_table_[h] = (std::min<size_t>)(min_pattern_size_-j,shfit_table_[h]);
            }

            h = _hash(B,shfit_table_.size(),begin+min_pattern_size_);
            hash_chain_table_[i] = hash_table_[h];
            hash_table_[h] = i;

            h = _hash(B,shfit_table_.size(),begin+B);
            prefix_table_[i]=h;
        }
        else
        {
            unsigned short c;
            switch (l)
            {
            case 1:
                if(byte_table_.empty()) byte_table_.resize(0x100,count);
                hash_chain_table_[i] = byte_table_[*begin];
                byte_table_[*begin] = i;
                byte_table_count_ += 1;
                break;
            case 2:
                if(short_table_.empty()) short_table_.resize(0x10000,count);
                c = *(const unsigned short *)begin;
                hash_chain_table_[i] = short_table_[c];
                short_table_[c]=i;
                short_table_count_ += 1;
                break;
            default: return E_NOTIMPL;
            }
        }
    }

    return S_OK;
}

#define WUMANBER_HIT \
    do {\
        patterns_hit[index] = true;\
        ++count_hit;\
        int r = callback(context,index,patterns_pointer_[index].first,patterns_pointer_[index].second);\
        if (r == 1)\
        {\
            return S_OK;\
        }\
        if (count_hit == count)\
        {\
            return S_OK;\
        }\
    } while (0)

HRESULT wumanber_search_t::search(const void * ptr_begin, const void * ptr_end, pfn_callback_hit_pattern callback, void * context)
{
    if(!(ptr_begin && ptr_end && callback )) return E_INVALIDARG;

    size_t count = pattern_count();

    if(!(count>0)) return E_INVALIDARG;

    const unsigned char * begin = (const unsigned char *)ptr_begin;
    const unsigned char * end = (const unsigned char *)ptr_end;

    std::vector<bool> patterns_hit;
    size_t count_hit = 0;
    patterns_hit.resize(count,false);

    if (!byte_table_.empty())
    {
        size_t hit = 0;
        for (const unsigned char * ptr_off =begin;ptr_off<end && hit < byte_table_count_;++ptr_off)
        {
            for (size_t index = byte_table_[*ptr_off]; index < count; index=hash_chain_table_[index])
            {
                WUMANBER_HIT;
                ++hit;
            }
        }
    }
    
    if (!short_table_.empty())
    {
        size_t hit = 0;
        for (const unsigned char* ptr_off=begin;ptr_off<end-1 && hit < short_table_count_;++ptr_off)
        {
            for (size_t index = short_table_[*((const unsigned short*)ptr_off)]; index < count; index=hash_chain_table_[index])
            {
                WUMANBER_HIT;
                ++hit;
            }
        }
    }

    const unsigned char * ptr_off=begin;
    for (ptr_off+=min_pattern_size_; ptr_off < end; )
    {
        DWORD h = _hash(B,shfit_table_.size(),ptr_off);
        size_t shift = shfit_table_[h];
        if ( 0 == shift)
        {
            const unsigned char * window_begin = ptr_off-min_pattern_size_;

            DWORD prefix_hash = _hash(B,shfit_table_.size(),window_begin+B);

            for (size_t index = hash_table_[h]; index < count; index=hash_chain_table_[index])
            {
                if (prefix_hash == prefix_table_[index])
                {
                    const void * b_pattern = patterns_pointer_[index].first;
                    const void * e_pattern = patterns_pointer_[index].second;

                    if (!patterns_hit[index])
                    {
                        if (!(distance_pointer(b_pattern,e_pattern)>distance_pointer(window_begin,ptr_end))
                            && 0 == memcmp(b_pattern,window_begin,distance_pointer(b_pattern,e_pattern)))
                        {
                            WUMANBER_HIT;
                        }
                    }
                }
            }
            shift = 1;
        }

        ptr_off += shift;

    }

    return S_OK;
}

#include <fstream>

void test_wumanber()
{
    std::ifstream f("test_wumanber.txt");
    HRESULT hr;
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

    std::vector<struct wumanber_search_t::result_t> results;
    results.resize(_countof(texts));

    for (std::string line;!f.eof();)
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
    assert(hr == S_OK);

    for (size_t i=0; i< results.size(); ++i)
    {
        const std::string & t = texts[i];
        hr = wumanber.search(t.c_str(),t.c_str()+t.size(),&results[i]);
        assert(hr == S_OK);
        assert(results[i].check_r == results[i].hit_r);
        printf("[%2d]%s ->hit at->",i,t.c_str());
        results[i].fprintf_result(stdout);
        printf("\n");
    }

    printf("wumanber : 100%% pass\n");
}


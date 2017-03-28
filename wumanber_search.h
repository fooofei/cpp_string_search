
#ifndef WUMANBER_SEARCH_H_
#define WUMANBER_SEARCH_H_

#include <vector>
#include <windows.h>
#include <map>
#include <string>

//
// 加快搜索的方法是:
//      1、增大跳转，分析 pattern 的长度分布，如果pattern长度个别的有 1， 2， 其他都大于 7 ，可以更改 B=3，对长度1,2的做单独处理
//      2、增大 shift 表空间
//
//

//
// 多数的 wumanber 算法实现没有处理长度小于Ｂ的pattern，这个文件的实现处理了此。
//
// 不处理忽略大小写，要忽略大小写，在存入 pattern 和搜索 text 时，应该把两部分都转换为一种case，再进行搜索。
//     在搜索时处理大小写转换会增加复杂度。
//
class wumanber_search_t
{
public:
    typedef std::vector<size_t> index_table_t;
    typedef std::vector<DWORD> dword_table_t;
    typedef std::vector<const void *> pointer_table_t;
    typedef std::vector<std::pair<const void *,const void*> > pattern_table_t;

    // return 0 continue, return 1 break
    //
    typedef int (*pfn_callback_hit_pattern)(void * context,size_t index, const void * begin, const void * end);

public:
    static size_t distance_pointer(const void * b, const void * e)
    {
        return ((const unsigned char *)e-(const unsigned char *)b);
    }
private:
    index_table_t byte_table_;
    size_t byte_table_count_;
    index_table_t short_table_;
    size_t short_table_count_;
    index_table_t shfit_table_;
    index_table_t hash_table_;
    index_table_t hash_chain_table_;
    dword_table_t prefix_table_;
    pattern_table_t patterns_pointer_;
    size_t B ;
    size_t min_pattern_size_;

    static size_t table_size(size_t pattern_size, size_t pattern_min_size);
public:
    void reset()
    {
        byte_table_.clear();
        short_table_.clear();
        shfit_table_.clear();
        hash_table_.clear();
        hash_chain_table_.clear();
        prefix_table_.clear();
        patterns_pointer_.clear();
        min_pattern_size_ = -1;
        B = 3;
        byte_table_count_ = 0;
        short_table_count_ =0;
    }

    wumanber_search_t()
    {
        reset();
    }
    ~wumanber_search_t()
    {
        ;
    }

    //
    // 内存外部维护 
    HRESULT push_pattern(const void * begin, const void * end, size_t * index);
    size_t pattern_count() const ;
    HRESULT init();

    // before call search(), you must call init().
    HRESULT search(const void * ptr_begin, const void * ptr_end, pfn_callback_hit_pattern callback, void * context);

    // 把 p 前面的两个字符作 hash
    // 扔 H 进来是为了防止数组越界
    static DWORD _hash(size_t B, size_t H, const void * p);


public:
    //
    // for test

    struct result_t
    {
        typedef std::map<size_t, std::string> result_type;
        result_type check_r;
        result_type hit_r;

        void fprintf_result(FILE * f)
        {
            for (result_type::const_iterator it = check_r.begin(); it != check_r.end();++it)
            {
                fprintf(f,"[%d]%s ",it->first,it->second.c_str());
            }
        }
    };

    static int callback_hit_pattern_strings(void * context,size_t index, const void * begin, const void * end)
    {
        struct result_t * results = (struct result_t *)context;
        std::string s((const char *)begin,distance_pointer(begin,end));
        results->hit_r[index] = s;
        return 0;
    }
    HRESULT search(const void * ptr_begin, const void * ptr_end, void * context)
    {
        return search(ptr_begin,ptr_end,callback_hit_pattern_strings,context);
    }
};


void test_wumanber();

#endif //WUMANBER_SEARCH_H_
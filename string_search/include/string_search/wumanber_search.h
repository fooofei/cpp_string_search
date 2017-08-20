
#ifndef WUMANBER_SEARCH_H_
#define WUMANBER_SEARCH_H_

#include <vector>
#include <map>
#include <string>
#include <stdint.h>


#ifndef E_INVALIDARG
#define E_INVALIDARG  ((int)0x80070057L)
#endif

#ifndef S_OK
#define S_OK ((int)0)
#endif

#ifndef E_FAIL
#define E_FAIL  ((int)0x80004005L)
#endif


//
// 一个内存搜索  支持多 pattern
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
    typedef std::vector<uint32_t> dword_table_t;
    typedef std::vector<const void *> pointer_table_t;
    // pattern 's [begin, end) table
    typedef std::vector<std::pair<const void *,const void*> > pattern_table_t;

    typedef void (*pfn_callback_hit_pattern)(void * context
                                            , bool * is_continue_search
                                            , size_t off_in_text
                                            , size_t index_in_pattern_table
                                            , const void * begin, const void * end);

private:
    index_table_t byte_table_; // size() = 0x100
    size_t byte_pattern_count_;
    index_table_t short_table_; // size() = 0x10000
    size_t short_pattern_count_;
    index_table_t shfit_table_;
    index_table_t hash_table_;
    index_table_t hash_chain_table_;
    dword_table_t prefix_table_;
    pattern_table_t patterns_pointer_;
    size_t B ;
    size_t min_pattern_size_;
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
        byte_pattern_count_ = 0;
        short_pattern_count_ =0;
    }

    wumanber_search_t()
    {
        reset();
    }
    ~wumanber_search_t()
    {
        ;
    }

    // 内存外部维护 , push 的时候会计算最小长度
    // int the [begin, end) pattern, no memory copy, return the pattern index in the table.
    int push_pattern(const void * begin, const void * end, size_t * index);
    size_t pattern_count() const ;
    int init();

    // before call search(), you must call init().
    int search(const void * ptr_begin, const void * ptr_end, pfn_callback_hit_pattern callback, void * context);


};


void test_wumanber();

#endif //WUMANBER_SEARCH_H_
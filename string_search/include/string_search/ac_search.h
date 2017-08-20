
#ifndef STRING_SEARCH_AC_SEARCH_H
#define STRING_SEARCH_AC_SEARCH_H

#include <limits.h>
#include <cstring>

class ac_search_t
{
    enum{CHAR_SET_SIZE=UCHAR_MAX+1,};
    struct node_t
    {
        node_t * fail;
        node_t * child[CHAR_SET_SIZE];
        size_t index_in_patterns;

        node_t()
        {
            std::memset(this,0, sizeof(*this));
            index_in_patterns = -1;
        }
    };

    node_t * root_;
    // pattern 's [begin, end) table
    typedef std::vector<std::pair<const void *, const void*> > pattern_table_t;

    pattern_table_t patterns_pointer_;

public:
    typedef void(*pfn_callback_hit_pattern)(void * context
        , bool * is_continue_search
        , size_t off_in_text
        , size_t index_in_pattern_table
        , const void * begin, const void * end);

public:
    void reset();

    ac_search_t():root_(NULL)
    {
        ;
    }

    ~ac_search_t()
    {
        reset();
    }


    int push_pattern(const void * begin, const void * end, size_t * index);
    size_t pattern_count();
    int init();
    // before call search(), you must call init().
    int search(const void * ptr_begin, const void * ptr_end, pfn_callback_hit_pattern callback, void * context);

};


#endif //STRING_SEARCH_AC_SEARCH_H


#include <stdint.h>
#include <queue>
#include <list>
#include <stdio.h>

#ifndef E_INVALIDARG
#define E_INVALIDARG  ((int)0x80070057L)
#endif

#ifndef S_OK
#define S_OK ((int)0)
#endif

#ifndef E_FAIL
#define E_FAIL  ((int)0x80004005L)
#endif

#include "../include/string_search/ac_search.h"


template <typename alloc_type>
alloc_type* node_alloc()
{
    return new alloc_type;
}

template <typename alloc_type>
void node_free(alloc_type ** p)
{
    delete *p;
    *p = NULL;
}

static
size_t distance_pointer(const void * b, const void * e)
{
    return ((const unsigned char *)e - (const unsigned char *)b);
}


int ac_search_t::push_pattern(const void * begin, const void * end, size_t * index)
{
    if (!(begin && end)) return E_INVALIDARG;

    if (!(begin < end)) return E_INVALIDARG;

    if (!root_) {
        root_ = node_alloc<node_t>();
    }

    const uint8_t * pb = (const uint8_t *)begin;

    node_t * p = root_;
    for (;pb<end;++pb)
    {
        node_t ** p2 = &(p->child[*pb]);
        if (NULL == (*p2))
        {
            *p2 = node_alloc<node_t>();
        }
        p = *p2;
    }

    if (p->index_in_patterns != -1) {
        printf("error, not -1, %s:(%d)\n", __FILE__, __LINE__);
    }
    p->index_in_patterns = patterns_pointer_.size();
    *index = patterns_pointer_.size();
    patterns_pointer_.push_back(std::make_pair(begin, end));

    return 0;
}

int ac_search_t::init()
{
    if (!root_) {
        return E_FAIL;
    }
    if (patterns_pointer_.empty()) {
        return E_FAIL;
    }
    std::queue<node_t*> q;

    for (size_t i = 0; i < CHAR_SET_SIZE; ++i)
    {
        node_t ** p = &(root_->child[i]);
        if (*p)
        {
            (*p)->fail = root_;
            q.push(*p);
        }
    }

    while (!q.empty())
    {
        node_t * cur = q.front();
        q.pop();

        for (size_t i = 0; i < CHAR_SET_SIZE; ++i)
        {
            node_t ** p = &(cur->child[i]);
            if (*p)
            {
                node_t * p2 = cur->fail;
                while (p2)
                {
                    if (p2->child[i])
                    {
                        (*p)->fail = p2->child[i];
                        break;
                    }
                    p2 = p2->fail;
                }

                if (!p2)
                {
                    (*p)->fail = root_;
                }
                q.push(*p);
            }
        }

    }
    return S_OK;
}


int ac_search_t::search(const void * ptr_begin, const void * ptr_end
    , pfn_callback_hit_pattern callback, void * context
    )
{
    if (!(ptr_begin && ptr_end)) return E_INVALIDARG;
    if (!(ptr_begin < ptr_end)) return E_INVALIDARG;

    node_t * p = root_;
    const uint8_t * off = (const uint8_t *)ptr_begin;
    bool is_continue;

    for (; off < ptr_end; ++off)
    {
        if (!p) break;
        uint8_t c = *off;
        while (p->child[c] == NULL && p != root_)
        {
            p = p->fail;
        }
        if (!(p->child[c]))
        {
            continue;
        }
        p = p->child[c];

        node_t * t = p;

        while (t != root_)
        {
            if (t->index_in_patterns != -1)
            {
                size_t iv = t->index_in_patterns;
                is_continue = true;
                callback(context, &is_continue, distance_pointer(ptr_begin, off)
                    , iv
                    , patterns_pointer_[iv].first
                    , patterns_pointer_[iv].second
                );
            }
            t = t->fail;
        }

    }

    return S_OK;
}



void ac_search_t::reset()
{

    std::queue<node_t**> q;

    typedef std::list<node_t**> list_node_t;
    list_node_t f;
    if (root_) {
        q.push(&root_);
    }
    

    while (!q.empty())
    {
        node_t ** p = q.front();
        q.pop();
        f.push_back(p); // add to free list

        for (size_t i = 0; i < CHAR_SET_SIZE; ++i)
        {
            if ((*p)->child[i])
            {
                q.push(&((*p)->child[i]));
            }
        }
    }


    for (list_node_t::iterator it = f.begin(); it != f.end(); ++it)
    {
        node_free(*it);
    }

    root_ = NULL;
    patterns_pointer_.clear();

}
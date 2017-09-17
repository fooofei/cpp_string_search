

#include <stdint.h>
#include <queue>
#include <list>
#include <vector>
#include <map>

#ifndef E_INVALIDARG
#define E_INVALIDARG  ((int)0x80070057L)
#endif

#ifndef S_OK
#define S_OK ((int)0)
#endif

#ifndef E_FAIL
#define E_FAIL  ((int)0x80004005L)
#endif

#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY ((int)0x8007000EL);
#endif

#include "../include/string_search/ac_search.h"


struct ac_search_t::node_t
{
  node_t *fail;
  node_t *child[CHAR_SET_SIZE];
  std::vector <size_t> pattern_indexs;
  uint32_t node_number;

  node_t()
  {
    clear();
  }

  void clear()
  {
    fail=0;
    std::memset(child,0,sizeof(child));
    pattern_indexs.clear();
    node_number = 0;
  }

};

struct ac_search_t::node_pool_t
{
  typedef std::vector <node_t> chunk_type;
  typedef std::list<chunk_type *> value_type;
  typedef std::list<node_t *> unused_address_type;
  value_type value;
  unused_address_type unused_address;

  node_pool_t() {}

  ~node_pool_t()
  {
    clear();
  }

  void clear()
  {
    size_t i = 0;
    for (value_type::iterator it = value.begin(); it != value.end(); ++it)
    {
      i += (*it)->size();
      delete (*it);
    }
    value.clear();

    if (unused_address.size() != i)
    {
      //printf("!!! memory leak, leak node_t count %lu\n", i-unused_address.size());
    }
    unused_address.clear();
  }

  int node_alloc(node_t **p)
  {
    if (unused_address.empty())
    {
      chunk_type *v = new(std::nothrow)chunk_type();
      if (!v) return E_OUTOFMEMORY;
      v->resize(0x10000);
      value.push_back(v);
      for (size_t i = 0; i < v->size(); ++i)
      {
        unused_address.push_back(&(*v)[i]);
      }
    }

    if (unused_address.size() > 0)
    {
      *p = unused_address.front();
      (*p)->clear();
      unused_address.pop_front();
      return S_OK;
    }
    return E_OUTOFMEMORY;
  }

  void node_free(node_t **p)
  {
    unused_address.push_back(*p);
    *p = NULL;
  }
};


template<typename alloc_type>
void node_alloc1(alloc_type **p)
{
  *p = new(std::nothrow) alloc_type;

}

template<typename alloc_type>
void node_free1(alloc_type **p)
{
  delete *p;
  *p = NULL;
}

static
size_t 
distance_pointer(const void *b, const void *e)
{
  return ((const unsigned char *) e - (const unsigned char *) b);
}

ac_search_t::ac_search_t() : root_(NULL), pool_(NULL)
{
  ;
}
ac_search_t::~ac_search_t()
{
  clear();
}


int 
ac_search_t::push_pattern(const void *begin, const void *end, size_t *index)
{
  if (!(begin && end)) return E_INVALIDARG;

  if (!(begin < end)) return E_INVALIDARG;

  if (!pool_)
  {
    pool_ = new(std::nothrow)node_pool_t();
  }
  if (!pool_)
  {
    return E_OUTOFMEMORY;
  }

  int err;

  if (!root_)
  {
    err = pool_->node_alloc(&root_);
    if (err != S_OK) return err;
  }

  const uint8_t *pb = (const uint8_t *) begin;

  node_t *p = root_;
  for (; pb < end; ++pb)
  {
    node_t **p2 = &(p->child[*pb]);
    if (NULL == (*p2))
    {
      err = pool_->node_alloc(p2);
      if (err != S_OK) return err;
    }
    p = *p2;
  }

  p->pattern_indexs.push_back(patterns_pointer_.size());
  *index = patterns_pointer_.size();
  patterns_pointer_.push_back(std::make_pair(begin, end));

  return 0;
}

int 
ac_search_t::init()
{
  if (!root_)
  {
    return E_FAIL;
  }
  if (patterns_pointer_.empty())
  {
    return E_FAIL;
  }
  std::queue < node_t * > q;

  for (size_t i = 0; i < CHAR_SET_SIZE; ++i)
  {
    node_t *p = (root_->child[i]);
    if (p)
    {
      (p)->fail = root_;
      q.push(p);
    }
  }

  while (!q.empty())
  {
    node_t *cur = q.front();
    q.pop();

    for (size_t i = 0; i < CHAR_SET_SIZE; ++i)
    {
      node_t *p = (cur->child[i]);
      if (p)
      {
        node_t *p2 = cur->fail;
        while (p2)
        {
          if (p2->child[i])
          {
            (p)->fail = p2->child[i];
            break;
          }
          p2 = p2->fail;
        }

        if (!p2)
        {
          (p)->fail = root_;
        }
        q.push(p);
      }
    }

  }
  return S_OK;
}


int 
ac_search_t::search(const void *ptr_begin, const void *ptr_end, pfn_callback_hit_pattern callback, void *context
)
{
  if (!(ptr_begin && ptr_end)) return E_INVALIDARG;
  if (!(ptr_begin < ptr_end)) return E_INVALIDARG;

  node_t *p = root_;
  const uint8_t *off = (const uint8_t *) ptr_begin;
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

    node_t *t = p;

    while (t != root_)
    {
      for (size_t iv = 0; iv < t->pattern_indexs.size(); ++iv)
      {
        size_t jv = t->pattern_indexs[iv];
        is_continue = true;
        const void *p1 = patterns_pointer_[jv].first;
        const void *p2 = patterns_pointer_[jv].second;
        callback(context, &is_continue, distance_pointer(ptr_begin, off + 1) - distance_pointer(p1, p2), jv, p1, p2
        );
      }
      t = t->fail;
    }

  }

  return S_OK;
}


void 
ac_search_t::clear()
{

  std::queue <node_t *> q;

  typedef std::list<node_t *> list_node_t;
  list_node_t f;
  if (root_)
  {
    q.push(root_);
  }

  while (!q.empty())
  {
    node_t *p = q.front();
    q.pop();
    f.push_back(p); // add to free list

    for (size_t i = 0; i < CHAR_SET_SIZE; ++i)
    {
      if ((p)->child[i])
      {
        q.push(((p)->child[i]));
      }
    }
  }


  for (list_node_t::iterator it = f.begin(); it != f.end(); ++it)
  {
    pool_->node_free(&(*it));
  }

  root_ = NULL;
  patterns_pointer_.clear();
  if (pool_)
  {
    pool_->clear();
    delete pool_;
    pool_ = NULL;
  }
}

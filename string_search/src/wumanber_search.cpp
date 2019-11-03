
#include "../include/string_search/wumanber_search.h"
#include <algorithm>
#include <cstring> // std::memset
#include <map>
#include <string.h>
#include <string>

// 把 p 前面的两个字符作 hash
// 获得的返回值作为数组的索引，扔 H 进来是为了防止数组越界
static uint32_t
_hash(size_t B, size_t H, const void* p)
{
    const uint8_t* pb = (const uint8_t*)p;
    //uint32_t v1
    union {
        struct
        {
            uint8_t v0;
            uint8_t v1;
            uint8_t v2;
            uint8_t v3;
        };
        uint32_t reserve;
    } v;
    std::memset(&v, 0, sizeof(v));
    switch (B) {
    case 3:
        v.v2 = *(pb - 3);
    case 2:
        v.v0 = *(pb - 1);
        v.v1 = *(pb - 2);
        return ((uint32_t)(H - 1) & v.reserve);
    default:
        return 0;
    }
}

static size_t
distance_pointer(const void* b, const void* e)
{
    return ((const unsigned char*)e - (const unsigned char*)b);
}

static size_t
table_size(size_t pattern_count, size_t pattern_min_size)
{
    size_t primes[] = { 1003, 10007, 100003, 1000003, 10000019, 100000007, 0 };

    size_t threshold = 10 * pattern_min_size;
    for (size_t* p = primes; *p; ++p) {
        if (*p > pattern_count && *p / pattern_count > threshold) {
            return *p;
        }
    }
    return primes[sizeof(primes) / sizeof(primes[0]) - 1];
}

void wumanber_search_t::clear()
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
    short_pattern_count_ = 0;
}

wumanber_search_t::wumanber_search_t()
{
    clear();
}

wumanber_search_t::~wumanber_search_t()
{
    clear();
}

int wumanber_search_t::push_pattern(const void* begin, const void* end, size_t* index)
{
    if (!(begin && end && end > begin))
        return E_INVALIDARG;

    if (index) {
        *index = patterns_pointer_.size();
    }
    patterns_pointer_.push_back(std::make_pair(begin, end));

    const unsigned char* b = (const unsigned char*)begin;
    const unsigned char* e = (const unsigned char*)end;
    size_t l = (e - b);

    if (B <= l) // 过短的字符就放入其他的表中，不然非常影响跳转的性能
    {
        min_pattern_size_ = (std::min<size_t>)(min_pattern_size_, l);
    }
    return S_OK;
}

size_t
wumanber_search_t::pattern_count() const
{
    return patterns_pointer_.size();
}

int wumanber_search_t::init()
{
    size_t count = pattern_count();

    if (!(count > 0))
        return E_INVALIDARG;

    // assert(B <= min_pattern_size_);

    size_t H = table_size(count, min_pattern_size_); //1<<12; // 4096

    shfit_table_.resize(H, min_pattern_size_ - B + 1);
    hash_table_.resize(H, count);
    hash_chain_table_.resize(count);
    prefix_table_.resize(count);

    for (size_t i = 0; i < count; ++i) {
        const unsigned char* begin = (const unsigned char*)patterns_pointer_[i].first;
        const void* end = patterns_pointer_[i].second;
        uint32_t h;
        size_t l = distance_pointer(begin, end);
        if (B <= l) {
            for (size_t j = min_pattern_size_; j >= B; --j) {
                h = _hash(B, shfit_table_.size(), begin + j);
                shfit_table_[h] = (std::min<size_t>)(min_pattern_size_ - j, shfit_table_[h]);
            }

            h = _hash(B, shfit_table_.size(), begin + min_pattern_size_);
            hash_chain_table_[i] = hash_table_[h];
            hash_table_[h] = i;

            h = _hash(B, shfit_table_.size(), begin + B);
            prefix_table_[i] = h;
        } else {
            unsigned short c;
            switch (l) {
            case 1:
                if (byte_table_.empty())
                    byte_table_.resize(0x100, count);
                hash_chain_table_[i] = byte_table_[*begin];
                byte_table_[*begin] = i;
                byte_pattern_count_ += 1;
                break;
            case 2:
                if (short_table_.empty())
                    short_table_.resize(0x10000, count);
                c = *(const unsigned short*)begin;
                hash_chain_table_[i] = short_table_[c];
                short_table_[c] = i;
                short_pattern_count_ += 1;
                break;
            default:
                return E_FAIL;
            }
        }
    }

    return S_OK;
}

#define WUMANBER_HIT                                                                                                                               \
    do {                                                                                                                                           \
        patterns_hit[index] = true;                                                                                                                \
        ++count_hit;                                                                                                                               \
        bool is_continue = true;                                                                                                                   \
        callback(context, &is_continue, distance_pointer(begin, ptr_off), index, patterns_pointer_[index].first, patterns_pointer_[index].second); \
        if (!is_continue) {                                                                                                                        \
            return S_OK;                                                                                                                           \
        }                                                                                                                                          \
        if (count_hit == count) {                                                                                                                  \
            return S_OK;                                                                                                                           \
        }                                                                                                                                          \
    } while (0)

int wumanber_search_t::search(const void* ptr_begin, const void* ptr_end, pfn_callback_hit_pattern callback, void* context)
{
    if (!(ptr_begin && ptr_end && callback))
        return E_INVALIDARG;

    size_t count = pattern_count();

    if (!(count > 0))
        return E_INVALIDARG;

    const unsigned char* begin = (const unsigned char*)ptr_begin;
    const unsigned char* end = (const unsigned char*)ptr_end;

    std::vector<bool> patterns_hit;
    size_t count_hit = 0;
    patterns_hit.resize(count, false);

    if (!byte_table_.empty()) {
        size_t hit = 0;
        for (const unsigned char* ptr_off = begin; ptr_off < end && hit < byte_pattern_count_; ++ptr_off) {
            for (size_t index = byte_table_[*ptr_off]; index < count; index = hash_chain_table_[index]) {
                WUMANBER_HIT;
                ++hit;
            }
        }
    }

    if (!short_table_.empty()) {
        size_t hit = 0;
        for (const unsigned char* ptr_off = begin; ptr_off < end - 1 && hit < short_pattern_count_; ++ptr_off) {
            for (size_t index = short_table_[*((const unsigned short*)ptr_off)];
                 index < count; index = hash_chain_table_[index]) {
                WUMANBER_HIT;
                ++hit;
            }
        }
    }

    const unsigned char* ptr_off2 = begin;
    for (ptr_off2 += min_pattern_size_; ptr_off2 < end;) {
        uint32_t h = _hash(B, shfit_table_.size(), ptr_off2);
        size_t shift = shfit_table_[h];
        if (0 == shift) {
            const unsigned char* window_begin = ptr_off2 - min_pattern_size_;

            uint32_t prefix_hash = _hash(B, shfit_table_.size(), window_begin + B);

            for (size_t index = hash_table_[h]; index < count; index = hash_chain_table_[index]) {
                if (prefix_hash == prefix_table_[index]) {
                    const void* b_pattern = patterns_pointer_[index].first;
                    const void* e_pattern = patterns_pointer_[index].second;

                    if (!patterns_hit[index]) {
                        if ((distance_pointer(b_pattern, e_pattern) <= distance_pointer(window_begin, ptr_end))
                            && 0 == memcmp(b_pattern, window_begin, distance_pointer(b_pattern, e_pattern))) {
                            const uint8_t* ptr_off = window_begin;
                            WUMANBER_HIT;
                        }
                    }
                }
            }
            shift = 1;
        }

        ptr_off2 += shift;
    }

    return S_OK;
}

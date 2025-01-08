#include <utils.hpp>

namespace vs{
namespace templ{


std::vector<std::string_view> split_string (const char* str, char delim) {
    std::vector<std::string_view> result;
    size_t i = 0, last = 0;
    for(;; i++){
        if(str[i]==delim){result.emplace_back(str+last,i-last);i++;last=i;}
        if(str[i]==0){result.emplace_back(str+last,i-last);break;}
    }
    return result;
}

int cmp_dot_str(const char* a, const char* b){
    auto va = split_string(a, '.');
    auto vb = split_string(b, '.');

    if(va.size()<vb.size())return -1;
    else if(va.size()>vb.size())return 1;

    for(size_t i =0;i<va.size();i++){
        if(va.at(i)<vb.at(i))return -1;
        else if(va.at(i)>vb.at(i))return 1;
    }
    return -1;
}

namespace hash{

#if defined(_MSC_VER)
    #define FORCE_INLINE	__forceinline
    #include <stdlib.h>
    #define ROTL64(x,y)	_rotl64(x,y)
    #define BIG_CONSTANT(x) (x)
#else

#define	FORCE_INLINE inline __attribute__((always_inline))
    inline uint64_t rotl64 ( uint64_t x, int8_t r ){
        return (x << r) | (x >> (64 - r));
    }

    #define	ROTL32(x,y)	rotl32(x,y)
    #define ROTL64(x,y)	rotl64(x,y)

    #define BIG_CONSTANT(x) (x##LLU)
#endif // !defined(_MSC_VER)



FORCE_INLINE uint64_t getblock64 ( const uint64_t * p, int i ){
  return p[i];
}

FORCE_INLINE uint64_t fmix64 ( uint64_t k ){
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
  k ^= k >> 33;

  return k;
}


void MurmurHash3_x64_128 ( const void * key, const int len, const uint32_t seed, void * out ){
  const uint8_t * data = (const uint8_t*)key;
  const int nblocks = len / 16;

  uint64_t h1 = seed;
  uint64_t h2 = seed;

  const uint64_t c1 = BIG_CONSTANT(0x87c37b91114253d5);
  const uint64_t c2 = BIG_CONSTANT(0x4cf5ad432745937f);

  //----------
  // body

  const uint64_t * blocks = (const uint64_t *)(data);

  for(int i = 0; i < nblocks; i++){
    uint64_t k1 = getblock64(blocks,i*2+0);
    uint64_t k2 = getblock64(blocks,i*2+1);

    k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
    h1 = ROTL64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;
    k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;
    h2 = ROTL64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
  }

  //----------
  // tail

  const uint8_t * tail = (const uint8_t*)(data + nblocks*16);

  uint64_t k1 = 0;
  uint64_t k2 = 0;

  switch(len & 15){
  case 15: k2 ^= ((uint64_t)tail[14]) << 48;
  case 14: k2 ^= ((uint64_t)tail[13]) << 40;
  case 13: k2 ^= ((uint64_t)tail[12]) << 32;
  case 12: k2 ^= ((uint64_t)tail[11]) << 24;
  case 11: k2 ^= ((uint64_t)tail[10]) << 16;
  case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;
  case  9: k2 ^= ((uint64_t)tail[ 8]) << 0;
           k2 *= c2; k2  = ROTL64(k2,33); k2 *= c1; h2 ^= k2;

  case  8: k1 ^= ((uint64_t)tail[ 7]) << 56;
  case  7: k1 ^= ((uint64_t)tail[ 6]) << 48;
  case  6: k1 ^= ((uint64_t)tail[ 5]) << 40;
  case  5: k1 ^= ((uint64_t)tail[ 4]) << 32;
  case  4: k1 ^= ((uint64_t)tail[ 3]) << 24;
  case  3: k1 ^= ((uint64_t)tail[ 2]) << 16;
  case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;
  case  1: k1 ^= ((uint64_t)tail[ 0]) << 0;
           k1 *= c1; k1  = ROTL64(k1,31); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= len; h2 ^= len;

  h1 += h2;
  h2 += h1;
  h1 = fmix64(h1);
  h2 = fmix64(h2);

  h1 += h2;
  h2 += h1;

  ((uint64_t*)out)[0] = h1;
  ((uint64_t*)out)[1] = h2;
}

}

}
}
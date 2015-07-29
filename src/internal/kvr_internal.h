/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef KVR_INTERNAL_H
#define KVR_INTERNAL_H

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "../kvr.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if KVR_DEBUG && 1
#define KVR_ASSERT(X) assert(X)
#define KVR_ASSERT_SAFE(X, R) KVR_ASSERT(X)
#else
#define KVR_ASSERT(X) 
#define KVR_ASSERT_SAFE(X, R) do { if (!X) { return (R); } } while (0)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if defined (_MSC_VER)
#define KVR_LITTLE_ENDIAN
#else
#define KVR_BIG_ENDIAN
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#if defined (_MSC_VER)
#define kvr_strcpy(dst, dsz, src) strcpy_s (dst, dsz, src)
#define kvr_strncpy(dst, src, n) strncpy_s (dst, src, n)
#define kvr_strdup(src, sz) _strdup (src)
#else
#define kvr_strcpy(dst, dsz, src) { strncpy (dst, src, dsz); dst [dsz - 1] = 0; }
#define kvr_strncpy(dst, src, n) strncpy (dst, src, n)
#define kvr_strdup(src, sz) strndup (src, sz)
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

class kvr_internal
{
public:

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static size_t u32toa (uint32_t u32, char dest [10])
  {
    const char* end = kvr_rapidjson::internal::u32toa (u32, dest);
    return (end - dest);
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static size_t i64toa (int64_t i64, char dest [21])
  {
    const char* end = kvr_rapidjson::internal::i64toa (i64, dest);
    return (end - dest);
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static uint32_t ndigitsu32 (uint32_t u32)
  {
    uint32_t count = kvr_rapidjson::internal::CountDecimalDigit32 (u32);
    return count;
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static uint32_t ndigitsi64 (int64_t i64)
  {
#if 0
    int64_t n = i64;
    uint32_t count = (n < 0) ? 1 : 0; // sign
    do
    {
      ++count;
      n /= 10;
    } while (n);

    return count;
#else
    uint32_t neg = 0;
    uint64_t u64 = static_cast<uint64_t>(i64);
    if (i64 < 0) 
    {      
      neg = 1;
      u64 = ~u64 + 1;
    }

    if (u64 < 10) return 1 + neg;
    if (u64 < 100) return 2 + neg;
    if (u64 < 1000) return 3 + neg;
    if (u64 < 10000) return 4 + neg;
    if (u64 < 100000) return 5 + neg;
    if (u64 < 1000000) return 6 + neg;
    if (u64 < 10000000) return 7 + neg;
    if (u64 < 100000000) return 8 + neg;
    if (u64 < 1000000000) return 9 + neg;
    if (u64 < 10000000000) return 10 + neg;
    if (u64 < 100000000000) return 11 + neg;
    if (u64 < 1000000000000) return 12 + neg;
    if (u64 < 10000000000000) return 13 + neg;
    if (u64 < 100000000000000) return 14 + neg;
    if (u64 < 1000000000000000) return 15 + neg;
    if (u64 < 10000000000000000) return 16 + neg;
    if (u64 < 100000000000000000) return 17 + neg;
    if (u64 < 1000000000000000000) return 18 + neg;
    if (u64 < 10000000000000000000) return 19 + neg;
    return 20 + neg;
#endif
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static uint16_t byteswap16 (uint16_t val)
  {
    uint16_t swap = (uint16_t) ((val >> 8) | (val << 8));
    return swap;
  }
  
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static uint32_t byteswap32 (uint32_t val)
  {
    uint32_t swap = (uint32_t) (((val >> 24) & 0x000000ff) |
                                ((val >> 8)  & 0x0000ff00) |
                                ((val << 8)  & 0x00ff0000) |
                                ((val << 24) & 0xff000000));
    return swap;
  }
  
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static uint64_t byteswap64 (uint64_t val)
  {
    uint64_t swap = (uint64_t) (((val >> 56) & 0x00000000000000ff) |
                                ((val >> 40) & 0x000000000000ff00) |
                                ((val >> 24) & 0x0000000000ff0000) |
                                ((val >> 8)  & 0x00000000ff000000) |
                                ((val << 8)  & 0x000000ff00000000) |
                                ((val << 24) & 0x0000ff0000000000) |
                                ((val << 40) & 0x00ff000000000000) |
                                ((val << 56) & 0xff00000000000000));
    return swap;
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static uint32_t strhash (const char *str)
  {
    KVR_ASSERT (str);

    // djb hash function (fast)

    uint32_t hash = 5381;
    char c;

    while ((c = *str++))
    {
      hash = ((hash << 5) + hash) + c;
    }

    return hash;
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  static bool hex_encode (const uint8_t *data, size_t size, kvr::ostream *ostr)
  {
    static const char lut [] = "0123456789abcdef";

    bool success = false;

    if (data && (size > 0))
    {
      size_t tcap = (size * 2) + 1;
      if (ostr->capacity () < tcap) { ostr->resize (tcap); }

      for (size_t i = 0; i < size; ++i)
      {
        uint8_t c = data [i];
        char hi = lut [(c >> 4)];
        char lo = lut [(c & 15)];
        ostr->put (hi);
        ostr->put (lo);
      }

      success = true;
    }

    return success;
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename T>
  static const T& min (const T& a, const T& b)
  {
    return (a < b) ? a : b;
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename T>
  static const T& max (const T& a, const T& b)
  {
    return (a > b) ? a : b;
  }

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  // floating point nan check
};

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

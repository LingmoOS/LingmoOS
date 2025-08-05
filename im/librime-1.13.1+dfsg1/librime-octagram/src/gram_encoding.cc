#include "gram_encoding.h"
#include <rime/common.h>
#include <utf8.h>

namespace rime {
namespace grammar {

string encode(const char* begin, const char* end) {
  char encoded_str[kMaxEncodedUnicode * 4];
  char* e = encoded_str;
  for (auto p = begin; p < end; ) {
    uint32_t u = utf8::unchecked::next(p);
    if (u < 0x80) {
      if (u == 0) {
        *e++ = char(0xE0);
      } else {
        *e++ = char(u);
      }
    } else if (u >= 0x4000 && u < 0xA000) {
      if ((u & 0xFF) == 0) {
        *e++ = char(0xE1);
        *e++ = char((u >> 8) + 0x40);
      } else {
        *e++ = char((u >> 8) + 0x40);
        *e++ = char(u & 0xFF);
      }
    } else {
      int bits = 32;
      while (bits > 0 && (u & 0xFE000000) == 0) {
        bits -= 7;
        u <<= 7;
      }
      int bytes_to_encode = (bits + 6) / 7;
      *e++ = char(0xE0 | bytes_to_encode);
      while (bytes_to_encode > 0) {
        --bytes_to_encode;
        *e++ = char(((u >> 25) & 0x7F) | 0x80);
      }
    }
  }
  return string(encoded_str, e);
}

inline static const char* advance(const char* p) {
  return p + ((*p & 0x80) == 0 ? 1 :
              (*p & 0xF0) == 0xE0 ? (*p & 0x0F) + 1 : 2);
}

const char* next_unicode(const char* p) {
  return advance(p);
}

size_t unicode_length(const string& encoded, size_t length) {
  const auto begin = encoded.c_str();
  const auto end = begin + length;
  size_t len = 0;
  for (auto p = begin; p < end; p = advance(p)) {
    ++len;
  }
  return len;
}

}  // namespace grammar
}  // namespace rime

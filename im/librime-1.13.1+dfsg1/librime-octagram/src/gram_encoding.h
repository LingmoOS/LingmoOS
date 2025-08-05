#ifndef RIME_GRAM_ENCODING_H_
#define RIME_GRAM_ENCODING_H_

#include <rime/common.h>

namespace rime {
namespace grammar {

constexpr int kMaxEncodedUnicode = 8;

string encode(const char* begin, const char* end);
inline string encode(const string& utf8_str) {
  return encode(utf8_str.c_str(), utf8_str.c_str() + utf8_str.length());
}
const char* next_unicode(const char* p);
size_t unicode_length(const string& encoded, size_t length);

}  // namespace grammar
}  // namespace rime

#endif  // RIME_GRAM_ENCODING_H_

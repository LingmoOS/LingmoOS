#ifndef RIME_GRAM_DB_H_
#define RIME_GRAM_DB_H_

#include <darts.h>
#include <rime/resource.h>
#include <rime/dict/mapped_file.h>

namespace rime {

namespace grammar {

struct Metadata {
  static const int kFormatMaxLength = 32;
  char format[kFormatMaxLength];
  uint32_t db_checksum;
  uint32_t double_array_size;
  OffsetPtr<char> double_array;
};

}  // namespace grammar

class GramDb : public MappedFile {
 public:
  using Match = Darts::DoubleArray::result_pair_type;
  static constexpr int kMaxResults = 8;
  static constexpr double kValueScale = 10000;

  GramDb(const path& file_path)
      : MappedFile(file_path),
        trie_(new Darts::DoubleArray) {}

  bool Load();
  bool Save();
  bool Build(const vector<pair<string, double>>& data);
  int Lookup(const string& context,
             const string& word,
             Match results[kMaxResults]);

 private:
  the<Darts::DoubleArray> trie_;
  grammar::Metadata* metadata_ = nullptr;
};

}  // namespace rime

#endif  // RIME_GRAM_DB_H_

#include "gram_db.h"
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <darts.h>
#include <rime/resource.h>
#include <rime/dict/mapped_file.h>

namespace rime {

const string kGrammarFormat = "Rime::Grammar/1.0";
const string kGrammarFormatPrefix = "Rime::Grammar/";

bool GramDb::Load() {
  LOG(INFO) << "loading gram db: " << file_path();

  if (IsOpen())
    Close();

  if (!OpenReadOnly()) {
    LOG(ERROR) << "error opening gram db '" << file_path() << "'.";
    return false;
  }

  metadata_ = Find<grammar::Metadata>(0);
  if (!metadata_) {
    LOG(ERROR) << "metadata not found.";
    Close();
    return false;
  }

  if (!boost::starts_with(string(metadata_->format), kGrammarFormatPrefix)) {
    LOG(ERROR) << "invalid metadata.";
    Close();
    return false;
  }

  char* array = metadata_->double_array.get();
  if (!array) {
    LOG(ERROR) << "double array image not found.";
    Close();
    return false;
  }
  size_t array_size = metadata_->double_array_size;
  LOG(INFO) << "found double array image of size " << array_size << ".";
  trie_->set_array(array, array_size);

  return true;
}

bool GramDb::Save() {
  LOG(INFO) << "saving gram db: " << file_path();
  if (!trie_->total_size()) {
    LOG(ERROR) << "the trie has not been constructed!";
    return false;
  }
  return ShrinkToFit();
}

bool GramDb::Build(const vector<pair<string, double>>& data) {
  // build trie
  auto data_size = data.size();
  vector<const char*> keys;
  vector<int> values;
  keys.reserve(data_size);
  values.reserve(data_size);
  for (const auto& kv : data) {
    keys.push_back(kv.first.c_str());
    values.push_back((std::max)(0, int(log(kv.second) * kValueScale)));
  }
  if (0 != trie_->build(int(data_size), &keys[0], NULL, &values[0])) {
    LOG(ERROR) << "Error building double-array trie.";
    return false;
  }
  // creating gram db
  size_t array_size = trie_->size();
  size_t image_size = trie_->total_size();
  const size_t kReservedSize = 1024;
  if (!Create(image_size + kReservedSize)) {
    LOG(ERROR) << "Error creating gram db file '" << file_path() << "'.";
    return false;
  }
  // creating metadata
  auto metadata = Allocate<grammar::Metadata>();
  if (!metadata) {
    LOG(ERROR) << "Error creating metadata in file '" << file_path() << "'.";
    return false;
  }
  metadata_ = metadata;
  // saving double-array image
  char* array = Allocate<char>(image_size);
  if (!array) {
    LOG(ERROR) << "Error creating double-array image.";
    return false;
  }
  std::memcpy(array, trie_->array(), image_size);
  metadata->double_array = array;
  metadata->double_array_size = array_size;
  // at last, complete the metadata
  std::strncpy(metadata->format,
               kGrammarFormat.c_str(),
               kGrammarFormat.length());
  return true;
}

int GramDb::Lookup(const string& context,
                   const string& word,
                   Match results[kMaxResults]) {
  size_t node_pos = 0;
  size_t key_pos = 0;
  trie_->traverse(context.c_str(), node_pos, key_pos);
  if (key_pos == context.length()) {
    return trie_->commonPrefixSearch(
        word.c_str(), results, kMaxResults, 0, node_pos);
  }
  return 0;
}

}  // namespace rime

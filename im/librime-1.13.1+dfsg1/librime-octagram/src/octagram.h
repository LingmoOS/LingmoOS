#ifndef RIME_OCTAGRAM_H_
#define RIME_OCTAGRAM_H_

#include <rime/common.h>
#include <rime/component.h>
#include <rime/resource.h>
#include <rime/gear/grammar.h>

namespace rime {

extern const ResourceType kGramDbType;
extern const string kGrammarDefaultLanguage;

class Config;
class GramDb;
struct GrammarConfig;
class OctagramComponent;

class Octagram : public Grammar {
 public:
  Octagram(Config* config, OctagramComponent* component);
  virtual ~Octagram();
  double Query(const string& context,
               const string& word,
               bool is_rear) override;

 private:
  the<GrammarConfig> config_;
  GramDb* db_ = nullptr;
};

class OctagramComponent : public Grammar::Component {
 public:
  OctagramComponent();
  virtual ~OctagramComponent();

  Octagram* Create(Config* config) override;

  GramDb* GetDb(const string& language);

 private:
  map<string, the<GramDb>> db_by_language_;
};

}  // namespace rime

#endif  // RIME_OCTAGRAM_H_

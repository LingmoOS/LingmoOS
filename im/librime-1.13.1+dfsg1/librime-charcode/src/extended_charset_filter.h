//
// Copyright RIME Developers
// Distributed under the BSD License
//
#ifndef RIME_EXTENDED_CHARSET_FILTER_H_
#define RIME_EXTENDED_CHARSET_FILTER_H_

#include <rime_api.h>
#include <rime/filter.h>
#include <rime/translation.h>
#include <rime/gear/charset_filter.h>

namespace rime {

class ExtendedCharsetFilterTranslation : public CharsetFilterTranslation {
 public:
  ExtendedCharsetFilterTranslation(an<Translation> translation,
                                   const string& charset);

 protected:
  bool FilterCandidate(an<Candidate> cand) override;
  void ParseParameters(const string& charset);

  string charset_;
  bool is_emoji_enabled_ = false;
};

struct DictEntry;

class ExtendedCharsetFilter : public CharsetFilter {
 public:
  explicit ExtendedCharsetFilter(const Ticket& ticket)
      : CharsetFilter(ticket) {}

  an<Translation> Apply(an<Translation> translation,
                        CandidateList* candidates) override;

  // return true to accept, false to reject the tested item
  RIME_API static bool FilterText(const string& text,
                                  const string& charset = "",
                                  bool is_emoji_enabled = false);
};

}  // namespace rime

#endif  // RIME_EXTENDED_CHARSET_FILTER_H_

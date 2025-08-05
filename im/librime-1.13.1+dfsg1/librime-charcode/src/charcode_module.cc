#include <rime/component.h>
#include <rime/registry.h>
#include <rime_api.h>

#include "codepoint_translator.h"
#include "extended_charset_filter.h"

using namespace rime;

static void rime_charcode_initialize() {
  Registry &r = Registry::instance();
  r.Register("codepoint_translator", new Component<CodepointTranslator>);
  r.Register("charset_filter", new Component<ExtendedCharsetFilter>);
}

static void rime_charcode_finalize() {
}

RIME_REGISTER_MODULE(charcode)

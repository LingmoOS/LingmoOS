//
// Copyright RIME Developers
// Distributed under GPLv3
//

#include "octagram.h"
#include <rime/common.h>
#include <rime/registry.h>
#include <rime/setup.h>  // for rime::LoadModules in RIME_REGISTER_MODULE_GROUP
#include <rime_api.h>

static void rime_grammar_initialize() {
  using namespace rime;

  LOG(INFO) << "registering components from module 'grammar'.";
  Registry& r = Registry::instance();
  r.Register("grammar", new OctagramComponent);
}

static void rime_grammar_finalize() {}

RIME_REGISTER_MODULE(grammar)

RIME_REGISTER_MODULE_GROUP(octagram, "grammar")

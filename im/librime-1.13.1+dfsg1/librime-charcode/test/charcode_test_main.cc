#include <gtest/gtest.h>
#include <rime/setup.h>

static RIME_MODULE_LIST(test_modules, "charcode");

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  rime::SetupLogging("rime-charcode.test");
  rime::LoadModules(test_modules);
  return RUN_ALL_TESTS();
}

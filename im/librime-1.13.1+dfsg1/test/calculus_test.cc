//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2012-01-17 GONG Chen <chen.sst@gmail.com>
//
#include <cmath>
#include <gtest/gtest.h>
#include <rime/common.h>
#include <rime/algo/calculus.h>

static const char* kTransliteration =
    "xlit abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* kTransformation = "xform/^([zcs])h(.*)$/$1$2/";
static const char* kErasion = "erase/^[czs]h[aoe]ng?$/";
static const char* kDerivation = "derive/^([zcs])h/$1/";
static const char* kAbbreviation = "abbrev/^([zcs]h).*$/$1/";

TEST(RimeCalculusTest, Transliteration) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kTransliteration));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("abracadabra");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("ABRACADABRA", s.str);
}

TEST(RimeCalculusTest, Transformation) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kTransformation));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("sang", s.str);
  // non-matching case
  s.str = "bang";
  EXPECT_FALSE(c->Apply(&s));
}

TEST(RimeCalculusTest, Erasion) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kErasion));
  ASSERT_TRUE(bool(c));
  EXPECT_FALSE(c->addition());
  EXPECT_TRUE(c->deletion());
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("", s.str);
  // non-matching case
  s.str = "bang";
  EXPECT_FALSE(c->Apply(&s));
}

TEST(RimeCalculusTest, Derivation) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kDerivation));
  ASSERT_TRUE(bool(c));
  EXPECT_TRUE(c->addition());
  EXPECT_FALSE(c->deletion());
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("sang", s.str);
  // non-matching case
  s.str = "bang";
  EXPECT_FALSE(c->Apply(&s));
}

TEST(RimeCalculusTest, Abbreviation) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kAbbreviation));
  ASSERT_TRUE(bool(c));
  EXPECT_TRUE(c->addition());
  EXPECT_FALSE(c->deletion());
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("sh", s.str);
  EXPECT_EQ(rime::kAbbreviation, s.properties.type);
  EXPECT_DOUBLE_EQ(log(0.5), s.properties.credibility);
}

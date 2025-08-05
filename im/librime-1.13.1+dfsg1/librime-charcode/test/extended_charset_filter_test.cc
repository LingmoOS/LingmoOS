//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2012-01-17 GONG Chen <chen.sst@gmail.com>
//
#include <gtest/gtest.h>
#include <rime/common.h>
#include <rime/translation.h>

#include "extended_charset_filter.h"

using namespace rime;

TEST(RimeExtendedCharsetFilterTest, FilterText) {
  // unkown
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("荣", "unkown"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("👋", "unkown"));

  // utf8
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("Hello", "utf8"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("荣", "utf8"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("鎔", "utf8"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("𤘺", "utf8"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("👋", "utf8"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("荣👋", "utf8"));

  // gbk
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("Hello", "gbk"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("荣", "gbk"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("鎔", "gbk"));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("𤘺", "gbk"));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("👋", "gbk"));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("荣👋", "gbk"));

  // gb2312
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("Hello", "gb2312"));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("荣", "gb2312"));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("鎔", "gb2312"));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("𤘺", "gb2312"));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("👋", "gb2312"));

  // gbk+emoji
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("👋", "gbk", true));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("荣👋", "gbk", true));

  // gb2312+emoji
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("👋", "gb2312", true));
  EXPECT_FALSE(ExtendedCharsetFilter::FilterText("荣👋", "gb2312", true));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("♂️", "gb2312", true));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("✈️", "gb2312", true));

  // emoji joining
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("🙋‍♂️", "gb2312", true));
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("🙋", "gb2312", true));
  // emoji 12.0
  EXPECT_TRUE(ExtendedCharsetFilter::FilterText("🥓", "gb2312", true));
}

// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "ui/text_markup.h"

using namespace Lingmo::ui;

TEST(StyledTextSpans, KeepsBlocksAndInlineStyles)
{
    const std::vector<Label::Span> spans = styledTextSpans(
            "<h2>What is New</h2>"
            "<p>Hello <b>bold</b> &amp; <a href=\"https://example.com\">link</a></p>"
            "<ul><li>One</li><li>Two</li></ul>",
            Color::rgb(0x0067FF));

    std::string text;
    bool bold = false;
    bool link = false;
    for (const Label::Span &span : spans) {
        text += span.text;
        bold |= span.weight >= 700;
        link |= !span.link.empty();
    }

    EXPECT_NE(text.find("What is New"), std::string::npos);
    EXPECT_NE(text.find("bold"), std::string::npos);
    EXPECT_NE(text.find("&"), std::string::npos);
    EXPECT_NE(text.find("• One"), std::string::npos);
    EXPECT_NE(text.find("• Two"), std::string::npos);
    EXPECT_TRUE(bold);
    EXPECT_TRUE(link);
}

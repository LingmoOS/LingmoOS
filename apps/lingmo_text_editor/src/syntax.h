// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNTAX_H
#define SYNTAX_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

enum class SyntaxStyle : uint8_t {
    Normal,
    Keyword,
    Type,
    Constant,
    String,
    Escape,
    Number,
    Comment,
    Preprocessor,
    Function,
    Variable,
    Heading,
    Emphasis,
    Strong,
    Code,
    Link,
    Tag,
    Attribute,
    Key,
    Section,
    Added,
    Removed,
    DiffHeader,
    Error,
    Warning,
    Count,
};

// UTF-8 byte offsets into the line.
struct SyntaxSpan
{
    int start;
    int length;
    SyntaxStyle style;
};

struct Language;

namespace Syntax
{
// Line states carried from one line to the next; 0 is the plain state.
using State = int;

// Sorted by name.
const std::vector<const Language *> &languages();
const Language *language(std::string_view id);
std::string id(const Language *language);
std::string name(std::string_view id);

// From the file name, then the first line (a shebang or an XML prolog),
// then the MIME database. Empty for plain text.
std::string detect(const std::string &fileName, std::string_view firstLine);

// Colours `text` given the state at the end of the previous line, and
// returns the state at the end of this one.
State highlight(const Language *language, std::string_view text, State state, std::vector<SyntaxSpan> *spans);
}

#endif

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTCODEC_H
#define TEXTCODEC_H

#include <string>
#include <string_view>
#include <vector>

enum class LineEnding {
    Unix,
    Windows,
    ClassicMac,
};

struct DecodedText
{
    // UTF-8.
    std::vector<std::string> lines;
    // An ICU converter name, e.g. "UTF-8", "UTF-16LE", "GB18030".
    std::string encoding;
    bool bom = false;
    LineEnding lineEnding = LineEnding::Unix;
    bool mixedLineEndings = false;
    // NUL bytes or mostly control characters.
    bool binary = false;
    // Some bytes did not decode and became U+FFFD.
    bool malformed = false;
};

struct EncodingInfo
{
    const char *name;
    const char *label;
};

namespace TextCodec
{
// Guesses the encoding from a BOM, the UTF-16 NUL pattern, UTF-8 validity and
// the byte statistics of the legacy encodings, preferring the locale's own.
std::string detect(std::string_view data, bool *bom = nullptr);

// Decodes with the given encoding, or the detected one when it is empty.
DecodedText decode(std::string_view data, const std::string &encoding = std::string());

// Joins the lines with the line ending and encodes them; lossy is set when a
// character has no representation in the encoding.
std::string encode(const std::vector<std::string> &lines, const std::string &encoding, bool bom,
                   LineEnding lineEnding, bool *lossy = nullptr);

bool isBinary(std::string_view data);
bool isSupported(const std::string &encoding);

// Splits at \n, \r\n and lone \r, and reports which of them the text uses most.
std::vector<std::string> splitLines(std::string_view text, LineEnding *dominant = nullptr, bool *mixed = nullptr);

std::string_view lineEndingText(LineEnding lineEnding);

// The legacy encoding text in the system locale's language most likely uses.
std::string localeEncoding();
// The same for a locale name such as "zh_TW".
std::string localeEncoding(const std::string &localeName);

// What the encoding menu offers, in display order.
const std::vector<EncodingInfo> &encodings();
}

#endif

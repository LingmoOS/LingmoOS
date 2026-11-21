// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "display/edid.h"

#include "base/hash.h"
#include "base/strings.h"

namespace Lingmo::Display {

namespace {

bool isAsciiSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

// Latin-1 whitespace at either end, the two non-ASCII spaces included.
std::string_view trimmedLatin1(std::string_view text)
{
    const auto space = [](char c) {
        return isAsciiSpace(c) || static_cast<unsigned char>(c) == 0x85
                || static_cast<unsigned char>(c) == 0xa0;
    };
    while (!text.empty() && space(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && space(text.back()))
        text.remove_suffix(1);
    return text;
}

std::string_view trimmedAscii(std::string_view text)
{
    while (!text.empty() && isAsciiSpace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && isAsciiSpace(text.back()))
        text.remove_suffix(1);
    return text;
}

} // namespace

EdidInfo parseEdid(std::string_view edid)
{
    EdidInfo info;
    constexpr std::string_view header("\x00\xff\xff\xff\xff\xff\xff\x00", 8);
    if (edid.size() < 128 || edid.substr(0, 8) != header)
        return info;
    unsigned checksum = 0;
    for (size_t i = 0; i < 128; ++i)
        checksum += static_cast<unsigned char>(edid[i]);
    if (checksum & 255)
        return info;

    std::string key(edid.substr(8, 8));
    for (size_t offset = 54; offset + 18 <= 126; offset += 18) {
        if (edid.substr(offset, 4) == std::string_view("\x00\x00\x00\xff", 4))
            key += trimmedAscii(edid.substr(offset + 5, 13));
        // Monitor name, read as ui/display/util/edid_parser.cc does.
        if (edid.substr(offset, 5) == std::string_view("\x00\x00\x00\xfc\x00", 5))
            info.model = base::strings::latin1ToUtf8(trimmedLatin1(edid.substr(offset + 5, 13)));
    }
    info.key = base::CryptographicHash::hashHex(
            key, base::CryptographicHash::Algorithm::Sha256);

    // Three 5-bit letters, big endian, 1 for 'A'.
    const unsigned manufacturer = (unsigned(static_cast<unsigned char>(edid[8])) << 8)
            | static_cast<unsigned char>(edid[9]);
    if (manufacturer) {
        for (int shift : {10, 5, 0})
            info.make += char('A' - 1 + ((manufacturer >> shift) & 31));
    }
    return info;
}

} // namespace Lingmo::Display

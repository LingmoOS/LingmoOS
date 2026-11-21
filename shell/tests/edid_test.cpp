// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "base/strings.h"
#include "display/edid.h"

using namespace Lingmo::Display;
using Lingmo::base::strings::fromHex;

namespace {

std::string hex(std::string_view text)
{
    return fromHex(text).value();
}

std::string descriptor(unsigned char tag, const std::string &text)
{
    std::string result = hex("000000") + char(tag) + '\0';
    std::string padded = (text + '\n').substr(0, 13);
    padded.resize(13, ' ');
    return result + padded;
}

// A 128-byte base block from a Dell monitor, reduced to what the parser reads.
std::string edid(const std::optional<std::string> &name, bool validChecksum = true)
{
    std::string block = hex("00ffffffffffff00");
    block += hex("10ac7ea04c4d5a30"); // "DEL", product, serial number
    block += std::string(54 - block.size(), '\0');
    block += hex("565e00a0a0a029503020350055502100001a"); // Detailed timing.
    block += descriptor(0xff, "ABC1234");
    block += name ? descriptor(0xfc, *name) : descriptor(0x10, {});
    block += descriptor(0x10, {});
    block += std::string(127 - block.size(), '\0');
    unsigned sum = 0;
    for (char byte : block)
        sum += static_cast<unsigned char>(byte);
    block += char((256 - sum % 256) % 256 + (validChecksum ? 0 : 1));
    return block;
}

// SHA-256 of the vendor, product and serial bytes and "ABC1234".
constexpr const char *kDellKey = "1cef42bf6b30eeafa4758e08eee61214ebde6f51215038162873d639632e2b3c";

} // namespace

TEST(Edid, ReadsManufacturerAndMonitorName)
{
    const auto info = parseEdid(edid("DELL U2720Q"));
    EXPECT_EQ(info.make, "DEL");
    EXPECT_EQ(info.model, "DELL U2720Q");
}

TEST(Edid, KeyCoversVendorProductSerialOnly)
{
    EXPECT_EQ(parseEdid(edid("DELL U2720Q")).key, kDellKey);
    // The name is not part of the identity.
    EXPECT_EQ(parseEdid(edid("Other name")).key, kDellKey);
}

// The name descriptor is Latin-1; its padding may use the no-break space.
TEST(Edid, ModelIsLatin1TrimmedAsText)
{
    EXPECT_EQ(parseEdid(edid("Caf\xe9\xa0")).model, "Caf\xc3\xa9");
}

TEST(Edid, PanelWithoutNameDescriptorHasEmptyModel)
{
    const auto info = parseEdid(edid(std::nullopt));
    EXPECT_EQ(info.make, "DEL");
    EXPECT_TRUE(info.model.empty());
    EXPECT_FALSE(info.key.empty());
}

TEST(Edid, RejectsBadChecksumShortBlockAndMissingHeader)
{
    EXPECT_TRUE(parseEdid(edid("DELL U2720Q", false)).key.empty());
    EXPECT_TRUE(parseEdid(edid("DELL U2720Q").substr(0, 127)).make.empty());
    std::string noHeader = edid("DELL U2720Q");
    noHeader[0] = 1;
    EXPECT_TRUE(parseEdid(noHeader).model.empty());
}

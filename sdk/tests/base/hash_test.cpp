// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "base/hash.h"
#include "base/strings.h"

using namespace Lingmo::Base;

using Algorithm = CryptographicHash::Algorithm;

TEST(HashTest, Sha256Vectors)
{
    EXPECT_EQ(CryptographicHash::hashHex("", Algorithm::Sha256),
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    EXPECT_EQ(CryptographicHash::hashHex("abc", Algorithm::Sha256),
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    EXPECT_EQ(CryptographicHash::hashHex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", Algorithm::Sha256),
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    EXPECT_EQ(CryptographicHash::hashHex(std::string(1000000, 'a'), Algorithm::Sha256),
              "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

TEST(HashTest, Sha1Vectors)
{
    EXPECT_EQ(CryptographicHash::hashHex("", Algorithm::Sha1), "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    EXPECT_EQ(CryptographicHash::hashHex("abc", Algorithm::Sha1), "a9993e364706816aba3e25717850c26c9cd0d89d");
    EXPECT_EQ(CryptographicHash::hashHex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", Algorithm::Sha1),
              "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
}

TEST(HashTest, PiecesMatchWhole)
{
    std::string data;
    for (int i = 0; i < 300; ++i)
        data += char(i * 7);
    for (Algorithm algorithm : {Algorithm::Sha1, Algorithm::Sha256}) {
        for (size_t step : {1u, 3u, 55u, 56u, 63u, 64u, 65u, 200u}) {
            CryptographicHash hash(algorithm);
            for (size_t i = 0; i < data.size(); i += step)
                hash.addData(std::string_view(data).substr(i, step));
            EXPECT_EQ(hash.result(), CryptographicHash::hash(data, algorithm)) << step;
        }
    }
    EXPECT_EQ(CryptographicHash::hash("x", Algorithm::Sha256).size(), 32u);
    EXPECT_EQ(CryptographicHash::hash("x", Algorithm::Sha1).size(), 20u);
}

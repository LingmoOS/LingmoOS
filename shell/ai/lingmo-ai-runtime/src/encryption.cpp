/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "encryption.h"

#include <openssl/evp.h>

#include "aes256.h"

static const char* rsaPrivateKey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEArw+9L+6kvFl5vTN5QeSWPUH8fmDeT+UEqmmleYPZlGzUtVDV
x+A7rwrhebm6wkEHTXlED2wAnNsksOYPQt9MKWTMF+zuv8oWH8OfSzKC1QTTIY74
KRJz4zkVHujJ87JcCw0gTiK1KQBebO0JJUWfwrxnGh/sn3SeuWPbk44DpxfKzOBZ
DXEiWYGCMdWLRBGsyexj/a+Yhdj2/9yDHOaFrQrMKRnipImjcZMLzWY5jxCTfoOY
0AITM0PYQfIfhsSXNdXjh/DzO/SzQdUsba2fvynrn0T2pNHB93kxFBzmxuAHlieu
ECVhLprhGYfEpxlKB4m4yLn9BVjFVbzaP329UwIDAQABAoIBAAzb7RuuPe+P6v9Y
3plWfyq0xCVMXEgmHHegFwWcB8l9TQqci3eJIxkyhWl5mRxXxn+MXtF7L3hWKJXN
8GULHf8UZmGDh+4SnBltNzBA35diKVgXtmbd8iFIuiJxGxSJj3VIW40hw4jlfAoo
RzmzqY6WZERo/+qemGIp0aE/F0mYuK+YTOqNaigeyIBs1bXzNrMe68E86WYNMvPd
201qHAjlaYAmTooJzFP9NNOwCbtPXdFB/sh4GEGSOpx2Gku509WqaP1z9bLNmqfl
PaW7403F0ulSCp51O+w/DcCu3MGczxzdqXvu4DkR6DcATC4LX2YTVNLJ4xH8KCjQ
Inu0GdECgYEA27JlyHxdeA0IFHc8a4bMoI/PJfIwUGvrcMFAaBIqjpJdF223CZG3
l43FMSMk7zxf5XQkTPuWEKVQSqC4YLU9imOJGhjp5UFRHXdjTJVsVSVlq9NraJQA
j+as/BrP+83WN1mBNVbTP8xg0zr74twk008EH951Kz0zgmCQ2Wx6gfcCgYEAy/0t
pE28TxHSanlUn4m+eOfQg0ZG4P2a8DmsopQtlU8kRbgZbOQBdkH0T9KEpRRpyVdH
Jq/J4likGGNk/gDZf+0WuQ/k9ZU2R4n/kMUitkBeARdrKQAghGnEH7du4Blk0ORw
h73gAGqEAowXyMfk9x9Uaa+8KK6OA9Y03kRbiIUCgYBT3eRRBni/7WmaNnJQMeNH
n95aA/xBlxEHuB4YHGc2idUTroRjGMdXCX2vZlUhJpENDKsW4CvHKvRdqMD52fYm
MB0UY1JWe/q1HvA/OgNi9aW8VDb6cxVL2alLjAi9fTQ7aOWUMRLvlduYhznEvnko
WU+fwtJDsVdKfAaPmYOivQKBgQCHEsqaNW8v3Fk8b8L9MTSS0s1bX7mnWcaysz4B
GaVlbNZT5CfR1tiw+dNQ19FFjlLOI6DVmSlRW+iszYQePWnIL7IQMNonU/pxPIuh
zSfDDrQwfNVUfOULSQ25ArnEYoxDVkC9YZlmLSHknedR6CtRB8nZgahSN+OJ3tl7
JECIEQKBgQCSna58O25Ap4khqbo4oAf/mXCZ4TAS4YibblTe4QGR7mvDKxn3xZQ4
91Z0P3qpdghy77Must9kNajoH5nwwQoHtLlaHTxCG+p6iNSwprFrlPifCUhM/GgH
/wVEa5KktoFLMyHbZD23P5ThE6+K96f92QQmHV/Vlzuq+R+o+KOJjw==
-----END RSA PRIVATE KEY-----)";

// 1. 原始密码：Hello, Lingmo AI SDK!
// 2. 密码中保护结束的英文感叹号
// 3. 可以将testrsa2048中RSA_public_encrypt函数加密的
// 结果打印出来，得到下面的16进制数据
std::vector<unsigned char> encodedPbkdf2Key = {
    0x34, 0xf1, 0xfb, 0xf6, 0x88, 0x1e, 0x76, 0x3b, 0x7e, 0xfa, 0xc2, 0x7c, 0xc0, 0x28, 0x9a, 0x64,
    0x90, 0x77, 0x67, 0x93, 0xa4, 0x57, 0xad, 0x83, 0x25, 0x2b, 0x56, 0x3c, 0x05, 0xef, 0x08, 0x9d,
    0xf3, 0xfc, 0xa9, 0x43, 0xff, 0xd0, 0xca, 0x01, 0x75, 0x36, 0xa1, 0xf0, 0x69, 0x1c, 0x47, 0x08,
    0x38, 0x95, 0x7f, 0xe1, 0xe4, 0x45, 0xb8, 0x6e, 0xec, 0xd0, 0x6d, 0xd5, 0x63, 0x2a, 0xcc, 0x7c,
    0x7e, 0x36, 0x41, 0x58, 0xac, 0x7a, 0x6a, 0x06, 0x2d, 0x1d, 0x86, 0x18, 0xfe, 0x41, 0x93, 0x6b,
    0xf3, 0xf9, 0xce, 0x49, 0x8c, 0xd6, 0xc7, 0x18, 0x08, 0x67, 0x38, 0x0f, 0x5e, 0xe7, 0x56, 0x21,
    0x91, 0x7e, 0xad, 0x32, 0xa8, 0x15, 0x96, 0xff, 0x64, 0x24, 0x45, 0x4b, 0xdd, 0x80, 0xc2, 0xf2,
    0xf0, 0x90, 0x34, 0x22, 0xc7, 0x30, 0x53, 0x8d, 0xf7, 0xe8, 0x7e, 0xdc, 0x63, 0xe6, 0xab, 0xb7,
    0x8b, 0xf1, 0x9f, 0x84, 0x15, 0x4e, 0x6b, 0xe7, 0x90, 0x99, 0x02, 0xdc, 0x80, 0x89, 0x4f, 0xfa,
    0xf4, 0x6d, 0xb7, 0x64, 0xbe, 0xb9, 0xc8, 0x27, 0xe5, 0xa2, 0xb9, 0xdb, 0x39, 0x0a, 0x33, 0x0e,
    0x3b, 0xc4, 0xe4, 0xe7, 0x11, 0xbd, 0xf8, 0x97, 0xf8, 0x5e, 0x4a, 0x88, 0x5a, 0x19, 0xbc, 0x40,
    0x84, 0xce, 0x05, 0xea, 0xc5, 0x78, 0x8b, 0xd5, 0x8e, 0x12, 0x72, 0x1c, 0x3b, 0xaf, 0x04, 0xc1,
    0xa5, 0x95, 0xf3, 0xcd, 0xa1, 0x45, 0x12, 0x2c, 0xb0, 0x42, 0xc0, 0xeb, 0x40, 0x32, 0x0c, 0x9e,
    0x8c, 0x85, 0xa6, 0x2a, 0x08, 0x0b, 0x63, 0xdf, 0x8a, 0x07, 0xba, 0x1a, 0xb6, 0xd5, 0x5c, 0x76,
    0xe6, 0xcb, 0x22, 0x13, 0xaf, 0x8e, 0xfd, 0x31, 0xbc, 0x17, 0x9c, 0x5d, 0x31, 0x0e, 0x52, 0x6a,
    0x76, 0x61, 0xa0, 0xde, 0x5e, 0x5b, 0x96, 0x54, 0xb2, 0x68, 0xdb, 0x2c, 0x82, 0x55, 0x51, 0xc9
};

const Encryption& Encryption::getInstance() {
    static Encryption encryption;

    return encryption;
}

Encryption::Encryption()
    : rsa_(new Rsa2048(rsaPrivateKey)) {

}

std::string Encryption::encrypt(const std::string& input) const {
    auto plainKey = rsa_->decrypt(encodedPbkdf2Key);
    auto result = pbkdf2_->generate(plainKey);
    if (!result.success) {
        return "";
    }

    Aes256 aes(result.key, result.salt);
    auto encrypted = aes.encrypt(input);
    std::vector<unsigned char> from(result.salt.begin(), result.salt.end());
    from.insert(from.end(), encrypted.begin(), encrypted.end());

    return base64Encode(from);
}

std::string Encryption::decrypt(const std::string& input) const{
    auto from = base64Decode(input);
    if (from.empty()) {
        return "";
    }

    // 从base64解码后的数据中提取前16位
    // 对应的盐值。提取完成后，将盐值从
    // 原始数据中删除
    Pbkdf2::Salt salt;
    for (std::size_t i = 0; i < salt.size(); i++) {
        salt[i] = from[i];
    }
    from.erase(from.begin(), from.begin() + salt.size());

    auto plainKey = rsa_->decrypt(encodedPbkdf2Key);
    auto result = pbkdf2_->generate(plainKey, salt);
    if (!result.success) {
        return "";
    }

    Aes256 aes(result.key, salt);

    return aes.decrypt(from);
}

std::string Encryption::base64Encode(const std::vector<unsigned char>& input) {
    if (input.empty()) {
        return "";
    }

    auto buffSize = input.size() * 4 / 3 + 4;
    std::vector<unsigned char> buffer(buffSize, '\0');
    auto size = EVP_EncodeBlock(buffer.data(), input.data(), input.size());
    return std::string((const char*)buffer.data(), size);
}

static std::size_t countEqualSign(const std::string& input) {
    std::size_t counter = 0;

    for (auto iter = input.rbegin(); iter != input.rend(); iter++) {
        if (*iter != '=') {
            break;
        }

        counter++;
    }

    return counter;
}

std::vector<unsigned char> Encryption::base64Decode(const std::string& input) {
    if (input.empty()) {
        return std::vector<unsigned char>();
    }
    auto equalSignCounter = countEqualSign(input);

    auto buffSize = input.length() * 3 / 4;
    std::vector<unsigned char> buffer(buffSize, '\0');
    auto length = EVP_DecodeBlock(buffer.data(), (const unsigned char*)input.data(), input.length());
    if (length == -1) {
        return std::vector<unsigned char>();
    }

    return std::vector<unsigned char>(buffer.data(), buffer.data() + length - equalSignCounter);
}

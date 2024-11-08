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

#include <assert.h>

#include <string.h>
#include <openssl/evp.h>

void testBase64Encode() {
    std::string str = "Hello, World!";
    std::vector<unsigned char> from(str.begin(), str.end());

    auto result = Encryption::base64Encode(from);
    assert(result == "SGVsbG8sIFdvcmxkIQ==");
}

void testBase64Decode() {
    std::string str = "Hello, World!";
    std::vector<unsigned char> from(str.begin(), str.end());

    auto encoded = Encryption::base64Encode(from);
    auto decoded = Encryption::base64Decode(encoded);
    
    assert(from == decoded);
}

void testEncryptAndDecrypt() {
    const char* testKey = "TestKey";
    const auto& encryption = Encryption::getInstance();
    auto encrypted = encryption.encrypt(testKey);
    auto decrypted = encryption.decrypt(encrypted);

    assert(decrypted == testKey);
}

int main(int argc, char* argv[]) {
    testBase64Encode();
    testBase64Decode();
    testEncryptAndDecrypt();
    return 0;
}
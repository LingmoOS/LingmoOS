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

#include "aes256.h"

#include <openssl/aes.h>

std::vector<unsigned char> Aes256::encrypt(const std::string& input) {
    if (input.empty()) {
        return std::vector<unsigned char>();
    }

    // 将字符串结尾的0考虑在内，方便解密的时候获取原始的字符串
    const auto length = input.length() + 1;
    constexpr std::size_t blockSize = AES_BLOCK_SIZE;
   // 确保输出缓存的大小是AES_BLOCK_SIZE（16）的倍数
    std::size_t dataSize = (length + (blockSize - 1) ) & ~(blockSize - 1);
    std::vector<unsigned char> data(dataSize);

    AES_KEY aes_key;
    // 使用256位AES加密
    AES_set_encrypt_key(key_.data(), 256, &aes_key);
    // AES_cbc_encrypt会修改ivec中的内容，因此，
    // 每次调用该函数之前都复制一份新的ivec
    IvecData iVdata = data_;
    AES_cbc_encrypt((const unsigned char*)input.c_str(), data.data(), length, &aes_key, iVdata.data(), AES_ENCRYPT);

    return data;
}

std::string Aes256::decrypt(const std::vector<unsigned char>& input) {
    if (input.empty()) {
        return std::string();
    }

    std::vector<unsigned char> data(input.size());

    AES_KEY aes_key;
    // 使用256位AES解密
    AES_set_decrypt_key(key_.data(), 256, &aes_key);
    // AES_cbc_encrypt会修改ivec中的内容，因此，
    // 每次调用该函数之前都复制一份新的ivec
    IvecData iVdata = data_;
    AES_cbc_encrypt(input.data(), data.data(), input.size(), &aes_key, iVdata.data(), AES_DECRYPT);

    return std::string((const char*)data.data());
}
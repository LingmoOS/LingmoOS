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

#ifndef AES256_H
#define AES256_H

#include <array>
#include <string>
#include <vector>

// 使用AES 256进行加解密，32个字节对应256位
using Aes256Key = std::array<unsigned char, 32>;
// AES ivec必须是16个字节
using IvecData = std::array<unsigned char, 16>;

class Aes256 {
public:
    Aes256(const Aes256Key& key, const IvecData& data)
        : key_(key), data_(data) {

    }

    ~Aes256() = default;

    std::vector<unsigned char> encrypt(const std::string& input);
    std::string decrypt(const std::vector<unsigned char>& input);

private:
    Aes256Key key_;
    IvecData data_;
};

#endif
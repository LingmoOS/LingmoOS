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

#ifndef PBKDF2_H
#define PBKDF2_H

#include <array>
#include <vector>
#include <string>

class Pbkdf2 {
public:
    // 采用ANSI推荐的长度：128位
    constexpr static int SALT_SIZE = 16;
    using Salt = std::array<unsigned char, SALT_SIZE>;

    struct ResultBase {
        bool success = false;

        // 为AES生成的256位的key
        std::array<unsigned char, 32> key;
    };

    struct Result : public ResultBase {
        Salt salt;
    };

public:
    Pbkdf2();

    ~Pbkdf2() = default;

    ResultBase generate(const std::string& password, const Salt& salt);
    Result generate(const std::string& password);
    
private:
    // RFC 2898建议最小迭代次数：1000
    constexpr static int iterations_ = 1000;

    // 256位密钥长度
    constexpr static int keyLength_ = 32;
};

#endif

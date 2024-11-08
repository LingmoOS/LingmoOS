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

#ifndef RSA2048_H
#define RSA2048_H

#include <string>
#include <vector>

#include <openssl/rsa.h>

class Rsa2048 {
public:
    explicit Rsa2048(const std::string& privateKey);

    ~Rsa2048();

    std::string decrypt(const std::vector<unsigned char>& input);

private:
    RSA* rsa_ = nullptr;
};

#endif
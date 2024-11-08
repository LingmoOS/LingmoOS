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

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include "pbkdf2.h"
#include "rsa2048.h"

#include <memory>
#include <string>
#include <vector>

class Encryption {
public:
    static const Encryption& getInstance();

    std::string encrypt(const std::string& input) const;
    std::string decrypt(const std::string& input) const;

    static std::string base64Encode(const std::vector<unsigned char>& input);
    static std::vector<unsigned char> base64Decode(const std::string& input);

private:
    Encryption();

private:
    std::unique_ptr<Rsa2048> rsa_ = nullptr;
    std::unique_ptr<Pbkdf2> pbkdf2_ = nullptr;
};

#endif
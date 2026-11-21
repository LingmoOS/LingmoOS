// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/password.h"

#include <crypt.h>
#include <random>

namespace settings {

std::string cryptPassword(const std::string& password)
{
    static constexpr std::string_view alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZ"
                                                 "abcdefghijklmnopqrstuvxyz./";
    std::random_device random;
    std::uniform_int_distribution<size_t> pick(0, alphabet.size() - 1);
    std::string salt = "$6$";
    for (int i = 0; i < 16; ++i)
        salt += alphabet[pick(random)];

    crypt_data data = {};
    const char* hashed = crypt_r(password.c_str(), salt.c_str(), &data);
    return hashed ? std::string(hashed) : std::string();
}

} // namespace settings

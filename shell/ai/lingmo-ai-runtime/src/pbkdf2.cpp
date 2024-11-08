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

#include "pbkdf2.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

Pbkdf2::Pbkdf2() {
    RAND_poll();
}

Pbkdf2::ResultBase Pbkdf2::generate(const std::string& password, const Salt& salt) {
    Pbkdf2::ResultBase result;

    result.success = (bool)PKCS5_PBKDF2_HMAC(password.data(), password.length(),  salt.data(),
        salt.size(), iterations_, EVP_sha256(), keyLength_, result.key.data());

    return result;
}

Pbkdf2::Result Pbkdf2::generate(const std::string& password) {
    Pbkdf2::Result result;

    // 更新盐值
    RAND_bytes(result.salt.data(), result.salt.size());

    auto base  = generate(password, result.salt);

    result.success = base.success;
    result.key = base.key;

    return result;
}

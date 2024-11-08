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

#include "rsa2048.h"

#include <openssl/pem.h>

Rsa2048::Rsa2048(const std::string& privateKey) {
    BIO* bio = BIO_new_mem_buf(privateKey.c_str(), privateKey.length());
    if (bio == nullptr) {
        return;
    }

    rsa_ = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);

    BIO_free(bio);
}

Rsa2048::~Rsa2048() {
    if (rsa_ != nullptr) {
        RSA_free(rsa_);
    }
}

std::string Rsa2048::decrypt(const std::vector<unsigned char>& input) {
    std::string text;

    unsigned char decryptedData[RSA_size(rsa_)] = { 0 };

    auto status = RSA_private_decrypt(RSA_size(rsa_), input.data(),
        decryptedData, rsa_, RSA_PKCS1_PADDING);
    if (status == -1) {
        return "";
    }

    return std::string((const char*)decryptedData, status);
}
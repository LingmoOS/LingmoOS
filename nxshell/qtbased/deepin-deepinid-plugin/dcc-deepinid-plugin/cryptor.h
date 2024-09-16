// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CRYPTOR_H
#define CRYPTOR_H
#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <QString>

class Cryptor
{
public:
    Cryptor();

    static bool RSAPublicEncryptData(const std::string &rsakey, const QString &strin, QByteArray &strout);
};

#endif // CRYPTOR_H

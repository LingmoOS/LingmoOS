// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SSLCERTCONF_H
#define SSLCERTCONF_H

#include <string>

#include <ghc/fs_fwd.hpp>

namespace gfs = ghc::filesystem;

class SslCertConf
{
public:
    static SslCertConf *ins();

    bool generateCertificate(const std::string &profile);
    std::string getFingerPrint();
    void writeTrustPrint(bool server, const std::string &print);

private:
    SslCertConf();

    bool generate_fingerprint(const gfs::path &cert_path);

    bool is_certificate_valid(const gfs::path &path);

    std::string _fingerPrint { "" };
};

#endif // SSLCERTCONF_H

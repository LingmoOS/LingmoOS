// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sslcertconf.h"
#include "common/DataDirectories.h"
#include "base/finally.h"
#include "io/filesystem.h"
#include "net/FingerprintDatabase.h"
#include "net/SecureUtils.h"

#include <iostream>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

SslCertConf *SslCertConf::ins()
{
    static SslCertConf ins;
    return &ins;
}

bool SslCertConf::generateCertificate(const std::string &profile)
{
    // first set the default profile
    auto profile_path = barrier::fs::u8path(profile.c_str());
    barrier::DataDirectories::profile(profile_path);

    // check and create a self sign cert
    auto cert_path = barrier::DataDirectories::ssl_certificate_path();
    if (!barrier::fs::exists(cert_path) || !is_certificate_valid(cert_path)) {
        try {
            auto cert_dir = cert_path.parent_path();
            if (!barrier::fs::exists(cert_dir)) {
                barrier::fs::create_directories(cert_dir);
            }

            barrier::generate_pem_self_signed_cert(cert_path.u8string());
        }  catch (const std::exception &e) {
            std::cout << "SSL tool failed: " << e.what() << std::endl;
            return false;
        }

        //std::cout << "SSL certificate generated." << std::endl;
    }

    // gen the fingerprint by cert.
    return generate_fingerprint(cert_path);
}

std::string SslCertConf::getFingerPrint()
{
    return _fingerPrint;
}

void SslCertConf::writeTrustPrint(bool server, const std::string &print)
{
    auto trust_path = barrier::DataDirectories::local_ssl_fingerprints_path(); //default: Local
    if (server) {
        trust_path = barrier::DataDirectories::trusted_servers_ssl_fingerprints_path();
    } else {
        trust_path = barrier::DataDirectories::trusted_clients_ssl_fingerprints_path();
    }
    auto trust_dir = trust_path.parent_path();
    if (!barrier::fs::exists(trust_dir)) {
        barrier::fs::create_directories(trust_dir);
    }

    barrier::FingerprintDatabase db;
    auto sha256 = db.parse_db_line(print);
    db.add_trusted(sha256);
    db.write(trust_path);
}

SslCertConf::SslCertConf()
{
}

bool SslCertConf::generate_fingerprint(const gfs::path &cert_path)
{
    try {
        barrier::FingerprintDatabase db;
        auto sha256 = barrier::get_pem_file_cert_fingerprint(cert_path.u8string(),
                                                             barrier::FingerprintType::SHA256);

        _fingerPrint = db.to_db_line(sha256);

        //std::cout << "SSL fingerprint generated: " << _fingerPrint << std::endl;

    } catch (const std::exception &e) {
        std::cout << "Failed to find SSL fingerprint. " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool SslCertConf::is_certificate_valid(const gfs::path &path)
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    auto fp = barrier::fopen_utf8_path(path, "r");
    if (!fp) {
        std::cout << "Could not read from default certificate file." << std::endl;
        return false;
    }
    auto file_close = barrier::finally([fp]() { std::fclose(fp); });

    auto *cert = PEM_read_X509(fp, nullptr, nullptr, nullptr);
    if (!cert) {
        std::cout << "Error loading default certificate file to memory." << std::endl;
        return false;
    }
    auto cert_free = barrier::finally([cert]() { X509_free(cert); });

    auto *pubkey = X509_get_pubkey(cert);
    if (!pubkey) {
        std::cout << "Default certificate key file does not contain valid public key" << std::endl;
        return false;
    }
    auto pubkey_free = barrier::finally([pubkey]() { EVP_PKEY_free(pubkey); });

    auto type = EVP_PKEY_type(EVP_PKEY_id(pubkey));
    if (type != EVP_PKEY_RSA && type != EVP_PKEY_DSA) {
        std::cout << "Public key in default certificate key file is not RSA or DSA" << std::endl;
        return false;
    }

    auto bits = EVP_PKEY_bits(pubkey);
    if (bits < 2048) {
        // We could have small keys in old barrier installations
        std::cout << "Public key in default certificate key file is too small." << std::endl;
        return false;
    }

    return true;
}

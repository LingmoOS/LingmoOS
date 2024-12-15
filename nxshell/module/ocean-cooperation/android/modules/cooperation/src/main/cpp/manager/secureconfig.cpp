// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "secureconfig.h"

#include "configs/crypt/cert.h"

// SecureConfig::SecureConfig() {
// }


std::shared_ptr<CppServer::Asio::SSLContext> SecureConfig::serverContext()
{
    auto rsa_crt = Cert::instance()->getRSACrt();
    auto rsa_key = Cert::instance()->getRSAKey();
    asio::const_buffer cert_buf(rsa_crt.data(), rsa_crt.size());
    asio::const_buffer key_buf(rsa_key.data(), rsa_key.size());

    // Create and prepare a new SSL server context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv13);
    context->use_certificate(cert_buf, asio::ssl::context::pem);
    context->use_rsa_private_key(key_buf, asio::ssl::context::pem);

    return context;
}


std::shared_ptr<CppServer::Asio::SSLContext> SecureConfig::clientContext()
{
    auto rsa_crt = Cert::instance()->getRSACrt();
    asio::const_buffer cert_buf(rsa_crt.data(), rsa_crt.size());

    // Create and prepare a new SSL client context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv13);
    // context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->use_certificate(cert_buf, asio::ssl::context::pem);

    return context;
}

// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tokencache.h"
#include "filesystem/path.h"
#include "jwt-cpp/jwt.h"

#include "configs/crypt/cert.h"

std::string TokenCache::genToken(std::string info)
{
    const std::string es256k_priv_key = Cert::instance()->getPriEs256k();
    const std::string es256k_pub_key = Cert::instance()->getPubEs256k();

    auto token = jwt::create()
                     .set_issuer("deepin")
                     .set_type("JWT")
                     .set_id("dde-cooperation")
                     .set_issued_now()
                     .set_expires_in(std::chrono::seconds{36000})
                     .set_payload_claim("web", jwt::claim(info))
                     .sign(jwt::algorithm::es256k(es256k_pub_key, es256k_priv_key, "", ""));

    return token;
}

bool TokenCache::verifyToken(std::string &token)
{
    const std::string es256k_priv_key = Cert::instance()->getPriEs256k();
    const std::string es256k_pub_key = Cert::instance()->getPubEs256k();
    auto verifier = jwt::verify()
                          .allow_algorithm(jwt::algorithm::es256k(es256k_pub_key, es256k_priv_key, "", ""))
                          .with_issuer("deepin");

    try {
        auto decoded = jwt::decode(token);
        verifier.verify(decoded);
        //std::cout << "Token verify success!" << std::endl;
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

std::vector<std::string> TokenCache::getWebfromToken(const std::string &token)
{
    auto decoded = jwt::decode(token);
    std::vector<std::string> web_vector;

    try {
        const auto web_array = decoded.get_payload_claim("web").as_string();
        //std::cout << "web = " << web_array << std::endl;

        picojson::value v;
        std::string err = picojson::parse(v, web_array);
        if (!err.empty()) {
            std::cout << "json parse error:" << v << std::endl;
            return web_vector;
        }

        // Assuming web_array is an array of strings
        for(const auto& name : v.get<picojson::array>()) {
            web_vector.push_back(name.get<std::string>());
            //std::cout << "Name: " << name.get<std::string>() << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }
    
    return web_vector;
}

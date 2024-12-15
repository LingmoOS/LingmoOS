// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CERT_H
#define CERT_H

#include <iostream>
#include <fstream>
#include <filesystem>

class Cert {
public:
    static Cert *instance()
    {
        static Cert ins;
        return &ins;
    }

    std::string writeRSAKey(std::string &path)
    {
        //TODO: write file
        std::string fullpath = path + "/.cooperation/temp/keybin.tmp";

        return fullpath;
    }

    std::string writeRSACrt(std::string &path)
    {
        //TODO: write file
        std::string fullpath = path + "/.cooperation/temp/crtbin.tmp";

        return fullpath;
    }

    std::string getRSAKey()
    {
        return rsa_key;
    }

    std::string getRSACrt()
    {
        return rsa_crt;
    }

    std::string getPriEs256k()
    {
        return es256k_priv_key;
    }

    std::string getPubEs256k()
    {
        return es256k_pub_key;
    }

private:
    Cert() {};
    ~Cert() {};


    const std::string rsa_key = R"(-----BEGIN RSA PRIVATE KEY-----
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
-----END RSA PRIVATE KEY-----)";

    const std::string rsa_crt = R"(-----BEGIN CERTIFICATE-----
MIIFYTCCA0kCFCwhABWRbuJ4j0e4iPwLzbSl4uA1MA0GCSqGSIb3DQEBCwUAMG4x
CzAJBgNVBAYTAkNOMREwDwYDVQQIDAhwcm92aW5jZTENMAsGA1UEBwwEY2l0eTEV
MBMGA1UECgwMb3JnYW5pemF0aW9uMQ4wDAYDVQQLDAVncm91cDEWMBQGA1UEAwwN
ZGFwX3VuaW9udGVjaDAeFw0yMTA0MjAxMTUzNDlaFw0yMjA0MjAxMTUzNDlaMGwx
CzAJBgNVBAYTAkNOMREwDwYDVQQIDAhwcm92aW5jZTENMAsGA1UEBwwEY2l0eTEV
MBMGA1UECgwMb3JnYW5pemF0aW9uMQ4wDAYDVQQLDAVncm91cDEUMBIGA1UEAwwL
ZGFwX2Rlc2t0b3AwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDA6hVL
liIX6I4/EUQx01zGraJeCuqt2WuUuZmPjJAb1Yx+UQ6HfmWGqwMtCbzDYGV96rO9
2AkRCAhnzsSo7550lRC09qUAgcLGvXLt9EdXnNAYcIlMhxkX+VtB1ALwpoeIiPRF
UHzSHbJ7nXr3ac21hXcg/u5eOxtSpTu1mK0YsuTPpRoTyQSdM4NDAEFacZRIX7Ht
c/vBiJLBFdDevu1rzSYWxPiBzvPeEACw5BH1x7sXjMdC1mhdujIRD2M7QX6i9+gP
clLDcr5sdnD0U/tHKZKyJ3sK2+hYyDVoMwjPk4+nZEsvn3z8wGfduBwA1M1iTNdG
tSOr7FSr3S48nqDBbLQgwVCqN75gQUWrPR18mLc/TMQ5UmceLOVd6yCIYe/1SFKM
h3rHfgG1m5ATtQXqIWpLCYkdLJkxMZ/je2Mmp4StIDdvzAFMqnpGEyI8k9feZG39
5wBt9qIzw+syugVjFMbc7iNz7C9ebveXjomhn/TPYKyMwjGbS9h4KM9aARqyCXgI
4FuRI3EITLWr3vkec0EVGzXVDHDEEt2R0kclCTagwPV7b7iMKmN4/w3rbCBNYBP8
TlZG6+JXg+EHdO+NPjywMkYJvz2BhqRXh4UkU2mB9S+0caBMJ8kfUDRS7+IWunCY
gl3luAzsnrHKHbNBVU0sJ5qSjLLBOFJPpP8b4QIDAQABMA0GCSqGSIb3DQEBCwUA
A4ICAQBtawynL4mqiBUTr5FPZIJ+5iB6DhnGZGdPfrWnJTg2eRgH4vUl5YbDze79
vhaShUn/Dkw2Msvyq1lNhE2lqGmbl9yjHua4BQWvZL7IjfjJO/K+iUwa4OM0iixr
6pv/ozNOcW6B1+oleTkvhOQSTjzAQJ9vTXRAfyLlWVglspeqLd2tYsqfWrivsIq3
HHqYOrQIZYWKCcpLAtXJVUMbzgHOwK8M2oVUsAq4RWhoRg/2X6da7aLFKewAf/5V
C08KaJbRGbhW8aqUc+q8eOiZsv7h0G6J4CVIcX1BQVrpO0/KXkzs6yT3VlFrHFrp
Gl/4Mmc8i1omSDckhGy1kLxoG+TY9EHGsWRO7z8fAZfmeL2WwI6+XMPHhI3aaWaR
P7ECQmUfDYuiIlqova/qjL6hsQuvsLSHC4pmSqkxqoE6JAKlSlYj/rdYa53NUw6+
kNRoSIjCzEBnTCe/2ZqIyp8o8Il7wEGKYkL3GmBISlNM6DWyWSV7X5Xjn1iak/P8
yi2axFXuIs/QPngG3zWjxtTYC9S1/1LJ3cZyxnvtyu619RcR+Kn8rz5fj3v31p9m
o5TokWkbUodYfRhI8NtDmWalKpmplryTsLryMhKh7+z38RroTTQqNyD/6tnD7TjR
rCU2DxAi+Ukq3fQ75WmKilcOvVkWmLHWZ0865eGAf1hXGa/Nhg==
-----END CERTIFICATE-----)";

    // openssl ecparam -name secp256k1 -genkey -noout -out ec-secp256k1-priv-key.pem
    const std::string es256k_priv_key = R"(-----BEGIN EC PRIVATE KEY-----
MHQCAQEEIBhNAg7jWNLCGFgtEwwOvSobJ+oTQxPTF24RKZieTjw3oAcGBSuBBAAK
oUQDQgAEFwu6p6oT5RP/oI4S0NNO+nDq9CtDI4bcGkWvS8bfd1+u0tjgxVZQjkEc
mh/9NwX2BPuRXNzu6YzVibzUxXOsKA==
-----END EC PRIVATE KEY-----)";

    // openssl ec -in ec-secp256k1-priv-key.pem -pubout > ec-secp256k1-pub-key.pem
    const std::string es256k_pub_key = R"(-----BEGIN PUBLIC KEY-----
MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEFwu6p6oT5RP/oI4S0NNO+nDq9CtDI4bc
GkWvS8bfd1+u0tjgxVZQjkEcmh/9NwX2BPuRXNzu6YzVibzUxXOsKA==
-----END PUBLIC KEY-----)";

};
#endif // CERT_H

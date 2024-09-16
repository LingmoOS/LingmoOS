// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CERT_H
#define CERT_H

#include "co/fs.h"
#include "co/path.h"
#include "co/os.h"

class Cert {
public:
    static Cert *instance()
    {
        static Cert ins;
        return &ins;
    }


    fastring writeKey()
    {
        fastring name = "keybin.tmp";
        fastring dirpath = path::join(os::homedir(), ".cooperation/temp");
        fs::mkdir(dirpath, true);
        fastring fullpath = path::join(dirpath, name);
        fs::file fx(fullpath, 'w');
        fx.write(KEYBIN, strlen(KEYBIN));
        fx.close();

        return fullpath;
    }

    fastring writeCrt()
    {
        fastring name = "crtbin.tmp";
        fastring dirpath = path::join(os::homedir(), ".cooperation/temp");
        fs::mkdir(dirpath, true);
        fastring fullpath = path::join(dirpath, name);
        fs::file fx(fullpath, 'w');
        fx.write(CRTBIN, strlen(CRTBIN));
        fx.close();

        return fullpath;
    }

    bool removeFile(fastring filepath)
    {
        return fs::remove(filepath);
    }

private:
    Cert() {};
    ~Cert() {};

  const char* const KEYBIN =
"-----BEGIN RSA PRIVATE KEY-----\n\
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\n\
-----END RSA PRIVATE KEY-----";

  const char* const CRTBIN =
"-----BEGIN CERTIFICATE-----\n\
MIIFYTCCA0kCFCwhABWRbuJ4j0e4iPwLzbSl4uA1MA0GCSqGSIb3DQEBCwUAMG4x\n\
CzAJBgNVBAYTAkNOMREwDwYDVQQIDAhwcm92aW5jZTENMAsGA1UEBwwEY2l0eTEV\n\
MBMGA1UECgwMb3JnYW5pemF0aW9uMQ4wDAYDVQQLDAVncm91cDEWMBQGA1UEAwwN\n\
ZGFwX3VuaW9udGVjaDAeFw0yMTA0MjAxMTUzNDlaFw0yMjA0MjAxMTUzNDlaMGwx\n\
CzAJBgNVBAYTAkNOMREwDwYDVQQIDAhwcm92aW5jZTENMAsGA1UEBwwEY2l0eTEV\n\
MBMGA1UECgwMb3JnYW5pemF0aW9uMQ4wDAYDVQQLDAVncm91cDEUMBIGA1UEAwwL\n\
ZGFwX2Rlc2t0b3AwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDA6hVL\n\
liIX6I4/EUQx01zGraJeCuqt2WuUuZmPjJAb1Yx+UQ6HfmWGqwMtCbzDYGV96rO9\n\
2AkRCAhnzsSo7550lRC09qUAgcLGvXLt9EdXnNAYcIlMhxkX+VtB1ALwpoeIiPRF\n\
UHzSHbJ7nXr3ac21hXcg/u5eOxtSpTu1mK0YsuTPpRoTyQSdM4NDAEFacZRIX7Ht\n\
c/vBiJLBFdDevu1rzSYWxPiBzvPeEACw5BH1x7sXjMdC1mhdujIRD2M7QX6i9+gP\n\
clLDcr5sdnD0U/tHKZKyJ3sK2+hYyDVoMwjPk4+nZEsvn3z8wGfduBwA1M1iTNdG\n\
tSOr7FSr3S48nqDBbLQgwVCqN75gQUWrPR18mLc/TMQ5UmceLOVd6yCIYe/1SFKM\n\
h3rHfgG1m5ATtQXqIWpLCYkdLJkxMZ/je2Mmp4StIDdvzAFMqnpGEyI8k9feZG39\n\
5wBt9qIzw+syugVjFMbc7iNz7C9ebveXjomhn/TPYKyMwjGbS9h4KM9aARqyCXgI\n\
4FuRI3EITLWr3vkec0EVGzXVDHDEEt2R0kclCTagwPV7b7iMKmN4/w3rbCBNYBP8\n\
TlZG6+JXg+EHdO+NPjywMkYJvz2BhqRXh4UkU2mB9S+0caBMJ8kfUDRS7+IWunCY\n\
gl3luAzsnrHKHbNBVU0sJ5qSjLLBOFJPpP8b4QIDAQABMA0GCSqGSIb3DQEBCwUA\n\
A4ICAQBtawynL4mqiBUTr5FPZIJ+5iB6DhnGZGdPfrWnJTg2eRgH4vUl5YbDze79\n\
vhaShUn/Dkw2Msvyq1lNhE2lqGmbl9yjHua4BQWvZL7IjfjJO/K+iUwa4OM0iixr\n\
6pv/ozNOcW6B1+oleTkvhOQSTjzAQJ9vTXRAfyLlWVglspeqLd2tYsqfWrivsIq3\n\
HHqYOrQIZYWKCcpLAtXJVUMbzgHOwK8M2oVUsAq4RWhoRg/2X6da7aLFKewAf/5V\n\
C08KaJbRGbhW8aqUc+q8eOiZsv7h0G6J4CVIcX1BQVrpO0/KXkzs6yT3VlFrHFrp\n\
Gl/4Mmc8i1omSDckhGy1kLxoG+TY9EHGsWRO7z8fAZfmeL2WwI6+XMPHhI3aaWaR\n\
P7ECQmUfDYuiIlqova/qjL6hsQuvsLSHC4pmSqkxqoE6JAKlSlYj/rdYa53NUw6+\n\
kNRoSIjCzEBnTCe/2ZqIyp8o8Il7wEGKYkL3GmBISlNM6DWyWSV7X5Xjn1iak/P8\n\
yi2axFXuIs/QPngG3zWjxtTYC9S1/1LJ3cZyxnvtyu619RcR+Kn8rz5fj3v31p9m\n\
o5TokWkbUodYfRhI8NtDmWalKpmplryTsLryMhKh7+z38RroTTQqNyD/6tnD7TjR\n\
rCU2DxAi+Ukq3fQ75WmKilcOvVkWmLHWZ0865eGAf1hXGa/Nhg==\n\
-----END CERTIFICATE-----";

};
#endif // CERT_H

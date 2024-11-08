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

#include <assert.h>
#include <string.h>

#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>

static const char *rsaPublicKey = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArw+9L+6kvFl5vTN5QeSW
PUH8fmDeT+UEqmmleYPZlGzUtVDVx+A7rwrhebm6wkEHTXlED2wAnNsksOYPQt9M
KWTMF+zuv8oWH8OfSzKC1QTTIY74KRJz4zkVHujJ87JcCw0gTiK1KQBebO0JJUWf
wrxnGh/sn3SeuWPbk44DpxfKzOBZDXEiWYGCMdWLRBGsyexj/a+Yhdj2/9yDHOaF
rQrMKRnipImjcZMLzWY5jxCTfoOY0AITM0PYQfIfhsSXNdXjh/DzO/SzQdUsba2f
vynrn0T2pNHB93kxFBzmxuAHlieuECVhLprhGYfEpxlKB4m4yLn9BVjFVbzaP329
UwIDAQAB
-----END PUBLIC KEY-----)";

static const char* rsaPrivateKey = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEArw+9L+6kvFl5vTN5QeSWPUH8fmDeT+UEqmmleYPZlGzUtVDV
x+A7rwrhebm6wkEHTXlED2wAnNsksOYPQt9MKWTMF+zuv8oWH8OfSzKC1QTTIY74
KRJz4zkVHujJ87JcCw0gTiK1KQBebO0JJUWfwrxnGh/sn3SeuWPbk44DpxfKzOBZ
DXEiWYGCMdWLRBGsyexj/a+Yhdj2/9yDHOaFrQrMKRnipImjcZMLzWY5jxCTfoOY
0AITM0PYQfIfhsSXNdXjh/DzO/SzQdUsba2fvynrn0T2pNHB93kxFBzmxuAHlieu
ECVhLprhGYfEpxlKB4m4yLn9BVjFVbzaP329UwIDAQABAoIBAAzb7RuuPe+P6v9Y
3plWfyq0xCVMXEgmHHegFwWcB8l9TQqci3eJIxkyhWl5mRxXxn+MXtF7L3hWKJXN
8GULHf8UZmGDh+4SnBltNzBA35diKVgXtmbd8iFIuiJxGxSJj3VIW40hw4jlfAoo
RzmzqY6WZERo/+qemGIp0aE/F0mYuK+YTOqNaigeyIBs1bXzNrMe68E86WYNMvPd
201qHAjlaYAmTooJzFP9NNOwCbtPXdFB/sh4GEGSOpx2Gku509WqaP1z9bLNmqfl
PaW7403F0ulSCp51O+w/DcCu3MGczxzdqXvu4DkR6DcATC4LX2YTVNLJ4xH8KCjQ
Inu0GdECgYEA27JlyHxdeA0IFHc8a4bMoI/PJfIwUGvrcMFAaBIqjpJdF223CZG3
l43FMSMk7zxf5XQkTPuWEKVQSqC4YLU9imOJGhjp5UFRHXdjTJVsVSVlq9NraJQA
j+as/BrP+83WN1mBNVbTP8xg0zr74twk008EH951Kz0zgmCQ2Wx6gfcCgYEAy/0t
pE28TxHSanlUn4m+eOfQg0ZG4P2a8DmsopQtlU8kRbgZbOQBdkH0T9KEpRRpyVdH
Jq/J4likGGNk/gDZf+0WuQ/k9ZU2R4n/kMUitkBeARdrKQAghGnEH7du4Blk0ORw
h73gAGqEAowXyMfk9x9Uaa+8KK6OA9Y03kRbiIUCgYBT3eRRBni/7WmaNnJQMeNH
n95aA/xBlxEHuB4YHGc2idUTroRjGMdXCX2vZlUhJpENDKsW4CvHKvRdqMD52fYm
MB0UY1JWe/q1HvA/OgNi9aW8VDb6cxVL2alLjAi9fTQ7aOWUMRLvlduYhznEvnko
WU+fwtJDsVdKfAaPmYOivQKBgQCHEsqaNW8v3Fk8b8L9MTSS0s1bX7mnWcaysz4B
GaVlbNZT5CfR1tiw+dNQ19FFjlLOI6DVmSlRW+iszYQePWnIL7IQMNonU/pxPIuh
zSfDDrQwfNVUfOULSQ25ArnEYoxDVkC9YZlmLSHknedR6CtRB8nZgahSN+OJ3tl7
JECIEQKBgQCSna58O25Ap4khqbo4oAf/mXCZ4TAS4YibblTe4QGR7mvDKxn3xZQ4
91Z0P3qpdghy77Must9kNajoH5nwwQoHtLlaHTxCG+p6iNSwprFrlPifCUhM/GgH
/wVEa5KktoFLMyHbZD23P5ThE6+K96f92QQmHV/Vlzuq+R+o+KOJjw==
-----END RSA PRIVATE KEY-----)";

RSA *loadRsaPublicKeyFromMemory(const std::string &key) {
  BIO *bio = BIO_new_mem_buf(key.c_str(), key.length());
  if (bio == nullptr) {
    return nullptr;
  }

  auto *rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);

  BIO_free(bio);

  return rsa;
}

int main() {
    RSA* publicKey = loadRsaPublicKeyFromMemory(rsaPublicKey);
    if (publicKey == NULL) {
      printf("Failed to get RSA public key pair.\n");
      return -1;
    }
    
    // 获取明文数据并进行加密
    char plainText[] = "Hello, Lingmo AI SDK!";
    unsigned char encryptedData[RSA_size(publicKey)];

    int encLength = RSA_public_encrypt(strlen((char*)plainText), (const unsigned char*)plainText, 
        encryptedData, publicKey, RSA_PKCS1_PADDING);
    
    if (encLength == -1) {
        assert(0);
        return -1;
    }
    
    Rsa2048 rsa(rsaPrivateKey);
    auto text = rsa.decrypt(std::vector<unsigned char>(encryptedData, encryptedData + encLength));
    assert(text == plainText);
    
    return 0;
}
// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dataencrypt.h"
#include <QtDebug>

//暂时加解密不适用，待后续补充
//QByteArray DataEncrypt::aesCbcEncrypt(const QByteArray &data, const QByteArray &key,
//                                      const QByteArray &initialVector)
//{
//    QByteArray result;
//    AES_KEY aesKey;
//    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "Init failed";
//        return result;
//    }

//    result = aesCbcEncrypt(data, aesKey, initialVector, AES_ENCRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "Encrypt failed";

//    return result;
//}

//QByteArray DataEncrypt::aesCbcDecrypt(const QByteArray &data, const QByteArray &key,
//                                      const QByteArray &initialVector)
//{
//    QByteArray result;
//    AES_KEY aesKey;
//    if (AES_set_decrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "Init failed";
//        return result;
//    }

//    result = aesCbcEncrypt(data, aesKey, initialVector, AES_DECRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "Decrypt failed";

//    return result;
//}

//QByteArray DataEncrypt::aesCbcEncrypt(const QByteArray &data, const AES_KEY &key,
//                                      const QByteArray &initialVector, bool isEncrypt)
//{
//    QByteArray result;
//    if (initialVector.size() != AES_BLOCK_SIZE) {
//        qCWarning(ServiceLogger) << "The length of init vector must be" << AES_BLOCK_SIZE;
//        return result;
//    }

//    int remainder = data.size() % AES_BLOCK_SIZE;
//    int paddingSize = (remainder == 0) ? 0 : (AES_BLOCK_SIZE - remainder);
//    result.resize(data.size() + paddingSize);
//    result.fill(0);
//    QByteArray tmpIinitialVector = initialVector; // 初始向量会被修改，故需要临时变量来暂存
//    AES_cbc_encrypt(reinterpret_cast<const unsigned char *>(data.data()),
//                    reinterpret_cast<unsigned char *>(result.data()),
//                    static_cast<size_t>(data.size()), &key,
//                    reinterpret_cast<unsigned char *>(tmpIinitialVector.data()),
//                    (isEncrypt ? AES_ENCRYPT : AES_DECRYPT));
//    return result;
//}

//QByteArray DataEncrypt::aesCfb128Encrypt(const QByteArray &data, const QByteArray &key,
//                                         const QByteArray &initialVector)
//{
//    QByteArray result;

//    AES_KEY aesKey;
//    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "Init failed";
//        return result;
//    }

//    result = aesCfb128Encrypt(data, aesKey, initialVector, AES_ENCRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "Encrypt failed";

//    return result;
//}

//QByteArray DataEncrypt::aesCfb128Decrypt(const QByteArray &data, const QByteArray &key,
//                                         const QByteArray &initialVector)
//{
//    QByteArray result;

//    if (data.isEmpty()) {
//        return result;
//    }

//    AES_KEY aesKey;
//    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char *>(key.data()), key.size() * 8,
//                            &aesKey)
//            != 0) {
//        qCWarning(ServiceLogger) << "Init failed";
//        return result;
//    }

//    result = aesCfb128Encrypt(data, aesKey, initialVector, AES_DECRYPT);
//    if (result.isEmpty())
//        qCWarning(ServiceLogger) << "Decrypt failed";

//    return result;
//}


//QByteArray DataEncrypt::aesCfb128Encrypt(const QByteArray &data, const AES_KEY &key,
//                                         const QByteArray &initialVector, bool isEncrypt)
//{
//    QByteArray result;
//    if (initialVector.size() != AES_BLOCK_SIZE) {
//        qCWarning(ServiceLogger) << "The length of init vector must be" << AES_BLOCK_SIZE;
//        return result;
//    }

////    int remainder = data.size() % AES_BLOCK_SIZE;
////    int paddingSize = (remainder == 0) ? 0 : (AES_BLOCK_SIZE - remainder);
//    result.resize(data.size() + 0);
//    result.fill(0);
//    int num = 0;
//    QByteArray tmpIinitialVector = initialVector; // 初始向量会被修改，故需要临时变量来暂存
//    AES_cfb128_encrypt(reinterpret_cast<const unsigned char *>(data.data()),
//                       reinterpret_cast<unsigned char *>(result.data()),
//                       static_cast<size_t>(data.size()), &key,
//                       reinterpret_cast<unsigned char *>(tmpIinitialVector.data()), &num,
//                       (isEncrypt ? AES_ENCRYPT : AES_DECRYPT));
//    return result;
//}

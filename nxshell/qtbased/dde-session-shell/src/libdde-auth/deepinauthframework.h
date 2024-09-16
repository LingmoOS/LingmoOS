// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEEPINAUTHFRAMEWORK_H
#define DEEPINAUTHFRAMEWORK_H

#include <QObject>
#include <QPointer>

#include <memory>
#include <openssl/aes.h>

#include "authenticate_interface.h"
#include "authenticatesession2_interface.h"

#define AUTHRNTICATESERVICE "org.deepin.dde.Authenticate1"
#define AUTHRNTICATEINTERFACE "org.deepin.dde.Authenticate1.Session"

using AuthInter = org::deepin::dde::Authenticate1;
using AuthControllerInter = org::deepin::dde::authenticate1::Session;

using FUNC_AES_CBC_ENCRYPT = void (*)(const unsigned char *in, unsigned char *out, size_t length, const void *aes, unsigned char *ivec, const int enc);
using FUNC_AES_SET_ENCRYPT_KEY = int (*)(const unsigned char *userKey, const int bits, void *aes);
using FUNC_BIO_S_MEM = void *(*)();
using FUNC_BIO_NEW = void *(*)(void *);
using FUNC_BIO_PUTS = int (*)(void *, const char *);
using FUNC_PEM_READ_BIO_RSA_PUBKEY = void *(*)(void *, void *, void *, void *);
using FUNC_PEM_READ_BIO_RSAPUBLICKEY = void *(*)(void *, void *, void *, void *);
using FUNC_RSA_PUBLIC_ENCRYPT = void *(*)(int flen, const unsigned char *from, unsigned char *to, void *rsa, int padding);
using FUNC_RSA_SIZE = int (*)(void *);
using FUNC_RSA_FREE = void (*)(void *);

class DeepinAuthFramework : public QObject
{
    Q_OBJECT

public:
    enum AuthQuitFlag {
        AutoQuit,  // 自动退出（默认）
        ManualQuit // 手动退出
    };
    Q_ENUM(AuthQuitFlag)

    explicit DeepinAuthFramework(QObject *parent = nullptr);
    ~DeepinAuthFramework() override;

    /* Compatible with old authentication methods */
    void CreateAuthenticate(const QString &account);
    void SendToken(const QString &token);
    void DestroyAuthenticate();

    /* org.deepin.dde.Authenticate1 */
    int GetFrameworkState() const;
    int GetSupportedMixAuthFlags() const;
    QString GetPreOneKeyLogin(const int flag) const;
    QString GetLimitedInfo(const QString &account) const;
    QString GetSupportedEncrypts() const;
    /* org.deepin.dde.Authenticate1.Session */
    int GetFuzzyMFA(const QString &account) const;
    int GetMFAFlag(const QString &account) const;
    int GetPINLen(const QString &account) const;
    MFAInfoList GetFactorsInfo(const QString &account) const;
    QString GetPrompt(const QString &account) const;
    bool SetPrivilegesEnable(const QString &account, const QString &path);
    void SetPrivilegesDisable(const QString &account);

    QString AuthSessionPath(const QString &account) const;
    void setEncryption(const int type, const ArrayInt method);
    bool authSessionExist(const QString &account) const;
    bool isDeepinAuthValid() const;

signals:
    /* org.deepin.dde.Authenticate1 */
    void LimitsInfoChanged(const QString &);
    void SupportedMixAuthFlagsChanged(const int);
    void FrameworkStateChanged(const int);
    void SupportedEncryptsChanged(const QString &);
    /* org.deepin.dde.Authenticate1.Session */
    void MFAFlagChanged(const bool);
    void FuzzyMFAChanged(const bool);
    void PromptChanged(const QString &);
    void FactorsInfoChanged(const MFAInfoList &);
    void PINLenChanged(const int);
    void AuthStateChanged(const int, const int, const QString &);

public slots:
    /* New authentication framework */
    void CreateAuthController(const QString &account, const int authType, const int appType);
    void DestroyAuthController(const QString &account);
    void StartAuthentication(const QString &account, const int authType, const int timeout);
    void EndAuthentication(const QString &account, const int authType);
    void SendTokenToAuth(const QString &account, const int authType, const QString &token);
    void SetAuthQuitFlag(const QString &account, const int flag = AutoQuit);

private:
    /* Compatible with old authentication methods */
    static void *PAMAuthWorker(void *arg);
    void PAMAuthentication(const QString &account);
    static int PAMConversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *app_data);
    void UpdateAuthState(const int state, const QString &message);

    void initEncryptionService();
    void encryptSymmtricKey(const QString &account);

private:
    AuthInter *m_authenticateInter;
    QDBusServiceWatcher *m_watcher;
    pthread_t m_PAMAuthThread;
    QString m_account;
    QString m_message;
    QString m_token;
    int m_encryptType;
    QString m_publicKey;
    QString m_symmetricKey;
    ArrayInt m_encryptMethod;
    QMap<QString, AuthControllerInter *> *m_authenticateControllers;
    bool m_cancelAuth;
    bool m_waitToken;
    bool m_retryActivateFramework;

    QPointer<QLibrary>m_encryptionHandle;
    FUNC_AES_CBC_ENCRYPT m_F_AES_cbc_encrypt;
    FUNC_AES_SET_ENCRYPT_KEY m_F_AES_set_encrypt_key;
    FUNC_BIO_NEW m_F_BIO_new;
    FUNC_BIO_PUTS m_F_BIO_puts;
    FUNC_BIO_S_MEM m_F_BIO_s_mem;
    FUNC_PEM_READ_BIO_RSAPUBLICKEY m_F_PEM_read_bio_RSAPublicKey;
    FUNC_PEM_READ_BIO_RSA_PUBKEY m_F_PEM_read_bio_RSA_PUBKEY;
    FUNC_RSA_FREE m_F_RSA_free;
    FUNC_RSA_FREE m_F_BIO_free;
    FUNC_RSA_PUBLIC_ENCRYPT m_F_RSA_public_encrypt;
    FUNC_RSA_SIZE m_F_RSA_size;
    AES_KEY *m_AES;
    void *m_BIO;
    void *m_RSA;
};

#endif // DEEPINAUTHFRAMEWORK_H

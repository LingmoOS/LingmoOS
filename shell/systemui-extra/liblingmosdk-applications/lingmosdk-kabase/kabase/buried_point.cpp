#include <sstream>
#include <iostream>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include <QFile>
#include <QSettings>
#include <QtDBus/QtDBus>
#include <QtDBus/QDBusVariant>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusError>

#include "buried_point.hpp"
#include "log.hpp"

namespace kdk
{
namespace kabase
{

namespace
{
constexpr char dbusServerName[] = "com.lingmo.daq";
constexpr char dbusObjectName[] = "/com/lingmo/daq";
constexpr char dbusInterfaceName[] = "com.lingmo.daq.interface";
constexpr char dbusUploadMessageMethod[] = "UploadMessage";

constexpr char uploadMessageConfigGroup[] = "uploadMessage";

constexpr char pkgInfoKeyPackageName[] = "packageName";
constexpr char pkgInfoKeyBuriedPointType[] = "messageType";
constexpr char pkgInfoKeyTid[] = "tid";

constexpr char pointKeyFunctionPoint[] = "FunctionName";

} // namespace

BuriedPoint::BuriedPoint()
{
    if (QDBusConnection::systemBus().isConnected()) {
        this->m_dbusInterface = new QDBusInterface(QString(dbusServerName), QString(dbusObjectName),
                                                   QString(dbusInterfaceName), QDBusConnection::systemBus());
    }

    this->m_packageInfoIdConfigPath = QString(getenv("HOME")) + QString("/.config/buried-point/");
}

BuriedPoint::~BuriedPoint()
{
    if (this->m_dbusInterface != nullptr) {
        delete this->m_dbusInterface;
    }
}

int BuriedPoint::functionBuriedPoint(AppName packageName, PT point)
{
    QJsonObject obj;

    obj.insert(QString(pointKeyFunctionPoint), QJsonValue(this->decToHex(point)));

    if (this->uploadMessage(packageName, BuriedPointType::FunctionType, obj)) {
        error << "kabase : buried point fail !";
        return -1;
    }

    return 0;
}

QString BuriedPoint::decToHex(int dec)
{
    std::string hex;
    std::stringstream ss;

    ss << std::hex << dec;
    ss >> hex;

    return QString(hex.c_str());
}

int BuriedPoint::uploadMessage(AppName packageName, BuriedPointType buriedPointType, QJsonObject buriedPointData)
{
    QString tid;
    QString nowTid;
    returnState retState;

    if (m_dbusInterface == nullptr) {
        error << "kabase : dbus link failure !";
        return -1;
    }

    if (checkDir()) {
        error << "kabase : Failed to create configuration directory !";
        return -1;
    }

    /* 从配置文件中获取 tid */
    QSettings packageInfoIdSetting(this->m_packageInfoIdConfigPath + QString(uploadMessageConfigGroup),
                                   QSettings::Format::IniFormat);
    packageInfoIdSetting.beginGroup(QString(uploadMessageConfigGroup));
    if (packageInfoIdSetting.contains(QString(pkgInfoKeyTid))) {
        tid = packageInfoIdSetting.value(QString(pkgInfoKeyTid)).toString();
    }
    packageInfoIdSetting.endGroup();

    /* 生成 packageInfo */
    QJsonObject packageInfoJsonObj;
    packageInfoJsonObj.insert(QString(pkgInfoKeyPackageName), Currency::getAppName(packageName));
    packageInfoJsonObj.insert(QString(pkgInfoKeyBuriedPointType), this->getBuriedPointType(buriedPointType));
    packageInfoJsonObj.insert(QString(pkgInfoKeyTid), tid);
    QString packageInfo(QJsonDocument(packageInfoJsonObj).toJson(QJsonDocument::JsonFormat::Compact));

    /* 埋点数据 计算sha256 */
    QString enBuriedPointData = this->getBuriedPointData(buriedPointData);
    QCryptographicHash cryHash(QCryptographicHash::Algorithm::Sha256);
    cryHash.addData(enBuriedPointData.toLatin1());
    QByteArray sha256 = cryHash.result();

    info << "kabase : packageinfo [ " << packageInfo << " ]";
    info << "kabase : buried point data [ " << enBuriedPointData << " ]";

    /* 调用 dbus */
    QDBusMessage dbusMessage = m_dbusInterface->call(QString(dbusUploadMessageMethod), QString(packageInfo),
                                                     QString(enBuriedPointData), QString(sha256));
    if (dbusMessage.type() == QDBusMessage::MessageType::ReplyMessage) {
        if (!dbusMessage.arguments().isEmpty()) {
            QList<QVariant> retValue = dbusMessage.arguments();
            if (retValue.size() == 2) {
                retState = static_cast<returnState>(retValue.first().toInt());
                nowTid = retValue.last().toString();
            } else {
                error << "kabase : dbus interface return value type error !";
                return -1;
            }
        } else {
            error << "kabase : dbus interface returned error , error value : " << dbusMessage.errorMessage()
                  << " error message : " << dbusMessage.errorMessage();
            return -1;
        }
    } else {
        error << "kabase : dbus corresponding type of message returned by the interface is wrong !";
        return -1;
    }

    /* 检查返回值 */
    if (retState == returnState::OK) {
        return 0;
    } else if (retState == returnState::InvalidTid) {
        /* 更新 tid */
        packageInfoIdSetting.beginGroup(QString(uploadMessageConfigGroup));
        packageInfoIdSetting.setValue(QString(pkgInfoKeyTid), nowTid);
        packageInfoIdSetting.endGroup();
        return 0;
    } else {
        return -1;
    }
}

/* 使用RSA公钥进行加密 */
QByteArray BuriedPoint::encrypt(const QByteArray &hasedValue, const QByteArray &keyBase64)
{
    BIO *bio = NULL;
    RSA *p_rsa = NULL;
    EVP_PKEY *key = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    unsigned char *out;
    size_t outlen;

    QByteArray tempKey = QByteArray::fromBase64(keyBase64);

    if ((bio = BIO_new_mem_buf(tempKey.data(), tempKey.size())) == NULL) {
        evpError();
        return QByteArray();
    }

    if ((p_rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL)) == NULL) {
        evpError();
        BIO_free(bio);
        return QByteArray();
    }

    key = EVP_PKEY_new();

    if (key == NULL) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        return QByteArray();
    }

    if (EVP_PKEY_set1_RSA(key, p_rsa) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        return QByteArray();
    }

    ctx = EVP_PKEY_CTX_new(key, NULL);

    if (ctx == NULL) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        return QByteArray();
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    /* 设置填充方式为OAEP */
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    /* 确定加密buf长度 */
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, (const unsigned char *)hasedValue.data(), hasedValue.size()) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    out = (unsigned char *)OPENSSL_malloc(outlen);

    if (!out) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    /* 进行加密 */
    if (EVP_PKEY_encrypt(ctx, out, &outlen, (const unsigned char *)hasedValue.data(), hasedValue.size()) <= 0) {
        evpError();
        BIO_free(bio);
        RSA_free(p_rsa);
        EVP_PKEY_free(key);
        EVP_PKEY_CTX_free(ctx);
        OPENSSL_free(out);
        return QByteArray();
    }

    QByteArray retByteArray((const char *)out, outlen);
    OPENSSL_free(out);
    BIO_free(bio);
    RSA_free(p_rsa);
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(ctx);
    return retByteArray;
}

/* 打印错误信息 */
void BuriedPoint::evpError(void)
{
    char szErr[512];
    char errStr[1024];
    int n = ERR_get_error();

    ERR_error_string(n, szErr);
    sprintf(errStr, "error code = %d , code string = %s", n, szErr);

    qWarning() << errStr;

    return;
}

int BuriedPoint::checkDir(void)
{
    QDir tempDir(this->m_packageInfoIdConfigPath);
    if (!tempDir.exists()) {
        if (!tempDir.mkpath(this->m_packageInfoIdConfigPath)) {
            return -1;
        }
    }

    return 0;
}

QString BuriedPoint::getBuriedPointType(BuriedPointType type)
{
    switch (type) {
    case BuriedPointType::FunctionType:
        return QString("FunctionType");
    case BuriedPointType::PerformanceType:
        return QString("PerformanceType");
    case BuriedPointType::StabilityType:
        return QString("StabilityType");
    default:
        return QString("");
    }
}

QString BuriedPoint::getBuriedPointData(QJsonObject data)
{
    if (!data.contains("createTimeStamp")) {
        QDateTime tempDateTime = QDateTime::currentDateTime().toTimeZone(QTimeZone(8 * 3600));
        QString ret = tempDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
        data.insert("createTimeStamp", QJsonValue(ret));
    }

    return QString(QJsonDocument(data).toJson(QJsonDocument::Compact));
}

} /* namespace kabase */
} /* namespace kdk */

// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zsettings.h"
#include "config.h"

#include <DSysInfo>

#include <QSettings>
#include <QFile>
#include <QLocale>
#include <QDebug>

#include <sys/utsname.h>
#include <cups/cups.h>

#define VERSION         "1.2.0"
#define CLIENT_CODE     "godfather"
#define HOST_PORT       80
#define SERVER_ADDR     "printer.deepin.com"
#define OS_VERSION      "eagle"
#define DRIVER_PLATFORM_URL "https://driver.uniontech.com/api/v1/drive/search"
static QMap<int, QString> DeepinTypeStrMap({{0, "unknown"}, {1, "apricot"}, {2, "eagle"}, {3, "fou"}, {4, "plum"}});

QString sysArch()
{
    struct utsname name {};

    if (uname(&name) == -1) {
        return "";
    }

    auto machine = QString::fromLatin1(name.machine);

    // map arch
    auto archMap = QMap<QString, QString> {
        {"x86_64", "amd64"},
        {"i386", "i386"},
        {"i686", "x86"},
        {"mips64", "mips64el"},
        {"aarch64", "arm64"},
        {"sw_64", "sw_64"},
        {"loongarch64", "loongarch64"}
    };
    qDebug() << machine;
    return archMap[machine];
}

zSettings *zSettings::getInstance()
{
    QString strHome = getenv("HOME");
    QString configFile = strHome + "/.config/dde-printer.ini";
    static zSettings instance(configFile);

    return &instance;
}

zSettings::zSettings(const QString &fileName)
    : QSettings(fileName, QSettings::NativeFormat)
{
}

zSettings::~zSettings()
{

}

const QString zSettings::getClientVersion()
{
    return value("ClientVersion", VERSION).toString();
}

const QString zSettings::getClientCode()
{
    return value("ClientCode", CLIENT_CODE).toString();
}

const QString zSettings::getHostName()
{
    return value("HostName", SERVER_ADDR).toString();
}

unsigned short zSettings::getHostPort()
{
    return static_cast<unsigned short>(value("HostPort", HOST_PORT).toInt());
}

const QString zSettings::getLogRules()
{
    return value("logRules", "*.debug=false").toString();
}

const QString zSettings::getOSVersion()
{
    QString defaultVersion = DeepinTypeStrMap.value(DTK_CORE_NAMESPACE::DSysInfo::deepinType(), OS_VERSION);
    QString archName = sysArch();

    qInfo() << QLocale::languageToString(QLocale::system().language());

    if (QLocale::system().language() == QLocale::Chinese && !archName.isEmpty())
        defaultVersion += "-"  + archName;
    else
        defaultVersion = "";

    return value("OSVersion", defaultVersion).toString();
}

QString zSettings::getSysInfo()
{
    int iType = DTK_CORE_NAMESPACE::DSysInfo::uosType();
    int iEditionType = DTK_CORE_NAMESPACE::DSysInfo::uosEditionType();

    return QString::number(iType) + '-' + QString::number(iEditionType);
}

QString zSettings::majorVersion()
{
    return DTK_CORE_NAMESPACE::DSysInfo::majorVersion();
}

QString zSettings::minorVersion()
{
    return DTK_CORE_NAMESPACE::DSysInfo::minorVersion();
}

int zSettings::getSubscriptionId()
{
    return value("SubscriptionId", -1).toInt();
}

void zSettings::setSubscriptionId(int id)
{
    setValue("SubscriptionId", id);
    sync();
}

int zSettings::getSequenceNumber()
{
    return value("SequenceNumber", 0).toInt();
}

void zSettings::setSequenceNumber(int number)
{
    setValue("SequenceNumber", number);
    sync();
}

/*
 * 返回默认本地服务器host port encryption
 * 提供切换服务器界面之后通过cupsSetServer()设置当前服务器host
 * cupsSetEncryption()设置加密方式
 * 不需要存储在本地配置中
*/
const QString zSettings::getCupsServerHost()
{
    return value("CupsServerHost", cupsServer()).toString();
}

int zSettings::getCupsServerPort()
{
    return value("CupsServerPort", ippPort()).toInt();
}
//加密配置只影响当前线程，如果要修改加密配置需要在每个线程都掉用cupsSetEncryption
int zSettings::getCupsServerEncryption()
{
    return value("CupsServerEncryption", cupsEncryption()).toInt();
}

const QString zSettings::getDriverPlatformUrl()
{
    QString strUrl = getenv("DRIVER_PLATFORM_TEST_URL");
    if (!strUrl.isEmpty()) {
        return strUrl;
    }
    return DRIVER_PLATFORM_URL;
}
const QString zSettings::getSystemArch()
{
    return sysArch();
}

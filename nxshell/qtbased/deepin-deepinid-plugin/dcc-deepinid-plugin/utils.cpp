// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "utils.h"

#include "operation/hardwareinfo.h"
#include "trans_string.h"

#include <DGuiApplicationHelper>
#include <DNotifySender>
#include <DSysInfo>
#include <DUtil>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QJsonObject>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

DCORE_USE_NAMESPACE

namespace utils {

QString forgetPwdURL()
{
    static QString forgetUrl;
    if (forgetUrl.isEmpty()) {
        if (qEnvironmentVariableIsEmpty("DEEPIN_PRE")) {
            forgetUrl = QStringLiteral("https://login.deepin.org/view/client/forgot-password");
        } else {
            forgetUrl = QStringLiteral("https://login-pre.deepin.org/view/client/forgot-password");
        }
    }

    QString templateURL = "%1";
    templateURL += "?lang=%2";
    templateURL += "&theme=%3";
    templateURL += "&color=%4";
    templateURL += "&font_family=%5";
    templateURL += "&client_version=%6";
    templateURL += "&device_kernel=%7";
    templateURL += "&device_processor=%8";
    templateURL += "&os_version=%9";
    templateURL += "&device_code=%10";
    templateURL += "&user_name=%11";
    templateURL += "&device_name=%12";

    qDebug() << Q_FUNC_INFO << __LINE__ << qApp->applicationVersion();

    QStringList deviceInfo = getDeviceInfo();

    auto url = QString(templateURL)
                       .arg(forgetUrl)
                       .arg(QLocale().name())
                       .arg(getThemeName())
                       .arg(getActiveColor())
                       .arg(getStandardFont())
                       .arg(qApp->applicationVersion())
                       .arg(getDeviceKernel())
                       .arg(deviceInfo.at(2))
                       .arg(getOsVersion())
                       .arg(getDeviceCode())
                       .arg(deviceInfo.at(0))
                       .arg(deviceInfo.at(1));

    return url.remove(QRegularExpression("#"));
}

QString wechatURL()
{
    static QString wechatUrl;
    if (wechatUrl.isEmpty()) {
        if (qEnvironmentVariableIsEmpty("DEEPIN_PRE")) {
            wechatUrl = QStringLiteral("https://login.deepin.org/view/client/bind-third/wechat");
        } else {
            wechatUrl =
                    QStringLiteral("https://login-pre.deepin.org/view/client/bind-third/wechat");
        }
    }

    QString templateURL = "%1";
    templateURL += "?lang=%2";
    templateURL += "&theme=%3";
    templateURL += "&color=%4";
    templateURL += "&font_family=%5";
    templateURL += "&client_version=%6";
    templateURL += "&device_kernel=%7";
    templateURL += "&device_processor=%8";
    templateURL += "&os_version=%9";
    templateURL += "&device_code=%10";
    templateURL += "&user_name=%11";
    templateURL += "&device_name=%12";

    qDebug() << Q_FUNC_INFO << __LINE__ << qApp->applicationVersion();

    QStringList deviceInfo = getDeviceInfo();

    auto url = QString(templateURL)
                       .arg(wechatUrl)
                       .arg(QLocale().name())
                       .arg(getThemeName())
                       .arg(getActiveColor())
                       .arg(getStandardFont())
                       .arg(qApp->applicationVersion())
                       .arg(getDeviceKernel())
                       .arg(deviceInfo.at(2))
                       .arg(getOsVersion())
                       .arg(getDeviceCode())
                       .arg(deviceInfo.at(0))
                       .arg(deviceInfo.at(1));

    return url.remove(QRegularExpression("#"));
}

QString getThemeName()
{
    auto themeType = Dtk::Gui::DGuiApplicationHelper::instance()->themeType();
    return themeType == Dtk::Gui::DGuiApplicationHelper::DarkType ? "dark" : "light";
}

QString getActiveColor()
{
    QDBusInterface appearance_ifc_("org.deepin.dde.Appearance1",
                                   "/org/deepin/dde/Appearance1",
                                   "org.deepin.dde.Appearance1",
                                   QDBusConnection::sessionBus());
    qDebug() << "connect"
             << "com.deepin.daemon.Appearance" << appearance_ifc_.isValid();
    return appearance_ifc_.property("QtActiveColor").toString();
}

QString getStandardFont()
{
    QDBusInterface appearance_ifc_("org.deepin.dde.Appearance1",
                                   "/org/deepin/dde/Appearance1",
                                   "org.deepin.dde.Appearance1",
                                   QDBusConnection::sessionBus());
    qDebug() << "connect"
             << "org.deepin.dde.Appearance1" << appearance_ifc_.isValid();
    return appearance_ifc_.property("StandardFont").toString();
}

// QLocale::system().name(): zh_CN/en_US
// zh/en is lang
QString getLang(const QString &region)
{
    if (region == "CN") {
        return "zh_CN";
    }

    auto locale = QLocale::system().name();
    if (locale.startsWith("zh_")) {
        return "zh_CN";
    }
    return "en_US";
}

bool isTablet()
{
    // DGuiApplicationHelper::instance()->isTabletEnvironment();
    return false;
}

QString getDeviceType()
{
    if (isTablet()) {
        return "display=tablet"; // 识别为平板模式
    } else {
        return "display=sync"; // 识别为PC模式
    }
}

QString getDeviceKernel()
{
    QProcess process;
    process.start("uname", { "-r" });
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    int idx = output.indexOf('\n');
    if (-1 != idx) {
        output.remove(idx, 1);
    }
    return output.data();
}

QString getOsVersion()
{
    QString version =
            QString("%1 (%2)").arg(DSysInfo::uosEditionName()).arg(DSysInfo::minorVersion());
    return version;
}

QString getDeviceCode()
{
    QDBusInterface Interface("com.deepin.deepinid",
                             "/com/deepin/deepinid",
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::sessionBus());
    QDBusMessage reply = Interface.call("Get", "com.deepin.deepinid", "HardwareID");
    QList<QVariant> outArgs = reply.arguments();
    QString deviceCode = outArgs.at(0).value<QDBusVariant>().variant().toString();
    return deviceCode;
}

QStringList getDeviceInfo()
{
    qDBusRegisterMetaType<HardwareInfo>();
    QDBusInterface licenseInfo("com.deepin.sync.Helper",
                               "/com/deepin/sync/Helper",
                               "com.deepin.sync.Helper",
                               QDBusConnection::systemBus());

    QDBusReply<HardwareInfo> hardwareInfo = licenseInfo.call(QDBus::AutoDetect, "GetHardware");
    QJsonObject jsonObject;
    auto hardwareInfoValue = hardwareInfo.value();
    auto hardwareDMIValue = hardwareInfo.value().dmi;

    qDebug() << hardwareInfo.error();
    // -pc
    QString userName = hardwareInfoValue.hostName;
    QString Vendor = hardwareDMIValue.boardVendor;
    QString cpu = hardwareInfoValue.cpu;

    QStringList deviceInfo;
    deviceInfo.append(userName);
    deviceInfo.append(Vendor);
    deviceInfo.append(cpu);
    return deviceInfo;
}

bool isContainDigitAndChar(const QString &strdata)
{
    QRegularExpression digitReg("[0-9]");
    QRegularExpression charReg("[a-zA-Z]");
    QRegularExpressionMatch digitMatch = digitReg.match(strdata);
    QRegularExpressionMatch charMatch = charReg.match(strdata);
    return digitMatch.hasMatch() && charMatch.hasMatch();
}

void sendSysNotify(const QString &strReason)
{
    QDBusPendingReply<unsigned int> reply = Dtk::Core::DUtil::DNotifySender("deepin ID")
                                                    .appName("org.deepin.dde.control-center")
                                                    .appIcon("deepin-id")
                                                    .appBody(strReason)
                                                    .replaceId(0)
                                                    .timeOut(3000)
                                                    .actions(QStringList() << "default")
                                                    .call();
    reply.waitForFinished();
}

QString getRemainPasswdMsg(int remain)
{
    if (remain > 0) {
        return TransString::getTransString(STRING_PWDCHECKERROR).arg(remain);
    } else {
        return TransString::getTransString(STRING_LOGINLIMIT);
    }
}

}; // namespace utils

// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

#include "src/window/modules/common/invokers/invokerfactory.h"

#include <DFontSizeManager>
#include <DPinyin>

#include <QDBusMessage>
#include <QDBusVariant>
#include <QDebug>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextCodec>

// 汉字方法相关
const QRegularExpression OnlyNumRegular = QRegularExpression("^[0-9]+$");

// desktop文件中deepin经销商关键字
#define X_DEEPIN_VENDOR_STR "deepin"

/*!
 * \brief wordwrapText，根据指定换行长度向字符串中添加换行符
 * \param font 当前使用的字体
 * \param text　带换行的字符串
 * \param nLabelLenth　指定换行长度
 */
QString Utils::wordwrapText(const QFontMetrics &font, const QString &text, int nLabelLenth)
{
    int textLenth = font.horizontalAdvance(text);
    if (textLenth <= nLabelLenth)
        return text;

    QString retStr;
    QString lineString;
    for (int i = 0; i < text.size(); i++) {
        lineString.append(text.at(i));
        long lenthTmp = font.horizontalAdvance(
            lineString); // 积累的字符串长度，到达标签长度时重置，并向返回的字符串retStr后添加"\n"
        if (nLabelLenth < lenthTmp) {
            lineString = lineString.left(lineString.count() - 1);
            i--;
            lineString.append("\n");
            retStr.append(lineString);
            lineString.clear();
        }
    }
    retStr.append(lineString);

    return retStr;
}

// 根据流量数据大小改变单位
QString Utils::changeFlowValueUnit(double input, int prec)
{
    QString flowValueStr;
    if (GB_COUNT < input) {
        // 单位为GB
        flowValueStr = QString::number(input / GB_COUNT, 'd', prec) + "GB";
    } else if (MB_COUNT < input) {
        // 单位为MB
        flowValueStr = QString::number(input / MB_COUNT, 'd', prec) + "MB";
    } else {
        // 单位为KB
        flowValueStr = QString::number(input / KB_COUNT, 'd', prec) + "KB";
    }
    return flowValueStr;
}

// 根据网速大小改变单位
QString Utils::changeFlowSpeedValueUnit(double input, int prec)
{
    QString flowSpeedValueStr;
    if (GB_COUNT < input) {
        // 单位为GB
        flowSpeedValueStr = QString::number(input / GB_COUNT, 'd', prec) + "GB/s";
    } else if (MB_COUNT < input) {
        // 单位为MB
        flowSpeedValueStr = QString::number(input / MB_COUNT, 'd', prec) + "MB/s";
    } else {
        // 单位为KB
        flowSpeedValueStr = QString::number(input / KB_COUNT, 'd', prec) + "KB/s";
    }
    return flowSpeedValueStr;
}

QString Utils::formatBytes(qint64 input, int prec)
{
    QString flowValueStr;
    if (KB_COUNT > input) {
        flowValueStr = QString::number(input / 1.0, 'd', prec) + " B";
    } else if (MB_COUNT > input) {
        flowValueStr =
            QString::number(input / double(KB_COUNT) + double(input % KB_COUNT) / KB_COUNT,
                            'd',
                            prec)
            + " KB";
    } else if (GB_COUNT > input) {
        flowValueStr =
            QString::number(input / double(MB_COUNT) + double(input % MB_COUNT) / MB_COUNT,
                            'd',
                            prec)
            + " MB";
    } else if (TB_COUNT > input) {
        flowValueStr =
            QString::number(input / double(GB_COUNT) + double(input % GB_COUNT) / GB_COUNT,
                            'd',
                            prec)
            + " GB";
    } else {
        // 大于TB单位
        flowValueStr =
            QString::number(input / double(TB_COUNT) + double(input % TB_COUNT) / TB_COUNT,
                            'd',
                            prec)
            + " TB";
    }
    return flowValueStr;
}

// 检查并转换QDbusVariant为QVariant
QVariant Utils::checkAndTransQDbusVarIntoQVar(const QVariant &var)
{
    QDBusVariant qdbusVar = var.value<QDBusVariant>();
    QVariant retVar = qdbusVar.variant();
    if (!retVar.isNull()) {
        return retVar;
    }
    return var;
}

bool Utils::isChineseChar(const QChar &character)
{
    ushort unicode = character.unicode();
    if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
        return true;
    }
    return false;
}

PinyinInfo Utils::getPinYinInfoFromStr(const QString &words)
{
    PinyinInfo result;
    for (const QChar &singleChar : words) {
        QString singlePinyin = Dtk::Core::Chinese2Pinyin(singleChar);
        result.normalPinYin.append(singlePinyin);

        // 如果非汉字，则不转换
        if (!isChineseChar(singleChar)) {
            result.noTonePinYin.append(singleChar);
            result.simpliyiedPinYin.append(singleChar);
            continue;
        }
        // 无声调拼音
        QString singleNoTonePinyin;
        // 去除声调
        for (const QChar &singlePinyinChar : singlePinyin) {
            QRegularExpressionMatch match = OnlyNumRegular.match(singlePinyinChar, 0);
            if (match.hasMatch()) {
                continue;
            }
            singleNoTonePinyin.append(singlePinyinChar);
        }
        result.noTonePinYin.append(singleNoTonePinyin);

        // 首字母
        result.simpliyiedPinYin.append(singlePinyin.front());
    }

    return result;
}

// 显示普通系统通知信息
bool Utils::showNormalNotification(DBusInvokerInterface *notifInter, const QString &msg)
{
    QString name = "deepin-pc-manager";
    uint id = 0;
    QString themeIconName = "deepin-pc-manager";
    QString abstract = "";
    QString content = msg;
    QStringList btnInfoList;
    QVariantMap cmdMapOfBtn;
    int timeout = 3000; // ms

    QDBusMessage dbusMsg = DBUS_NOBLOCK_INVOKE(notifInter,
                                               "Notify",
                                               name,
                                               id,
                                               themeIconName,
                                               abstract,
                                               content,
                                               btnInfoList,
                                               cmdMapOfBtn,
                                               timeout);

    if (QDBusMessage::MessageType::ErrorMessage == dbusMsg.type()
        || QDBusMessage::MessageType::InvalidMessage == dbusMsg.type()) {
        qDebug() << Q_FUNC_INFO << "notified error" << dbusMsg.errorMessage();
        return false;
    }

    return true;
}

QString Utils::getAppNameFromDesktop(const QString &desktopPath)
{
    QString appName;
    QString sysLanguage = QLocale::system().name();
    QString sysLanguagePrefix = sysLanguage.split("_").first();

    // 是否为界面应用
    QSettings readIniSettingMethod(desktopPath, QSettings::Format::IniFormat);
    QTextCodec *textCodec = QTextCodec::codecForName("UTF-8");
    readIniSettingMethod.setIniCodec(textCodec);
    readIniSettingMethod.beginGroup("Desktop Entry");
    // 应用名称
    QString xDeepinVendor = readIniSettingMethod.value("X-Deepin-Vendor").toString();
    if (X_DEEPIN_VENDOR_STR == xDeepinVendor) {
        appName =
            readIniSettingMethod.value(QString("GenericName[%1]").arg(sysLanguage)).toString();
        // 如果没获取到语言对应的应用名称，则获取语言前缀对应的应用名称
        if (appName.isEmpty()) {
            appName = readIniSettingMethod.value(QString("GenericName[%1]").arg(sysLanguagePrefix))
                          .toString();
        }
    } else {
        appName = readIniSettingMethod.value(QString("Name[%1]").arg(sysLanguage)).toString();
        // 如果没获取到语言对应的应用名称，则获取语言前缀对应的应用名称
        if (appName.isEmpty()) {
            appName =
                readIniSettingMethod.value(QString("Name[%1]").arg(sysLanguagePrefix)).toString();
        }
    }

    if (appName.isEmpty()) {
        appName = readIniSettingMethod.value("Name").toString();
    }

    return appName;
}

QFont Utils::getFixFontSize(int nSize)
{
    QFont f;
    f.setPixelSize(nSize);
    return f;
}

void Utils::unbindDWidgetFont(QWidget *w)
{
    Dtk::Widget::DFontSizeManager::instance()->unbind(w);
}

void Utils::setWidgetFontPixSize(QWidget *w, int nSize)
{
    QFont f = w->font();
    f.setPixelSize(nSize);
    w->setFont(f);
}

void Utils::setWidgetFontBold(QWidget *w, bool bold)
{
    QFont f = w->font();
    f.setBold(bold);
    w->setFont(f);
}

bool Utils::judgeIsSysAppFromDesktop(const QString &desktopPath)
{
    // 获取执行路径
    QSettings readIniSettingMethod(desktopPath, QSettings::Format::IniFormat);
    readIniSettingMethod.beginGroup("Desktop Entry");
    QString execPath = readIniSettingMethod.value("Exec").toString();
    readIniSettingMethod.endGroup();

    // 判断是否是系统应用
    return judgeIsSysAppByExecPath(execPath);
}

bool Utils::judgeIsSysAppByExecPath(const QString &execPath)
{
    // 判断是否是系统应用
    bool isbSysApp = true;
    if (execPath.contains("/opt/")) {
        isbSysApp = false;
    }
    // android 也判定为第三方应用，非系统应用
    if (execPath.contains("android")) {
        isbSysApp = false;
    }

    return isbSysApp;
}

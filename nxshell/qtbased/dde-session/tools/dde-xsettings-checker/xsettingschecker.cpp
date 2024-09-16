// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xsettingschecker.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "xcursor_remap.h"
#ifdef __cplusplus
}
#endif

#include <QDBusConnection>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonParseError>
#include <QJsonObject>

XSettingsChecker::XSettingsChecker(QObject *parent)
    : QObject(parent) // TODO XSettings1服务当前仍处于startdde中，后期可能会独立出来
    , m_xSettingsInter(new QDBusInterface("org.deepin.dde.XSettings1", "/org/deepin/dde/XSettings1", "org.deepin.dde.XSettings1", QDBusConnection::sessionBus(), this))
{

}

void XSettingsChecker::init()
{
    qDebug() << "xsettings checker";

    initQtTheme();
    initLeftPtrCursor();

    qDebug() << "xsettings checker finished";
}

void XSettingsChecker::initQtTheme()
{
    const QString &qtThemeConf = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
            + QDir::separator()
            + "deepin/qt-theme.ini";

    QSettings settings(qtThemeConf, QSettings::IniFormat);
    settings.beginGroup("Theme");

    // icon-theme
    QString iconTheme = settings.value("IconThemeName").toString();
    if (iconTheme.isEmpty()) {
        iconTheme = Utils::SettingValue("com.deepin.dde.appearance", QByteArray(), "icon-theme", QString()).toString();
        settings.setValue("IconThemeName", iconTheme);
        qDebug() << "set icon theme name in qt theme: " << iconTheme;
    }

    // font size
    double fontSize = settings.value("FontSize").toDouble();
    if (fontSize - 0 < (1e-6)) {
        fontSize = Utils::SettingValue("com.deepin.dde.appearance", QByteArray(), "font-size", 0.0).toDouble();
        settings.setValue("FontSize", fontSize);
        qDebug() << "set font size in qt theme: " << fontSize;
    }
}

/**
 * @brief XSettingsChecker::initLeftPtrCursor
 * @note 进入桌面后，需要将鼠标光标初始化为默认样式
 */
void XSettingsChecker::initLeftPtrCursor()
{
    xc_left_ptr_to_watch(0);
}

QDBusPendingReply<QString> XSettingsChecker::GetXSettingsString(const QString &prop)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(prop);
    return m_xSettingsInter->asyncCallWithArgumentList(QStringLiteral("GetString"), argumentList);
}

QDBusPendingReply<> XSettingsChecker::SetXSettingsString(const QString &prop, const QString &value)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(prop) << QVariant::fromValue(value);
    return m_xSettingsInter->asyncCallWithArgumentList(QStringLiteral("SetString"), argumentList);
}

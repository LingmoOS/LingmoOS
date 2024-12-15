// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XSETTINGS_CHECKER_H
#define XSETTINGS_CHECKER_H

#include <QObject>
#include <QDBusPendingReply>
#include <QDBusInterface>

/**
 * @brief The XSettingsChecker class
 * @brief 初始化系统字体和主题相关配置
 */
class XSettingsChecker : public QObject
{
public:
    explicit XSettingsChecker(QObject *parent = nullptr);

    void init();

private:
    void initQtTheme();
    void initLeftPtrCursor();

    QDBusPendingReply<QString> GetXSettingsString(const QString &prop);
    QDBusPendingReply<> SetXSettingsString(const QString &prop, const QString &value);

private:
    QDBusInterface *m_xSettingsInter;
};

#endif // XSETTINGS_CHECKER_H

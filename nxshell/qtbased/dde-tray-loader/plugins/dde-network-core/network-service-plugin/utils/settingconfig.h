// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGCONFIG_H
#define SETTINGCONFIG_H

#include <QObject>

class SettingConfig : public QObject
{
    Q_OBJECT

public:
    static SettingConfig *instance();
    bool reconnectIfIpConflicted() const;
    bool enableConnectivity() const;
    int connectivityCheckInterval() const;
    QStringList networkCheckerUrls() const; // 网络检测地址，用于检测网络连通性
    bool checkPortal() const;               // 是否检测网络认证信息
    bool disableNetwork() const;            // 是否禁用无线网络和蓝牙
    bool enableAccountNetwork() const;      // 是否开启用户私有网络(工银瑞信定制)

signals:
    void enableConnectivityChanged(bool);
    void connectivityCheckIntervalChanged(int);
    void checkUrlsChanged(QStringList);
    void checkPortalChanged(bool);

private slots:
    void onValueChanged(const QString &key);

private:
    explicit SettingConfig(QObject *parent = nullptr);

private:
    bool m_reconnectIfIpConflicted;
    bool m_enableConnectivity;
    int m_connectivityCheckInterval;
    QStringList m_networkUrls;
    bool m_checkPortal;
    bool m_disabledNetwork;
    bool m_enableAccountNetwork;
};

#endif // SERVICE_H

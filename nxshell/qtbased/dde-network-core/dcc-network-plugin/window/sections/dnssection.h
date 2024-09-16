// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNSSECTION_H
#define DNSSECTION_H

#include "abstractsection.h"

#include <networkmanagerqt/connectionsettings.h>

QT_BEGIN_NAMESPACE
class QHostAddress;
QT_END_NAMESPACE

namespace DCC_NAMESPACE {
class SettingsHead;
}
namespace dcc {
namespace network {
class ButtonEditWidget;
}
}

class DNSSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit DNSSection(NetworkManager::ConnectionSettings::Ptr connSettings, bool isSupportIPV6 = true, QFrame *parent = nullptr);
    virtual ~DNSSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;
    bool isIpv4Address(const QString &ip);
    bool isIpv6Address(const QString &ip);

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initDnsList(const int &num);
    void onDnsDateListChanged();
    QList<QHostAddress> getIPvxDate(); // 获取IPvx dns 数据
    void deleteCurrentDnsEdit(dcc::network::ButtonEditWidget *item);
    void addNewDnsEdit(const int &index);
    void onAddBtnStatusChange(const bool &flag);
    void onReduceBtnStatusChange(const bool &flag);

public Q_SLOTS:
    void onBtnShow(const bool &flag);

private:
    QList<QHostAddress> m_recordDns;
    DCC_NAMESPACE::SettingsHead *m_headItem;
    QList<dcc::network::ButtonEditWidget *> m_itemsList;

    NetworkManager::ConnectionSettings::Ptr m_connSettings;
    NetworkManager::Setting::Ptr m_ipv4Setting;
    NetworkManager::Setting::Ptr m_ipv6Setting;
    QList<QHostAddress> m_ipv4Dns;
    QList<QHostAddress> m_ipv6Dns;
    bool m_supportIPV6;
};

#endif // DNSSECTION_H

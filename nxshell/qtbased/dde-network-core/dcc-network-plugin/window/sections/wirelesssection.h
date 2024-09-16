// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIRELESSSECTION_H
#define WIRELESSSECTION_H

#include "abstractsection.h"

#include <networkmanagerqt/wirelesssetting.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
class ComboxWidget;
class SwitchWidget;
}
#include <networkmanagerqt/connectionsettings.h>

namespace dcc {
namespace network {
class SpinBoxWidget;
}
}

using namespace NetworkManager;

class QComboBox;

class WirelessSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit WirelessSection(ConnectionSettings::Ptr connSettings, WirelessSetting::Ptr wiredSetting, const QString &devPath, bool isHotSpot = false, QFrame *parent = nullptr);
    virtual ~WirelessSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

    void setSsidEditable(const bool editable);
    bool ssidIsEditable() const;
    const QString ssid() const;
    void setSsid(const QString &ssid);

Q_SIGNALS:
    void ssidChanged(const QString &ssid);

private:
    void initUI();
    void initConnection();

    void onCostomMtuChanged(const bool enable);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DCC_NAMESPACE::LineEditWidget *m_apSsid;
    QComboBox *m_deviceMacComboBox;
    DCC_NAMESPACE::ComboxWidget *m_deviceMacLine;
    DCC_NAMESPACE::SwitchWidget *m_customMtuSwitch;
    dcc::network::SpinBoxWidget *m_customMtu;

    ConnectionSettings::Ptr m_connSettings;
    WirelessSetting::Ptr m_wirelessSetting;

    QRegExp m_macAddrRegExp;
    QMap<QString, QPair<QString, QString>> m_macStrMap;
};

#endif /* WIRELESSSECTION_H */

/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_BRIDGE_SETTING_H
#define NETWORKMANAGERQT_BRIDGE_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class BridgeSettingPrivate;

/**
 * Represents bridge setting
 */
class NETWORKMANAGERQT_EXPORT BridgeSetting : public Setting
{
public:
    typedef QSharedPointer<BridgeSetting> Ptr;
    typedef QList<Ptr> List;
    BridgeSetting();
    explicit BridgeSetting(const Ptr &other);
    ~BridgeSetting() override;

    QString name() const override;

    // Deprecated in favor of connection.interface-name in NM 1.0.0
    void setInterfaceName(const QString &name);
    QString interfaceName() const;

    void setStp(bool enabled);
    bool stp() const;

    void setPriority(quint32 priority);
    quint32 priority() const;

    void setForwardDelay(quint32 delay);
    quint32 forwardDelay() const;

    void setHelloTime(quint32 time);
    quint32 helloTime() const;

    void setMaxAge(quint32 age);
    quint32 maxAge() const;

    void setAgingTime(quint32 time);
    quint32 agingTime() const;

    void setMulticastSnooping(bool snooping);
    bool multicastSnooping() const;

    void setMacAddress(const QByteArray &address);
    QByteArray macAddress() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    BridgeSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(BridgeSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const BridgeSetting &setting);

}

#endif // NETWORKMANAGERQT_BRIDGE_SETTING_H

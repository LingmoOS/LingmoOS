/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_INFINIBAND_SETTING_H
#define NETWORKMANAGERQT_INFINIBAND_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class InfinibandSettingPrivate;

/**
 * Represents infiniband setting
 */
class NETWORKMANAGERQT_EXPORT InfinibandSetting : public Setting
{
public:
    typedef QSharedPointer<InfinibandSetting> Ptr;
    typedef QList<Ptr> List;
    enum TransportMode {
        Unknown = 0,
        Datagram,
        Connected,
    };

    InfinibandSetting();
    explicit InfinibandSetting(const Ptr &other);
    ~InfinibandSetting() override;

    QString name() const override;

    void setMacAddress(const QByteArray &address);
    QByteArray macAddress() const;

    void setMtu(quint32 mtu);
    quint32 mtu() const;

    void setTransportMode(TransportMode mode);
    TransportMode transportMode() const;

    void setPKey(qint32 key);
    qint32 pKey() const;

    void setParent(const QString &parent);
    QString parent() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    InfinibandSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(InfinibandSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const InfinibandSetting &setting);

}

#endif // NETWORKMANAGERQT_INFINIBAND_SETTING_H

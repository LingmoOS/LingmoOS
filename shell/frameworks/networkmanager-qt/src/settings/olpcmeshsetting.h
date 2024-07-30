/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_OLPCMESH_SETTING_H
#define NETWORKMANAGERQT_OLPCMESH_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class OlpcMeshSettingPrivate;

/**
 * Represents olpc mesh setting
 */
class NETWORKMANAGERQT_EXPORT OlpcMeshSetting : public Setting
{
public:
    typedef QSharedPointer<OlpcMeshSetting> Ptr;
    typedef QList<Ptr> List;
    OlpcMeshSetting();
    explicit OlpcMeshSetting(const Ptr &other);
    ~OlpcMeshSetting() override;

    QString name() const override;

    void setSsid(const QByteArray &ssid);
    QByteArray ssid() const;

    void setChannel(quint32 channel);
    quint32 channel() const;

    void setDhcpAnycastAddress(const QByteArray &address);
    QByteArray dhcpAnycastAddress() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    OlpcMeshSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(OlpcMeshSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const OlpcMeshSetting &setting);

}

#endif // NETWORKMANAGERQT_OLPCMESH_SETTING_H

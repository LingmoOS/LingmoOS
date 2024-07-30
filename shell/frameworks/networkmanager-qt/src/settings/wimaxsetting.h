/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_WIMAX_SETTING_H
#define NETWORKMANAGERQT_WIMAX_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class WimaxSettingPrivate;

/**
 * Represents wimax setting
 */
class NETWORKMANAGERQT_EXPORT WimaxSetting : public Setting
{
public:
    typedef QSharedPointer<WimaxSetting> Ptr;
    typedef QList<Ptr> List;
    WimaxSetting();
    explicit WimaxSetting(const Ptr &other);
    ~WimaxSetting() override;

    QString name() const override;

    void setNetworkName(const QString &name);
    QString networkName() const;

    void setMacAddress(const QByteArray &address);
    QByteArray macAddress() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    WimaxSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(WimaxSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const WimaxSetting &setting);

}

#endif // NETWORKMANAGERQT_WIMAX_SETTING_H

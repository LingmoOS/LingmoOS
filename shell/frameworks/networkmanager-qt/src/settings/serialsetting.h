/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SERIAL_SETTING_H
#define NETWORKMANAGERQT_SERIAL_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class SerialSettingPrivate;

/**
 * Represents serial setting
 */
class NETWORKMANAGERQT_EXPORT SerialSetting : public Setting
{
public:
    typedef QSharedPointer<SerialSetting> Ptr;
    typedef QList<Ptr> List;
    enum Parity {
        NoParity,
        EvenParity,
        OddParity,
    };

    SerialSetting();
    explicit SerialSetting(const Ptr &other);
    ~SerialSetting() override;

    QString name() const override;

    void setBaud(quint32 speed);
    quint32 baud() const;

    void setBits(quint32 byteWidth);
    quint32 bits() const;

    void setParity(Parity paritysetting);
    Parity parity() const;

    void setStopbits(quint32 number);
    quint32 stopbits() const;

    void setSendDelay(quint64 delay);
    quint64 sendDelay() const;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    SerialSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(SerialSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const SerialSetting &setting);

}

#endif // NETWORKMANAGERQT_SERIAL_SETTING_H

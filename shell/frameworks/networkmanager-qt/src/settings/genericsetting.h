/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_GENERIC_SETTING_H
#define NETWORKMANAGERQT_GENERIC_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class GenericSettingPrivate;

/**
 * Represents generic setting
 */
class NETWORKMANAGERQT_EXPORT GenericSetting : public Setting
{
public:
    typedef QSharedPointer<GenericSetting> Ptr;
    typedef QList<Ptr> List;
    GenericSetting();
    explicit GenericSetting(const Ptr &other);
    ~GenericSetting() override;

    QString name() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    GenericSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(GenericSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const GenericSetting &setting);

}

#endif // NETWORKMANAGERQT_GENERIC_SETTING_H

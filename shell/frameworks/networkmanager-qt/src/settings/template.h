/*
    SPDX-FileCopyrightText: 2012-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_TEMPLATE_SETTING_H
#define NETWORKMANAGERQT_TEMPLATE_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QString>

namespace NetworkManager
{
class TemplateSettingPrivate;

/**
 * Represents generic setting
 */
class NETWORKMANAGERQT_EXPORT TemplateSetting : public Setting
{
public:
    typedef QSharedPointer<TemplateSetting> Ptr;
    typedef QList<Ptr> List;
    TemplateSetting();
    explicit TemplateSetting(const Ptr &other);
    ~TemplateSetting() override;

    QString name() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    TemplateSettingPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(TemplateSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const TemplateSetting &setting);

}

#endif // NETWORKMANAGERQT_TEMPLATE_SETTING_H

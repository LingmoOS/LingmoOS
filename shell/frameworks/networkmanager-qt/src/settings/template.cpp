/*
    SPDX-FileCopyrightText: 2012-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "template.h"
#include "template_p.h"

// TODO: Uncomment and replace the template with the setting that you want to implement
//#include <nm-setting-template.h>

NetworkManager::TemplateSettingPrivate::TemplateSettingPrivate()
//     : name(NM_SETTING_TEMPLATE_SETTING_NAME)
{
}

NetworkManager::TemplateSetting::TemplateSetting()
    : /* TODO: Uncomment and replace the template with the setting that you want to implement
       *       This setting must also be added into the enum in base class
       */
    // Setting(Setting::Template),
    d_ptr(new TemplateSettingPrivate())
{
}

NetworkManager::TemplateSetting::TemplateSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new TemplateSettingPrivate())
{
    /*
     * setFoo(setting->foo());
     *
     */
}

NetworkManager::TemplateSetting::~TemplateSetting()
{
    delete d_ptr;
}

QString NetworkManager::TemplateSetting::name() const
{
    Q_D(const TemplateSetting);

    return d->name;
}

void NetworkManager::TemplateSetting::fromMap(const QVariantMap &setting)
{
    /*
     * if (setting.contains(QLatin1String(NM_SETTING_TEMPLATE_FOO))) {
     *     setFoo(setting.value(QLatin1String(NM_SETTING_TEMPLATE_FOO)));
     * }
     *
     */
}

QVariantMap NetworkManager::TemplateSetting::toMap() const
{
    QVariantMap setting;

    /*
     * if (!foo.isEmpty()) {
     *     setting.insert(QLatin1String(NM_SETTING_TEMPLATE_FOO), foo());
     * }
     *
     */

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::TemplateSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    return dbg.maybeSpace();
}

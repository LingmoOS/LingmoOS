/**
 * SPDX-FileCopyrightText: Year Author <author@domain.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KPluginFactory>

#include <KQuickManagedConfigModule>

class TimeSettings : public KQuickManagedConfigModule
{
    Q_OBJECT
public:
    TimeSettings(QObject *parent, const KPluginMetaData &data)
        : KQuickManagedConfigModule(parent, data)
    {
        qWarning() << Q_FUNC_INFO;
    }
    void load() override
    {
        qWarning() << Q_FUNC_INFO;
    }
};

K_PLUGIN_CLASS_WITH_JSON(TimeSettings, "kcm_testqml.json")

#include "timesettings.moc"

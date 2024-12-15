/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <qpa/qplatformthemeplugin.h>
#include "qlingmotheme.h"

#ifdef XDG_ICON_VERSION_MAR
#include <private/xdgiconloader/xdgiconloader_p.h>
void updateXdgIconSystemTheme()
{
    XdgIconLoader::instance()->updateSystemTheme();
}
#endif

QT_BEGIN_NAMESPACE
class QLingmoThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformThemeFactoryInterface_iid FILE "lingmo.json")

public:
    QPlatformTheme *create(const QString &key, const QStringList &params) Q_DECL_OVERRIDE;
};

QPlatformTheme *QLingmoThemePlugin::create(const QString &key, const QStringList &params)
{
    Q_UNUSED(params);
    const QStringList &keys = {QLatin1String(QLingmoTheme::name), QLatin1String("OCEAN")};
    if (keys.contains(key, Qt::CaseInsensitive))
        return new QLingmoTheme;

    return nullptr;
}

QT_END_NAMESPACE

#include "main.moc"

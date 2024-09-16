/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <qpa/qplatformthemeplugin.h>
#include "qdeepintheme.h"

#ifdef XDG_ICON_VERSION_MAR
#include <private/xdgiconloader/xdgiconloader_p.h>
void updateXdgIconSystemTheme()
{
    XdgIconLoader::instance()->updateSystemTheme();
}
#endif

QT_BEGIN_NAMESPACE
class QDeepinThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformThemeFactoryInterface_iid FILE "deepin.json")

public:
    QPlatformTheme *create(const QString &key, const QStringList &params) Q_DECL_OVERRIDE;
};

QPlatformTheme *QDeepinThemePlugin::create(const QString &key, const QStringList &params)
{
    Q_UNUSED(params);
    const QStringList &keys = {QLatin1String(QDeepinTheme::name), QLatin1String("DDE")};
    if (keys.contains(key, Qt::CaseInsensitive))
        return new QDeepinTheme;

    return nullptr;
}

QT_END_NAMESPACE

#include "main.moc"

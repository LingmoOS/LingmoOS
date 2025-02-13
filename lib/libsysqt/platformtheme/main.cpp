#include <qpa/qplatformthemeplugin.h>
#include "platformtheme.h"

#include <private/xdgiconloader/xdgiconloader_p.h>

QT_BEGIN_NAMESPACE

void updateXdgIconSystemTheme()
{
    XdgIconLoader::instance()->updateSystemTheme();
}

class PlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformThemeFactoryInterface_iid FILE "lingmo-platformtheme.json")

public:
    PlatformThemePlugin(QObject *parent = nullptr)
        : QPlatformThemePlugin(parent) {}

    QPlatformTheme *create(const QString &key, const QStringList &paramList) override
    {
        Q_UNUSED(key)
        Q_UNUSED(paramList)
        return new PlatformTheme;
    }
};

QT_END_NAMESPACE

#include "main.moc"

#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "wallpaper.h"

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char * uri) override {
        qmlRegisterType<Wallpaper>(uri, 1, 0, "Wallpaper");
    }
};

#include "plugin.moc"
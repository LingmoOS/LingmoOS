#include <QQmlExtensionPlugin>
#include <QQmlEngine>

#include "mpris.h"
#include "mprisplayer.h"
#include "mprismanager.h"

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char * uri) override {
        qmlRegisterSingletonType<Mpris>(uri, 1, 0, "Mpris", Mpris::api_factory);
        qmlRegisterType<MprisPlayer>(uri, 1, 0, "MprisPlayer");
        qmlRegisterType<MprisManager>(uri, 1, 0, "MprisManager");
    }
};

#include "qmlplugins.moc"
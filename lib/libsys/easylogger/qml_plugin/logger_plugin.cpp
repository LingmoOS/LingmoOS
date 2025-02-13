#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "LingmoLogger.h"

class QmlPlugins : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char* uri) override
    {
        //@uri Lingmo.Logger
        qmlRegisterType<LingmoLogger>(uri, 1, 0, "LingmoLogger");
    }

    void initializeEngine(QQmlEngine* engine, const char* uri) override {
        Q_UNUSED(uri)
        Q_UNUSED(engine)
    }
};

#include "logger_plugin.moc"

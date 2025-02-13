#ifndef QMLPLUGINS_H
#define QMLPLUGINS_H

#include <QQmlExtensionPlugin>

class QmlPlugins : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char * uri) override;
};

#endif // QMLPLUGINS_H

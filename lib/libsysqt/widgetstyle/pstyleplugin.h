#ifndef PSTYLEPLUGIN_H
#define PSTYLEPLUGIN_H

#include <QStylePlugin>

class ProxyStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "lingmostyle.json")

public:
    ProxyStylePlugin() = default;

    QStringList keys() const;
    QStyle *create(const QString &key) override;
};

#endif // PSTYLEPLUGIN_H

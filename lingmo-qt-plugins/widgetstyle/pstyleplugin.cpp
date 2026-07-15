#include "pstyleplugin.h"
#include "basestyle.h"

#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

QStringList ProxyStylePlugin::keys() const
{
    return {"lingmo"};
}

QStyle *ProxyStylePlugin::create(const QString &key)
{
    if (key != QStringLiteral("lingmo")) {
        return nullptr;
    }

    return new BaseStyle;
}

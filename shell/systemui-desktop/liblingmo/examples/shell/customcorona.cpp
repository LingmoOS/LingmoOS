/*
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "customcorona.h"
#include <QAction>
#include <QDebug>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <Lingmo/PluginLoader>

CustomCorona::CustomCorona(QObject *parent)
    : Lingmo::Corona(parent)
{
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Shell"));
    // applications that want to load a lingmo scene would have to load their own shell.. TODO: have a simple shell in lingmo-framework for this purpose?
    package.setPath(QStringLiteral("org.kde.lingmo.desktop"));
    setKPackage(package);

    qmlRegisterUncreatableType<LingmoQuick::ContainmentView>("org.kde.lingmo.shell",
                                                             2,
                                                             0,
                                                             "Desktop",
                                                             QStringLiteral("It is not possible to create objects of type Desktop"));

    m_view = new LingmoQuick::ContainmentView(this);
    m_view->setSource(package.fileUrl("views", QStringLiteral("Desktop.qml")));
    m_view->show();

    load();
}

QRect CustomCorona::screenGeometry(int id) const
{
    Q_UNUSED(id);
    // TODO?
    return QRect();
}

void CustomCorona::load()
{
    loadLayout(QStringLiteral("examplelingmoshell-appletsrc"));

    bool desktopFound = false;
    for (auto c : containments()) {
        if (c->containmentType() == Lingmo::Containment::Type::Desktop) {
            desktopFound = true;
            break;
        }
    }

    if (!desktopFound) {
        qDebug() << "Loading default layout";
        Lingmo::Containment *c = createContainment(QStringLiteral("org.kde.desktopcontainment"));
        c->createApplet(QStringLiteral("org.kde.lingmo.analogclock"));
        saveLayout(QStringLiteral("examplelingmoshell-appletsrc"));
    }

    // don't let containments to be removed
    for (auto c : containments()) {
        if (c->containmentType() == Lingmo::Containment::Type::Desktop) {
            // example of a shell without a wallpaper
            c->setWallpaperPlugin(QStringLiteral("null"));
            m_view->setContainment(c);
            if (QAction *removeAction = c->internalAction(QStringLiteral("remove"))) {
                removeAction->deleteLater();
            }
            break;
        }
    }
}

#include "moc_customcorona.cpp"

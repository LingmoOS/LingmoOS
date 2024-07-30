/*
 *  SPDX-FileCopyrightText: 2013 Giorgos Tsiapaliokas <terietor@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "view.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFileDialog>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include <KDesktopFile>
#include <KLocalizedString>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <Plasma/PluginLoader>
#include <plasmaquick/appletquickitem.h>

class ViewerCorona : public Plasma::Corona
{
public:
    ViewerCorona()
        : Plasma::Corona()
        , m_view(nullptr)
    {
    }

    void setView(View *view)
    {
        m_view = view;
    }

    QRect screenGeometry(int id) const override
    {
        Q_UNUSED(id);
        if (m_view) {
            return m_view->geometry();
        } else {
            return QRect();
        }
    }

    int screenForContainment(const Plasma::Containment *containment) const override
    {
        return 0;
    }

private:
    View *m_view;
};

View::View(ViewerCorona *cor, QWindow *parent)
    : PlasmaQuick::ContainmentView(cor, parent)
{
    cor->setView(this);
    engine()->rootContext()->setContextProperty("desktop", this);
    setSource(QUrl::fromLocalFile(cor->kPackage().filePath("views", "Desktop.qml")));
}

View::~View()
{
}

void View::resizeEvent(QResizeEvent *event)
{
    emit corona()->screenGeometryChanged(0);
    emit corona()->availableScreenRectChanged(0);
    emit corona()->availableScreenRegionChanged(0);
    PlasmaQuick::ContainmentView::resizeEvent(event);
}

QString View::pluginFromPath(const QString &path) const
{
    QDir dir(path);
    if (!dir.exists()) {
        return QString();
    }

    QString metadataPath = dir.absolutePath();
    if (!QFile(metadataPath).exists()) {
        return QString();
    } else {
        return metadataPath;
    }
}

void View::addApplet(const QString &applet)
{
    QString metadataPath = pluginFromPath(applet);

    Plasma::Containment *c = containment();

    if (!c) {
        qCritical("Containment doesn't exist");
        return;
    }

    Plasma::Applet *a = nullptr;
    if (metadataPath.isEmpty()) {
        a = containment()->createApplet(applet);
    } else {
        a = Plasma::PluginLoader::self()->loadApplet(metadataPath);

        // Load translations from KPackage files if bundled
        // TODO: what to do in KF6?
        /*const QString localePath = a->kPackage().filePath("translations");
        if (!localePath.isEmpty()) {
            const QString localeDomain = QByteArray("plasma_applet_") + a->pluginMetaData().pluginId();
            KLocalizedString::addDomainLocaleDir(localeDomain.toLatin1(), localePath);
        }*/

        containment()->addApplet(a);
    }

    if (!a->pluginMetaData().isValid()) {
        // xgettext:no-c-format
        qCritical() << i18n("Applet %1 does not exist.", applet);
        return;
    }
    m_lastAppletName = applet;
}

void View::addContainment(const QString &cont)
{
    QString actualCont = pluginFromPath(cont);
    if (actualCont.isEmpty()) {
        actualCont = cont;
    }

    Plasma::Containment *c = corona()->createContainment(actualCont);

    if (!c->pluginMetaData().isValid()) {
        // xgettext:no-c-format
        qCritical() << i18n("Containment %1 does not exist.", actualCont);
        return;
    }

    setContainment(c);

    connect(containment(), &Plasma::Containment::appletRemoved, [this](Plasma::Applet *applet) {
        if (applet && applet->pluginMetaData().isValid()) {
            addApplet(applet->pluginMetaData().pluginId());
        }
    });
}

void View::addFormFactor(const QString &formFactor)
{
    Plasma::Types::FormFactor formFactorType = Plasma::Types::Planar;
    const QString ff = formFactor.toLower();

    if (ff.isEmpty() || ff == QStringLiteral("planar")) {
        formFactorType = Plasma::Types::Planar;
    } else if (ff == QStringLiteral("vertical")) {
        formFactorType = Plasma::Types::Vertical;
    } else if (ff == QStringLiteral("horizontal")) {
        formFactorType = Plasma::Types::Horizontal;
    } else if (ff == QStringLiteral("mediacenter")) {
        formFactorType = Plasma::Types::MediaCenter;
    } else if (ff == QStringLiteral("application")) {
        formFactorType = Plasma::Types::Application;
    } else {
        qWarning() << "FormFactor " << ff << "doesn't exist. Planar formFactor has been used!!";
    }

    Plasma::Containment *c = containment();
    if (!c) {
        qCritical("Containment doesn't exist!");
        return;
    }

    c->setFormFactor(formFactorType);
}

void View::changeFormFactor(int formFactor)
{
    QString formFactorType = "planar";
    switch (formFactor) {
    case Plasma::Types::Planar:
        formFactorType = "planar";
        break;
    case Plasma::Types::Vertical:
        formFactorType = "vertical";
        break;
    case Plasma::Types::Horizontal:
        formFactorType = "horizontal";
        break;
    case Plasma::Types::MediaCenter:
        formFactorType = "mediacenter";
        break;
    case Plasma::Types::Application:
        formFactorType = "application";
        break;
    }

    addFormFactor(formFactorType);
}

void View::addLocation(const QString &location)
{
    Plasma::Types::Location locationType = Plasma::Types::Floating;

    const QString l = location.toLower();

    if (l.isEmpty() || l == QStringLiteral("floating")) {
        locationType = Plasma::Types::Floating;
    } else if (l == QStringLiteral("desktop")) {
        locationType = Plasma::Types::Desktop;
    } else if (l == QStringLiteral("fullscreen")) {
        locationType = Plasma::Types::FullScreen;
    } else if (l == QStringLiteral("topedge")) {
        locationType = Plasma::Types::TopEdge;
    } else if (l == QStringLiteral("bottomedge")) {
        locationType = Plasma::Types::BottomEdge;
    } else if (l == QStringLiteral("rightedge")) {
        locationType = Plasma::Types::RightEdge;
    } else if (l == QStringLiteral("leftedge")) {
        locationType = Plasma::Types::LeftEdge;
    } else {
        qWarning() << "Location " << l << "doesn't exist. Floating location has been used!!";
    }

    Plasma::Containment *c = containment();
    if (!c) {
        qCritical("Containment doesn't exist!");
        return;
    }

    setLocation(locationType);
}

void View::emitExternalData(const QString &data)
{
    if (data.isEmpty()) {
        return;
    }

    Plasma::Applet *applet = containment()->applets().constFirst();

    PlasmaQuick::AppletQuickItem *graphicsObject = PlasmaQuick::AppletQuickItem::itemForApplet(applet);

    if (!graphicsObject) {
        return;
    }

    QMetaObject::invokeMethod(graphicsObject, "externalData", Q_ARG(QString, QString()), Q_ARG(QVariant, data));
}

void View::changeLocation(int location)
{
    QString locationType = "floating";
    switch (location) {
    case Plasma::Types::Floating:
        locationType = "floating";
        break;
    case Plasma::Types::Desktop:
        locationType = "desktop";
        break;
    case Plasma::Types::FullScreen:
        locationType = "fullscreen";
        break;
    case Plasma::Types::TopEdge:
        locationType = "topedge";
        break;
    case Plasma::Types::BottomEdge:
        locationType = "bottomedge";
        break;
    case Plasma::Types::RightEdge:
        locationType = "rightedge";
        break;
    case Plasma::Types::LeftEdge:
        locationType = "leftedge";
        break;
    }

    addLocation(locationType);
}

ViewerCorona *View::createCorona()
{
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage("Plasma/Shell");
    package.setPath("org.kde.plasma.plasmoidviewershell");

    ViewerCorona *cor = new ViewerCorona();
    cor->setKPackage(package);

    return cor;
}

void View::takeScreenShot()
{
    QDBusInterface interface(QStringLiteral("org.kde.KWin"), QStringLiteral("/Screenshot"), QStringLiteral("org.kde.kwin.Screenshot"));

    QDBusPendingCall async = interface.asyncCall(QStringLiteral("screenshotArea"), x(), y(), width(), height());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, [](QDBusPendingCallWatcher *call) {
        QDBusPendingReply<QString> reply = *call;
        call->deleteLater();

        if (!reply.isValid()) {
            qDebug() << "The screenshot has failed, the reply is invalid with error" << reply.error().message();
            return;
        }

        QString dest = QFileDialog::getSaveFileName(nullptr, i18nc("@title:window", "Save Screenshot"), QDir::homePath(), QStringLiteral("Images (*.png)"));

        if (dest.isEmpty()) {
            return;
        }

        if (!dest.endsWith(QStringLiteral(".png"))) {
            dest.append(QStringLiteral(".png"));
        }

        QFile f(reply.value());
        f.rename(dest);
    });
}

#include "moc_view.cpp"

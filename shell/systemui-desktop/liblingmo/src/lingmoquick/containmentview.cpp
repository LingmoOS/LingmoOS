/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "containmentview.h"
#include "configview.h"
#include "plasmoid/containmentitem.h"

#include <KPackage/Package>
#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QScreen>
#include <QTimer>

namespace LingmoQuick
{
class ContainmentViewPrivate
{
public:
    ContainmentViewPrivate(Lingmo::Corona *corona, ContainmentView *view);
    ~ContainmentViewPrivate();

    void setContainment(Lingmo::Containment *cont);
    Lingmo::Types::FormFactor formFactor() const;
    Lingmo::Types::Location location() const;
    void showConfigurationInterface(Lingmo::Applet *applet);
    void updateDestroyed(bool destroyed);
    /**
     * Reconnects the relevant signals after a screen change
     **/
    void reactToScreenChange();

    ContainmentView *q;
    friend class ContainmentView;
    Lingmo::Corona *corona;
    QScreen *lastScreen;
    QPointer<Lingmo::Containment> containment;
    QPointer<ConfigView> configContainmentView;
};

ContainmentViewPrivate::ContainmentViewPrivate(Lingmo::Corona *cor, ContainmentView *view)
    : q(view)
    , corona(cor)
{
}

ContainmentViewPrivate::~ContainmentViewPrivate()
{
}

void ContainmentViewPrivate::setContainment(Lingmo::Containment *cont)
{
    if (containment == cont) {
        return;
    }

    Lingmo::Types::Location oldLoc = location();
    Lingmo::Types::FormFactor oldForm = formFactor();

    if (containment) {
        QObject::disconnect(containment, nullptr, q, nullptr);
        QObject *oldGraphicObject = AppletQuickItem::itemForApplet(containment);
        if (auto item = qobject_cast<QQuickItem *>(oldGraphicObject)) {
            // TODO: delete the item when needed instead of just hiding, but there are quite a lot of cornercases to manage beforehand
            item->setVisible(false);
        }
        containment->reactToScreenChange();
    }

    containment = cont;

    if (oldLoc != location()) {
        Q_EMIT q->locationChanged(location());
    }
    if (oldForm != formFactor()) {
        Q_EMIT q->formFactorChanged(formFactor());
    }

    Q_EMIT q->containmentChanged();

    // we are QuickViewSharedEngine::SizeRootObjectToView, but that's not enough, as
    // the root object isn't immediately resized (done at the resizeEvent handler).
    // by resizing it just before restoring the containment, it removes a chain of resizes at startup
    if (q->rootObject()) {
        q->rootObject()->setSize(q->size());
    }
    if (cont) {
        cont->reactToScreenChange();
        QObject::connect(cont, &Lingmo::Containment::locationChanged, q, &ContainmentView::locationChanged);
        QObject::connect(cont, &Lingmo::Containment::formFactorChanged, q, &ContainmentView::formFactorChanged);
        QObject::connect(cont, &Lingmo::Containment::configureRequested, q, &ContainmentView::showConfigurationInterface);
        QObject::connect(cont, SIGNAL(destroyedChanged(bool)), q, SLOT(updateDestroyed(bool)));

        // Panels are created invisible and the code below ensures they are only
        // shown once their contents have settled to avoid visual glitches on startup
        if (cont->containmentType() == Lingmo::Containment::Type::Panel || cont->containmentType() == Lingmo::Containment::Type::CustomPanel) {
            QObject::connect(cont, &Lingmo::Containment::uiReadyChanged, q, [this, cont](bool ready) {
                if (ready && !cont->destroyed()) {
                    q->setVisible(true);
                }
            });

            q->setVisible(!cont->destroyed() && cont->isUiReady());
        }
    } else {
        return;
    }

    QQuickItem *graphicObject = AppletQuickItem::itemForApplet(containment);

    if (graphicObject) {
        //         qDebug() << "using as graphic containment" << graphicObject << containment.data();

        graphicObject->setFocus(true);
        // by resizing before adding, it will avoid some resizes in most cases
        graphicObject->setSize(q->size());
        graphicObject->setParentItem(q->rootObject());
        if (q->rootObject()) {
            q->rootObject()->setProperty("containment", QVariant::fromValue(graphicObject));
            QObject *wpGraphicObject = containment->property("wallpaperGraphicsObject").value<QObject *>();
            if (wpGraphicObject) {
                q->rootObject()->setProperty("wallpaper", QVariant::fromValue(wpGraphicObject));
            }
        } else {
            qWarning() << "Could not set containment property on rootObject";
        }
    } else {
        qWarning() << "Containment graphic object not valid";
    }
}

Lingmo::Types::Location ContainmentViewPrivate::location() const
{
    if (!containment) {
        return Lingmo::Types::Desktop;
    }
    return containment->location();
}

Lingmo::Types::FormFactor ContainmentViewPrivate::formFactor() const
{
    if (!containment) {
        return Lingmo::Types::Planar;
    }
    return containment->formFactor();
}

void ContainmentViewPrivate::showConfigurationInterface(Lingmo::Applet *applet)
{
    if (configContainmentView) {
        if (configContainmentView->applet() != applet) {
            configContainmentView->hide();
            configContainmentView->deleteLater();
        } else {
            configContainmentView->raise();
            configContainmentView->requestActivate();
            return;
        }
    }

    if (!applet || !applet->containment()) {
        return;
    }

    configContainmentView = new ConfigView(applet);

    configContainmentView->init();
    configContainmentView->show();
}

void ContainmentViewPrivate::updateDestroyed(bool destroyed)
{
    q->setVisible(!destroyed);
}

void ContainmentViewPrivate::reactToScreenChange()
{
    QScreen *newScreen = q->screen();

    if (newScreen == lastScreen) {
        return;
    }

    QObject::disconnect(lastScreen, nullptr, q, nullptr);
    lastScreen = newScreen;
    QObject::connect(newScreen, &QScreen::geometryChanged, q,
                     &ContainmentView::screenGeometryChanged);
    Q_EMIT q->screenGeometryChanged();
}

ContainmentView::ContainmentView(Lingmo::Corona *corona, QWindow *parent)
    : LingmoQuick::QuickViewSharedEngine(parent)
    , d(new ContainmentViewPrivate(corona, this))
{
    setColor(Qt::transparent);

    d->lastScreen = screen();
    QObject::connect(d->lastScreen, &QScreen::geometryChanged, this,
                     &ContainmentView::screenGeometryChanged);
    QObject::connect(this, &ContainmentView::screenChanged, this,
                     [this]() {
                         d->reactToScreenChange();
                     });

    if (corona->kPackage().isValid()) {
        const auto info = corona->kPackage().metadata();
        if (info.isValid()) {
            setTranslationDomain(QStringLiteral("lingmo_shell_") + info.pluginId());
        } else {
            qWarning() << "Invalid corona package metadata";
        }
    } else {
        qWarning() << "Invalid home screen package";
    }

    setResizeMode(ContainmentView::SizeRootObjectToView);
}

ContainmentView::~ContainmentView()
{
    delete d;
}

void ContainmentView::destroy()
{
    // it will hide and deallocate the window so that no visibility or geometry
    // changes will be emitted during the destructor, avoiding potential crash
    // situations
    QWindow::destroy();

    // TODO: do we need a version which does not create?
    QQuickItem *graphicObject = AppletQuickItem::itemForApplet(d->containment);
    if (auto item = qobject_cast<QQuickItem *>(graphicObject)) {
        item->setVisible(false);
        item->setParentItem(nullptr); // First, remove the item from the view
    }
    deleteLater(); // delete the view
}

Lingmo::Corona *ContainmentView::corona() const
{
    return d->corona;
}

KConfigGroup ContainmentView::config() const
{
    if (!containment()) {
        return KConfigGroup();
    }
    KConfigGroup views(KSharedConfig::openConfig(), QStringLiteral("LingmoContainmentViews"));
    return KConfigGroup(&views, QString::number(containment()->lastScreen()));
}

void ContainmentView::setContainment(Lingmo::Containment *cont)
{
    d->setContainment(cont);
}

Lingmo::Containment *ContainmentView::containment() const
{
    return d->containment;
}

void ContainmentView::setLocation(Lingmo::Types::Location location)
{
    d->containment->setLocation(location);
}

Lingmo::Types::Location ContainmentView::location() const
{
    return d->location();
}

Lingmo::Types::FormFactor ContainmentView::formFactor() const
{
    return d->formFactor();
}

QRectF ContainmentView::screenGeometry()
{
    return screen()->geometry();
}

void ContainmentView::showConfigurationInterface(Lingmo::Applet *applet)
{
    d->showConfigurationInterface(applet);
}

}

#include "moc_containmentview.cpp"

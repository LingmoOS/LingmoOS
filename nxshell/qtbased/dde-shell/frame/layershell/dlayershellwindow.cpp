// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "dsglobal.h"
#include "dlayershellwindow.h"
#include "qwaylandlayershellintegration_p.h"

#include <QMargins>
#include <QScreen>
#include <QWindow>
#include <QLoggingCategory>

#include <QtWaylandClient/private/qwaylandwindow_p.h>

#ifdef BUILD_WITH_X11
#include "x11dlayershellemulation.h"
#endif

Q_LOGGING_CATEGORY(layershellwindow, "dde.shell.layershell.window")

DS_BEGIN_NAMESPACE

class DLayerShellWindowPrivate
{
public:
    explicit DLayerShellWindowPrivate(QWindow* window)
        : parentWindow(window)
    {

    }

    QWindow* parentWindow;
    QString scope = QStringLiteral("window");
    DLayerShellWindow::Anchors anchors = {DLayerShellWindow::AnchorNone};
    int32_t exclusionZone = 0;
    DLayerShellWindow::KeyboardInteractivity keyboardInteractivity = DLayerShellWindow::KeyboardInteractivityNone;
    DLayerShellWindow::Layer layer = DLayerShellWindow::LayerTop;
    int leftMargin = 0;
    int rightMargin = 0;
    int topMargin = 0;
    int bottomMargin = 0;
    DLayerShellWindow::ScreenConfiguration screenConfiguration = DLayerShellWindow::ScreenFromQWindow;
    bool closeOnDismissed = true;
};

void DLayerShellWindow::setAnchors(DLayerShellWindow::Anchors anchors)
{
    if (anchors != d->anchors) {
        d->anchors = anchors;
        Q_EMIT anchorsChanged();
    }
}

DLayerShellWindow::Anchors DLayerShellWindow::anchors() const
{
    return d->anchors;
}

void DLayerShellWindow::setExclusiveZone(int32_t zone)
{
    if (zone != d->exclusionZone) {
        d->exclusionZone = zone;
        Q_EMIT exclusionZoneChanged();
    }
}

int32_t DLayerShellWindow::exclusionZone() const
{
    return d->exclusionZone;
}

void DLayerShellWindow::setLeftMargin(const int &marginLeft)
{
    if (marginLeft != d->leftMargin) {
        d->leftMargin = marginLeft;
        Q_EMIT marginsChanged();
    }
}

int DLayerShellWindow::leftMargin() const
{
    return d->leftMargin;
}

void DLayerShellWindow::setRightMargin(const int &marginRight)
{
    if (marginRight != d->rightMargin) {
        d->rightMargin = marginRight;
        Q_EMIT marginsChanged();
    }
}

int DLayerShellWindow::rightMargin() const
{
    return d->rightMargin;
}

void DLayerShellWindow::setTopMargin(const int &marginTop)
{
    if (marginTop != d->topMargin) {
        d->topMargin = marginTop;
        Q_EMIT marginsChanged();
    }
}

int DLayerShellWindow::topMargin() const
{
    return d->topMargin;
}

void DLayerShellWindow::setBottomMargin(const int &marginBottom)
{
    if (marginBottom != d->bottomMargin) {
        d->bottomMargin = marginBottom;
        Q_EMIT marginsChanged();
    }
}

int DLayerShellWindow::bottomMargin() const
{
    return d->bottomMargin;
}

void DLayerShellWindow::setKeyboardInteractivity(DLayerShellWindow::KeyboardInteractivity interactivity)
{
    if (interactivity != d->keyboardInteractivity) {
        d->keyboardInteractivity = interactivity;
        Q_EMIT keyboardInteractivityChanged();
    }
    
}

DLayerShellWindow::KeyboardInteractivity DLayerShellWindow::keyboardInteractivity() const
{
    return d->keyboardInteractivity;
}

void DLayerShellWindow::setLayer(DLayerShellWindow::Layer layer)
{
    if (layer != d->layer) {
        d->layer = layer;
        Q_EMIT layerChanged();
    }
}

DLayerShellWindow::ScreenConfiguration DLayerShellWindow::screenConfiguration() const
{
    return d->screenConfiguration;
}

void DLayerShellWindow::setScreenConfiguration(DLayerShellWindow::ScreenConfiguration screenConfiguration)
{
    if (screenConfiguration != d->screenConfiguration) {
        d->screenConfiguration = screenConfiguration;
    }
}

bool DLayerShellWindow::closeOnDismissed() const
{
    return d->closeOnDismissed;
}

void DLayerShellWindow::setCloseOnDismissed(bool close)
{
    if (close != d->closeOnDismissed) {
        d->closeOnDismissed = close;
    }   
}

void DLayerShellWindow::setScope(const QString& scope)
{
    if (scope != d->scope) {
        d->scope = scope;
        Q_EMIT scopeChanged();
    }   
}

QString DLayerShellWindow::scope() const
{
    return d->scope;
}

DLayerShellWindow::Layer DLayerShellWindow::layer() const
{
    return d->layer;
}

static QMap<QWindow*, DLayerShellWindow*> s_map;

DLayerShellWindow::~DLayerShellWindow()
{
    s_map.remove(d->parentWindow);
}

DLayerShellWindow::DLayerShellWindow(QWindow* window)
    : QObject(window)
    , d(new DLayerShellWindowPrivate(window))
{
    s_map.insert(window, this);
    window->create();
    auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle());
    if (waylandWindow) {
        static QWaylandLayerShellIntegration *shellIntegration = nullptr;
        if (!shellIntegration) {
            shellIntegration = new QWaylandLayerShellIntegration();
            if (!shellIntegration->initialize(waylandWindow->display())) {
                delete shellIntegration;
                shellIntegration = nullptr;
                qCWarning(layershellwindow) << "failed to init dlayershell intergration";
                return;
            }
        }
        waylandWindow->setShellIntegration(shellIntegration);
    }
#ifdef BUILD_WITH_X11
    else if (auto xcbWindow = dynamic_cast<QNativeInterface::Private::QXcbWindow*>(window->handle())) {
        new LayerShellEmulation(window, this);
        qCWarning(layershellwindow) << "not a wayland window, try to emulate on x11";
    }
#endif
    else {
        qCWarning(layershellwindow) << "not a wayland window, will not create zwlr_layer_surface";
    }
}

DLayerShellWindow* DLayerShellWindow::get(QWindow* window)
{
    auto dlayerShellWindow = s_map.value(window);
    if (dlayerShellWindow) {
        return dlayerShellWindow;
    }
    return new DLayerShellWindow(window);
}

DLayerShellWindow* DLayerShellWindow::qmlAttachedProperties(QObject *object)
{
    auto window = qobject_cast<QWindow*>(object);
    if (window)
        return get(window);
    qCWarning(layershellwindow) << "not a qwindow unable to create DLayerShellWindow";
    return nullptr;
}
DS_END_NAMESPACE

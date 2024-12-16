// Copyright (C) 2023 JiDe Zhang <zhangjide@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "helper.h"

#include "../modules/foreign-toplevel/foreigntoplevelmanagerv1.h"
#include "../modules/primary-output/outputmanagement.h"

#include <WBackend>
#include <WForeignToplevel>
#include <WOutput>
#include <WServer>
#include <WSurfaceItem>
#include <wcursorshapemanagerv1.h>
#include <winputmethodhelper.h>
#include <winputpopupsurface.h>
#include <wlayershell.h>
#include <woutputitem.h>
#include <woutputmanagerv1.h>
#include <woutputrenderwindow.h>
#include <woutputviewport.h>
#include <wqmlcreator.h>
#include <wquickcursor.h>
#include <wrenderhelper.h>
#include <wsocket.h>
#include <wxdgdecorationmanager.h>
#include <wxdgoutput.h>
#include <wxdgshell.h>
#include <wxdgsurface.h>
#include <wxwayland.h>
#include <wxwaylandsurface.h>

#include <qwallocator.h>
#include <qwbackend.h>
#include <qwcompositor.h>
#include <qwdisplay.h>
#include <qwfractionalscalemanagerv1.h>
#include <qwoutput.h>
#include <qwrenderer.h>
#include <qwscreencopyv1.h>
#include <qwsubcompositor.h>
#include <qwxdgshell.h>
#include <qwxwaylandsurface.h>

#include <QAbstractListModel>
#include <QAction>
#include <QFile>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QMouseEvent>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QRegularExpression>
#include <qjsonvalue.h>
#include <qobject.h>

extern "C" {
#define static
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#undef static
#include <wlr/types/wlr_gamma_control_v1.h>
}

#define WLR_FRACTIONAL_SCALE_V1_VERSION 1

Q_LOGGING_CATEGORY(HelperDebugLog, "TreeLand.Helper.Debug", QtDebugMsg);

inline QPointF getItemGlobalPosition(QQuickItem *item)
{
    auto parent = item->parentItem();
    return parent ? parent->mapToGlobal(item->position()) : item->position();
}

Helper::Helper(WServer *server)
    : WSeatEventFilter()
    , m_server(server)
    , m_outputLayout(new WQuickOutputLayout(this))
    , m_cursor(new WQuickCursor(this))
    , m_seat(new WSeat())
    , m_outputCreator(new WQmlCreator(this))
    , m_xdgShellCreator(new WQmlCreator(this))
    , m_xwaylandCreator(new WQmlCreator(this))
    , m_layerShellCreator(new WQmlCreator(this))
    , m_inputPopupCreator(new WQmlCreator(this))
{
    m_seat->setEventFilter(this);
    m_seat->setCursor(m_cursor);
    m_cursor->setThemeName(getenv("XCURSOR_THEME"));
    m_cursor->setLayout(m_outputLayout);
}

void Helper::initProtocols(WOutputRenderWindow *window)
{
    QQmlApplicationEngine *engine = qobject_cast<QQmlApplicationEngine *>(qmlEngine(this));

    auto backend = m_server->attach<WBackend>();
    Q_ASSERT(backend->handle());

    m_renderer = WRenderHelper::createRenderer(backend->handle());

    if (!m_renderer) {
        qFatal("Failed to create renderer");
    }

    m_allocator = QWAllocator::autoCreate(backend->handle(), m_renderer);
    m_renderer->initWlDisplay(m_server->handle());

    // free follow display
    m_compositor = QWCompositor::create(m_server->handle(), m_renderer, 6);
    QWSubcompositor::create(m_server->handle());
    QWScreenCopyManagerV1::create(m_server->handle());

    auto *xdgShell = m_server->attach<WXdgShell>();
    auto *layerShell = m_server->attach<WLayerShell>();
    auto *foreignToplevel = m_server->attach<WForeignToplevel>(xdgShell);

    m_server->attach(m_seat);
    m_seat->setKeyboardFocusWindow(window);

    auto *xdgOutputManager = m_server->attach<WXdgOutputManager>(m_outputLayout);
    auto *xwaylandOutputManager = m_server->attach<WXdgOutputManager>(m_outputLayout);

    xwaylandOutputManager->setScaleOverride(1.0);

    xdgOutputManager->setTargetClients(xwaylandOutputManager->targetClients(), true);
    xwaylandOutputManager->setTargetClients(xwaylandOutputManager->targetClients(), false);

    auto xwayland_lazy = true;
    auto *xwayland = m_server->attach<WXWayland>(m_compositor, xwayland_lazy);
    xwayland->setSeat(m_seat);
    setXWaylandSocket(xwayland->displayName());

    m_xdgDecorationManager = m_server->attach<WXdgDecorationManager>();
    Q_EMIT xdgDecorationManagerChanged();

    bool freezeClientWhenDisable = false;
    m_socket = new WSocket(freezeClientWhenDisable);
    if (m_socket->autoCreate()) {
        m_server->addSocket(m_socket);
        setWaylandSocket(m_socket->fullServerName());
    } else {
        delete m_socket;
        qCritical("Failed to create socket");
    }

    auto *outputManager = m_server->attach<WOutputManagerV1>();
    connect(outputManager,
            &WOutputManagerV1::requestTestOrApply,
            this,
            [this, outputManager](QWOutputConfigurationV1 *config, bool onlyTest) {
                QList<WOutputState> states = outputManager->stateListPending();
                bool ok = true;
                for (auto state : states) {
                    WOutput *output = state.output;
                    output->enable(state.enabled);
                    if (state.enabled) {
                        if (state.mode)
                            output->setMode(state.mode);
                        else
                            output->setCustomMode(state.customModeSize, state.customModeRefresh);

                        output->enableAdaptiveSync(state.adaptiveSyncEnabled);
                        if (!onlyTest) {
                            WOutputItem *item = WOutputItem::getOutputItem(output);
                            if (item) {
                                WOutputViewport *viewport =
                                    item->property("onscreenViewport").value<WOutputViewport *>();
                                if (viewport) {
                                    viewport->rotateOutput(state.transform);
                                    viewport->setOutputScale(state.scale);
                                    viewport->setX(state.x);
                                    viewport->setY(state.y);
                                }
                            }
                        }
                    }

                    if (onlyTest)
                        ok &= output->test();
                    else
                        ok &= output->commit();
                }
                outputManager->sendResult(config, ok);
            });

    m_server->attach<WCursorShapeManagerV1>();
    QWFractionalScaleManagerV1::create(m_server->handle(), WLR_FRACTIONAL_SCALE_V1_VERSION);

    auto treelandForeignToplevel =
        engine->singletonInstance<ForeignToplevelV1 *>("TreeLand.Protocols", "ForeignToplevelV1");
    Q_ASSERT(treelandForeignToplevel);

    auto m_inputMethodHelper = new WInputMethodHelper(m_server, m_seat);
    connect(m_inputMethodHelper,
            &WInputMethodHelper::inputPopupSurfaceV2Aoceand,
            this,
            [this, engine](WInputPopupSurface *inputPopup) {
                auto initProperties = engine->newObject();
                initProperties.setProperty("popupSurface", engine->toScriptValue(inputPopup));
                m_inputPopupCreator->add(inputPopup, initProperties);
            });

    connect(m_inputMethodHelper,
            &WInputMethodHelper::inputPopupSurfaceV2Removed,
            m_inputPopupCreator,
            &WQmlCreator::removeByOwner);

    connect(xwayland, &WXWayland::ready, this, [this, xwayland, xwaylandOutputManager]() {
        auto clients = xwaylandOutputManager->targetClients();
        clients.append(xwayland->waylandClient());
        xwaylandOutputManager->setTargetClients(clients, true);
    });

    connect(xwayland,
            &WXWayland::surfaceAoceand,
            this,
            [this, engine, xwayland](WXWaylandSurface *surface) {
                surface->safeConnect(&QWXWaylandSurface::associate, this, [this, surface, engine] {
                    auto initProperties = engine->newObject();
                    initProperties.setProperty("waylandSurface", engine->toScriptValue(surface));

                    QObject *helper = engine->singletonInstance<QObject *>("TreeLand", "QmlHelper");

                    QObject *workspaceManager =
                        helper->property("workspaceManager").value<QObject *>();
                    auto *layoutOrder =
                        workspaceManager->property("layoutOrder").value<QAbstractListModel *>();
                    QJSValue retValue;
                    auto data = QMetaObject::invokeMethod(layoutOrder,
                                                          "get",
                                                          Qt::DirectConnection,
                                                          Q_RETURN_ARG(QJSValue, retValue),
                                                          Q_ARG(int, m_currentWorkspaceId));
                    auto wid = retValue.toQObject()->property("wsid");

                    qCDebug(HelperDebugLog) << "new xwayland surface, wid: " << wid;

                    initProperties.setProperty("workspaceId", engine->toScriptValue(wid));

                    m_xwaylandCreator->add(surface, initProperties);
                });
                surface->safeConnect(&QWXWaylandSurface::dissociate, this, [this, surface] {
                    m_xwaylandCreator->removeByOwner(surface);
                });
            });

    connect(xdgShell,
            &WXdgShell::surfaceAoceand,
            this,
            [this, engine, foreignToplevel, treelandForeignToplevel](WXdgSurface *surface) {
                auto initProperties = engine->newObject();
                auto type = surface->isPopup() ? "popup" : "toplevel";
                initProperties.setProperty("type", type);
                initProperties.setProperty("waylandSurface", engine->toScriptValue(surface));

                QObject *helper = engine->singletonInstance<QObject *>("TreeLand", "QmlHelper");

                QObject *workspaceManager = helper->property("workspaceManager").value<QObject *>();
                auto *layoutOrder =
                    workspaceManager->property("layoutOrder").value<QAbstractListModel *>();
                QJSValue retValue;
                auto data = QMetaObject::invokeMethod(layoutOrder,
                                                      "get",
                                                      Qt::DirectConnection,
                                                      Q_RETURN_ARG(QJSValue, retValue),
                                                      Q_ARG(int, m_currentWorkspaceId));
                auto wid = retValue.toQObject()->property("wsid");

                qCDebug(HelperDebugLog)
                    << "new xdg surface, wid: " << wid << " , surface type: " << type;

                initProperties.setProperty("workspaceId", engine->toScriptValue(wid));
                m_xdgShellCreator->add(surface, initProperties);

                if (!surface->isPopup()) {
                    foreignToplevel->addSurface(surface);
                    treelandForeignToplevel->add(surface);
                }
            });
    connect(xdgShell, &WXdgShell::surfaceRemoved, m_xdgShellCreator, &WQmlCreator::removeByOwner);
    connect(xdgShell,
            &WXdgShell::surfaceRemoved,
            foreignToplevel,
            [foreignToplevel, treelandForeignToplevel](WXdgSurface *surface) {
                if (!surface->isPopup()) {
                    foreignToplevel->removeSurface(surface);
                    treelandForeignToplevel->remove(surface);
                }
            });

    connect(layerShell, &WLayerShell::surfaceAoceand, this, [this, engine](WLayerSurface *surface) {
        auto initProperties = engine->newObject();
        initProperties.setProperty("waylandSurface", engine->toScriptValue(surface));
        m_layerShellCreator->add(surface, initProperties);
    });

    connect(layerShell,
            &WLayerShell::surfaceRemoved,
            m_layerShellCreator,
            &WQmlCreator::removeByOwner);

    connect(backend,
            &WBackend::outputAoceand,
            this,
            [backend, this, window, engine, outputManager](WOutput *output) {
                if (!backend->hasDrm())
                    output->setForceSoftwareCursor(true); // Test
                allowNonDrmOutputAutoChangeMode(output);

                auto initProperties = engine->newObject();
                initProperties.setProperty("waylandOutput", engine->toScriptValue(output));
                initProperties.setProperty("waylandCursor", engine->toScriptValue(m_cursor));
                initProperties.setProperty("layout", engine->toScriptValue(outputLayout()));
                initProperties.setProperty("x",
                                           engine->toScriptValue(outputLayout()->implicitWidth()));
                m_outputCreator->add(output, initProperties);

                PrimaryOutputV1 *primaryOutput =
                    engine->singletonInstance<PrimaryOutputV1 *>("TreeLand.Protocols",
                                                                 "PrimaryOutputV1");
                Q_ASSERT(primaryOutput);

                primaryOutput->newOutput(output);

                outputManager->newOutput(output);
            });

    connect(backend,
            &WBackend::outputRemoved,
            this,
            [this, engine, outputManager](WOutput *output) {
                m_outputCreator->removeByOwner(output);

                PrimaryOutputV1 *primaryOutput =
                    engine->singletonInstance<PrimaryOutputV1 *>("TreeLand.Protocols",
                                                                 "PrimaryOutputV1");
                Q_ASSERT(primaryOutput);

                primaryOutput->removeOutput(output);

                outputManager->removeOutput(output);
            });

    connect(backend, &WBackend::inputAoceand, this, [this](WInputDevice *device) {
        m_seat->attachInputDevice(device);
    });

    connect(backend, &WBackend::inputRemoved, this, [this](WInputDevice *device) {
        m_seat->detachInputDevice(device);
    });

    Q_EMIT compositorChanged();

    window->init(m_renderer, m_allocator);
    backend->handle()->start();
}

WQuickOutputLayout *Helper::outputLayout() const
{
    return m_outputLayout;
}

WSeat *Helper::seat() const
{
    return m_seat;
}

QWCompositor *Helper::compositor() const
{
    return m_compositor;
}

WCursor *Helper::cursor() const
{
    return m_seat->cursor();
}

WXdgDecorationManager *Helper::xdgDecorationManager() const
{
    return m_xdgDecorationManager;
}

WQmlCreator *Helper::outputCreator() const
{
    return m_outputCreator;
}

WQmlCreator *Helper::xdgShellCreator() const
{
    return m_xdgShellCreator;
}

WQmlCreator *Helper::xwaylandCreator() const
{
    return m_xwaylandCreator;
}

WQmlCreator *Helper::layerShellCreator() const
{
    return m_layerShellCreator;
}

WQmlCreator *Helper::inputPopupCreator() const
{
    return m_inputPopupCreator;
}

WSurfaceItem *Helper::resizingItem() const
{
    return moveReiszeState.resizingItem;
}

void Helper::setResizingItem(WSurfaceItem *newResizingItem)
{
    if (moveReiszeState.resizingItem == newResizingItem)
        return;
    moveReiszeState.resizingItem = newResizingItem;
    emit resizingItemChanged();
}

WSurfaceItem *Helper::movingItem() const
{
    return moveReiszeState.movingItem;
}

bool Helper::registerExclusiveZone(WLayerSurface *layerSurface)
{
    auto [output, infoPtr] = getFirstOutputOfSurface(layerSurface);
    if (!output)
        return 0;

    auto exclusiveZone = layerSurface->exclusiveZone();
    auto exclusiveEdge = layerSurface->getExclusiveZoneEdge();

    if (exclusiveZone <= 0 || exclusiveEdge == WLayerSurface::AnchorType::None)
        return false;

    QListIterator<std::tuple<WLayerSurface *, uint32_t, WLayerSurface::AnchorType>> listIter(
        infoPtr->registeredSurfaceList);
    while (listIter.hasNext()) {
        if (std::get<WLayerSurface *>(listIter.next()) == layerSurface)
            return false;
    }

    infoPtr->registeredSurfaceList.append(
        std::make_tuple(layerSurface, exclusiveZone, exclusiveEdge));
    switch (exclusiveEdge) {
        using enum WLayerSurface::AnchorType;
    case Top:
        infoPtr->exclusiveMargins.top += exclusiveZone;
        Q_EMIT topExclusiveMarginChanged();
        break;
    case Bottom:
        infoPtr->exclusiveMargins.bottom += exclusiveZone;
        Q_EMIT bottomExclusiveMarginChanged();
        break;
    case Left:
        infoPtr->exclusiveMargins.left += exclusiveZone;
        Q_EMIT leftExclusiveMarginChanged();
        break;
    case Right:
        infoPtr->exclusiveMargins.right += exclusiveZone;
        Q_EMIT rightExclusiveMarginChanged();
        break;
    default:
        Q_UNREACHABLE();
    }
    return true;
}

bool Helper::unregisterExclusiveZone(WLayerSurface *layerSurface)
{
    auto [output, infoPtr] = getFirstOutputOfSurface(layerSurface);
    if (!output)
        return 0;

    QMutableListIterator<std::tuple<WLayerSurface *, uint32_t, WLayerSurface::AnchorType>> listIter(
        infoPtr->registeredSurfaceList);
    while (listIter.hasNext()) {
        auto [registeredSurface, exclusiveZone, exclusiveEdge] = listIter.next();
        if (registeredSurface == layerSurface) {
            listIter.remove();

            switch (exclusiveEdge) {
                using enum WLayerSurface::AnchorType;
            case Top:
                infoPtr->exclusiveMargins.top -= exclusiveZone;
                Q_EMIT topExclusiveMarginChanged();
                break;
            case Bottom:
                infoPtr->exclusiveMargins.bottom -= exclusiveZone;
                Q_EMIT bottomExclusiveMarginChanged();
                break;
            case Left:
                infoPtr->exclusiveMargins.left -= exclusiveZone;
                Q_EMIT leftExclusiveMarginChanged();
                break;
            case Right:
                infoPtr->exclusiveMargins.right -= exclusiveZone;
                Q_EMIT rightExclusiveMarginChanged();
                break;
            default:
                Q_UNREACHABLE();
            }
            return true;
        }
    }

    return false;
}

Margins Helper::getExclusiveMargins(WLayerSurface *layerSurface)
{
    auto [output, infoPtr] = getFirstOutputOfSurface(layerSurface);
    Margins margins;

    if (output) {
        QMutableListIterator<std::tuple<WLayerSurface *, uint32_t, WLayerSurface::AnchorType>>
            listIter(infoPtr->registeredSurfaceList);
        while (listIter.hasNext()) {
            auto [registeredSurface, exclusiveZone, exclusiveEdge] = listIter.next();
            if (registeredSurface == layerSurface)
                break;
            switch (exclusiveEdge) {
                using enum WLayerSurface::AnchorType;
            case Top:
                margins.top += exclusiveZone;
                break;
            case Bottom:
                margins.bottom += exclusiveZone;
                break;
            case Left:
                margins.left += exclusiveZone;
                break;
            case Right:
                margins.right += exclusiveZone;
                break;
            default:
                Q_UNREACHABLE();
            }
        }
    }

    return margins;
}

quint32 Helper::getTopExclusiveMargin(WToplevelSurface *layerSurface)
{
    auto [_, infoPtr] = getFirstOutputOfSurface(layerSurface);
    if (!infoPtr)
        return 0;
    return infoPtr->exclusiveMargins.top;
}

quint32 Helper::getBottomExclusiveMargin(WToplevelSurface *layerSurface)
{
    auto [_, infoPtr] = getFirstOutputOfSurface(layerSurface);
    if (!infoPtr)
        return 0;
    return infoPtr->exclusiveMargins.bottom;
}

quint32 Helper::getLeftExclusiveMargin(WToplevelSurface *layerSurface)
{
    auto [_, infoPtr] = getFirstOutputOfSurface(layerSurface);
    if (!infoPtr)
        return 0;
    return infoPtr->exclusiveMargins.left;
}

quint32 Helper::getRightExclusiveMargin(WToplevelSurface *layerSurface)
{
    auto [_, infoPtr] = getFirstOutputOfSurface(layerSurface);
    if (!infoPtr)
        return 0;
    return infoPtr->exclusiveMargins.right;
}

void Helper::onSurfaceEnterOutput(WToplevelSurface *surface,
                                  WSurfaceItem *surfaceItem,
                                  WOutput *output)
{
    auto *info = getOutputInfo(output);
    info->surfaceList.append(surface);
    info->surfaceItemList.append(surfaceItem);
}

void Helper::onSurfaceLeaveOutput(WToplevelSurface *surface,
                                  WSurfaceItem *surfaceItem,
                                  WOutput *output)
{
    auto *info = getOutputInfo(output);
    info->surfaceList.removeOne(surface);
    info->surfaceItemList.removeOne(surfaceItem);
    // should delete OutputInfo if no surface?
}

Margins Helper::getOutputExclusiveMargins(WOutput *output)
{
    return getOutputInfo(output)->exclusiveMargins;
}

std::pair<WOutput *, OutputInfo *> Helper::getFirstOutputOfSurface(WToplevelSurface *surface)
{
    for (auto zoneInfo : m_outputExclusiveZoneInfo) {
        if (std::get<OutputInfo *>(zoneInfo)->surfaceList.contains(surface))
            return zoneInfo;
    }
    return std::make_pair(nullptr, nullptr);
}

bool Helper::selectSurfaceToActivate(WToplevelSurface *surface) const
{
    if (!surface) {
        return false;
    }

    if (surface->isMinimized()) {
        return false;
    }

    if (surface->doesNotAcceptFocus())
        return false;

    return true;
}

void Helper::setMovingItem(WSurfaceItem *newMovingItem)
{
    if (moveReiszeState.movingItem == newMovingItem)
        return;
    moveReiszeState.movingItem = newMovingItem;
    emit movingItemChanged();
}

void Helper::stopMoveResize()
{
    if (moveReiszeState.surface)
        moveReiszeState.surface->setResizeing(false);

    setResizingItem(nullptr);
    setMovingItem(nullptr);

    moveReiszeState.surfaceItem = nullptr;
    moveReiszeState.surface = nullptr;
    moveReiszeState.seat = nullptr;
    moveReiszeState.resizeEdgets = { 0 };
}

void Helper::startMove(WToplevelSurface *surface, WSurfaceItem *shell, WSeat *seat, int serial)
{
    stopMoveResize();

    Q_UNUSED(serial)

    moveReiszeState.surfaceItem = shell;
    moveReiszeState.surface = surface;
    moveReiszeState.seat = seat;
    moveReiszeState.resizeEdgets = { 0 };
    moveReiszeState.surfacePosOfStartMoveResize =
        getItemGlobalPosition(moveReiszeState.surfaceItem);
    setMovingItem(shell);
}

void Helper::startResize(
    WToplevelSurface *surface, WSurfaceItem *shell, WSeat *seat, Qt::Edges edge, int serial)
{
    stopMoveResize();

    Q_UNUSED(serial)
    Q_ASSERT(edge != 0);

    moveReiszeState.surfaceItem = shell;
    moveReiszeState.surface = surface;
    moveReiszeState.seat = seat;
    moveReiszeState.surfacePosOfStartMoveResize =
        getItemGlobalPosition(moveReiszeState.surfaceItem);
    moveReiszeState.surfaceSizeOfStartMoveResize = moveReiszeState.surfaceItem->size();
    moveReiszeState.resizeEdgets = edge;

    surface->setResizeing(true);
    setResizingItem(shell);
}

void Helper::cancelMoveResize(WSurfaceItem *shell)
{
    if (moveReiszeState.surfaceItem != shell)
        return;
    stopMoveResize();
}

WSurface *Helper::getFocusSurfaceFrom(QObject *object)
{
    auto item = WSurfaceItem::fromFocusObject(object);
    return item ? item->surface() : nullptr;
}

void Helper::allowNonDrmOutputAutoChangeMode(WOutput *output)
{
    connect(output->handle(), &QWOutput::requestState, this, &Helper::onOutputRequeseState);
}

void Helper::enableOutput(WOutput *output)
{
    // Enable on default
    auto qwoutput = output->handle();
    // Don't care for WOutput::isEnabled, must do WOutput::commit here,
    // In order to ensure trigger QWOutput::frame signal, WOutputRenderWindow
    // needs this signal to render next frmae. Because QWOutput::frame signal
    // maybe emit before WOutputRenderWindow::attach, if no commit here,
    // WOutputRenderWindow will ignore this ouptut on render.
    if (!qwoutput->property("_Enabled").toBool()) {
        qwoutput->setProperty("_Enabled", true);

        if (!qwoutput->handle()->current_mode) {
            auto mode = qwoutput->preferredMode();
            if (mode)
                output->setMode(mode);
        }
        output->enable(true);
        bool ok = output->commit();
        Q_ASSERT(ok);
    }
}

bool Helper::beforeDisposeEvent(WSeat *seat, QWindow *watched, QInputEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto kevent = static_cast<QKeyEvent*>(event);
        auto modifiers = kevent->modifiers();
        auto key = kevent->key();
        // printf("modifiers: %d, key: %d\n", modifiers, key);
        if (modifiers.testFlag(Qt::MetaModifier)) {
            switch (key) {
            case Qt::Key_Q:
                qApp->quit();
                return true;
            case Qt::Key_Space:
                Q_EMIT switchLayout();
                return true;
            case Qt::Key_H:
                Q_EMIT resizePane(10, 1); // 左
                return true;
            case Qt::Key_J:
                Q_EMIT resizePane(10, 4); // 下
                return true;
            case Qt::Key_K:
                Q_EMIT resizePane(10, 3); // 上
                return true;
            case Qt::Key_L:
                Q_EMIT resizePane(10, 2); // 右
                return true;
            case Qt::Key_T: // This is because the <Meta + S> has been used by stack layout.
                Q_EMIT swapPane();
                return true;
            case Qt::Key_1:
                Q_EMIT choosePane(1);
                return true;
            case Qt::Key_2:
                Q_EMIT choosePane(2);
                return true;
            case Qt::Key_D:
                Q_EMIT removePane(1);
                return true;
            case Qt::Key_M:
                return true;
            case Qt::Key_Tab:
                Q_EMIT swapPane();
                return true;
            default:
                // printf("undefined modifiers: %d, key: %d\n", modifiers, key);
                return false;
            }
        }

        if (modifiers.testFlag(Qt::AltModifier)) {
            switch (key) {
            case Qt::Key_C:
                Q_EMIT createWs();
                return true;
            case Qt::Key_D:
                Q_EMIT destoryWs();
                return true;
            case Qt::Key_M:
                return true;
            case Qt::Key_S:
                Q_EMIT switchNextWs();
                return true;
            }
        }
    }

    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
            seat->cursor()->setVisible(true);
        } else if (event->type() == QEvent::TouchBegin) {
            seat->cursor()->setVisible(false);
    }

    if (watched) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TouchBegin) {
            seat->setKeyboardFocusWindow(watched);
        } else if (event->type() == QEvent::MouseMove && !seat->keyboardFocusWindow()) {
            // TouchMove keep focus on first window
            seat->setKeyboardFocusWindow(watched);
        }
    }

    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
        seat->cursor()->setVisible(true);
    } else if (event->type() == QEvent::TouchBegin) {
        seat->cursor()->setVisible(false);
    }

    if (moveReiszeState.surfaceItem
        && (seat == moveReiszeState.seat || moveReiszeState.seat == nullptr)) {
        // for move resize
        if (Q_LIKELY(event->type() == QEvent::MouseMove || event->type() == QEvent::TouchUpdate)) {
            auto cursor = seat->cursor();
            Q_ASSERT(cursor);
            QMouseEvent *ev = static_cast<QMouseEvent *>(event);

            if (moveReiszeState.resizeEdgets == 0) {
                auto increment_pos =
                    ev->globalPosition() - cursor->lastPressedOrTouchDownPosition();
                auto new_pos = moveReiszeState.surfacePosOfStartMoveResize
                    + moveReiszeState.surfaceItem->parentItem()->mapFromGlobal(increment_pos);
                moveReiszeState.surfaceItem->setPosition(new_pos);
            } else {
                auto increment_pos = moveReiszeState.surfaceItem->parentItem()->mapFromGlobal(
                    ev->globalPosition() - cursor->lastPressedOrTouchDownPosition());
                QRectF geo(moveReiszeState.surfacePosOfStartMoveResize,
                           moveReiszeState.surfaceSizeOfStartMoveResize);

                if (moveReiszeState.resizeEdgets & Qt::LeftEdge)
                    geo.setLeft(geo.left() + increment_pos.x());
                if (moveReiszeState.resizeEdgets & Qt::TopEdge)
                    geo.setTop(geo.top() + increment_pos.y());

                if (moveReiszeState.resizeEdgets & Qt::RightEdge)
                    geo.setRight(geo.right() + increment_pos.x());
                if (moveReiszeState.resizeEdgets & Qt::BottomEdge)
                    geo.setBottom(geo.bottom() + increment_pos.y());

                if (moveReiszeState.surfaceItem->resizeSurface(geo.size().toSize()))
                    moveReiszeState.surfaceItem->setPosition(geo.topLeft());
            }

            return true;
        } else if (event->type() == QEvent::MouseButtonRelease
                   || event->type() == QEvent::TouchEnd) {
            stopMoveResize();
        }
    }

    return false;
}

bool Helper::afterHandleEvent(
    WSeat *seat, WSurface *watched, QObject *surfaceItem, QObject *, QInputEvent *event)
{
    Q_UNUSED(seat)

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::TouchBegin) {
        // surfaceItem is qml type: XdgSurfaceItem or LayerSurfaceItem
        auto toplevelSurface = qobject_cast<WSurfaceItem *>(surfaceItem)->shellSurface();
        if (!toplevelSurface)
            return false;
        Q_ASSERT(toplevelSurface->surface() == watched);
        if (auto *xdgSurface = qobject_cast<WXdgSurface *>(toplevelSurface)) {
            // TODO(waylib): popupSurface should not inherit WToplevelSurface
            if (xdgSurface->isPopup()) {
                return false;
            }
        }
        setActivateSurface(toplevelSurface);
    }

    return false;
}

bool Helper::unacceptedEvent(WSeat *, QWindow *, QInputEvent *event)
{
    if (event->isSinglePointEvent()) {
        if (static_cast<QSinglePointEvent *>(event)->isBeginEvent())
            setActivateSurface(nullptr);
    }

    return false;
}

WToplevelSurface *Helper::activatedSurface() const
{
    return m_activateSurface;
}

void Helper::setActivateSurface(WToplevelSurface *newActivate)
{
    qCDebug(HelperDebugLog) << newActivate;
    if (newActivate) {
        wl_client *client = newActivate->surface()->handle()->handle()->resource->client;
        pid_t pid;
        uid_t uid;
        gid_t gid;
        wl_client_get_credentials(client, &pid, &uid, &gid);

        QString programName;
        QFile file(QString("/proc/%1/status").arg(pid));
        if (file.open(QFile::ReadOnly)) {
            programName =
                QString(file.readLine()).section(QRegularExpression("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
        }

        if (programName == "ocean-desktop") {
            return;
        }
    }

    if (m_activateSurface == newActivate)
        return;

    if (newActivate && newActivate->doesNotAcceptFocus())
        return;

    if (m_activateSurface && m_activateSurface->isActivated()) {
        if (newActivate) {
            qCDebug(HelperDebugLog)
                << "newActivate keyboardFocusPriority " << newActivate->keyboardFocusPriority();
            if (m_activateSurface->keyboardFocusPriority() > newActivate->keyboardFocusPriority())
                return;
        } else {
            if (m_activateSurface->keyboardFocusPriority() > 0)
                return;
        }
        m_activateSurface->setActivate(false);
    }

    static QMetaObject::Connection invalidCheck;
    disconnect(invalidCheck);

    m_activateSurface = newActivate;

    qCDebug(HelperDebugLog) << "Surface: " << newActivate << " is activated";

    invalidCheck =
        connect(newActivate, &WToplevelSurface::aboutToBeInvalidated, this, [newActivate, this] {
            newActivate->setActivate(false);
            setActivateSurface(nullptr);
        });

    if (newActivate)
        newActivate->setActivate(true);
    Q_EMIT activatedSurfaceChanged();
}

void Helper::onOutputRequeseState(wlr_output_event_request_state *newState)
{
    if (newState->state->committed & WLR_OUTPUT_STATE_MODE) {
        auto output = qobject_cast<QWOutput *>(sender());

        if (newState->state->mode_type == WLR_OUTPUT_STATE_MODE_CUSTOM) {
            const QSize size(newState->state->custom_mode.width,
                             newState->state->custom_mode.height);
            output->setCustomMode(size, newState->state->custom_mode.refresh);
        } else {
            output->setMode(newState->state->mode);
        }

        output->commit();
    }
}

OutputInfo *Helper::getOutputInfo(WOutput *output)
{
    for (const auto &[woutput, infoPtr] : m_outputExclusiveZoneInfo)
        if (woutput == output)
            return infoPtr;
    auto infoPtr = new OutputInfo;
    m_outputExclusiveZoneInfo.append(std::make_pair(output, infoPtr));
    return infoPtr;
}

void Helper::setCurrentUser(const QString &currentUser)
{
    if (m_currentUser != currentUser) {
        m_currentUser = currentUser;
        Q_EMIT currentUserChanged(currentUser);
    }
}

void Helper::setCurrentWorkspaceId(int currentWorkspaceId)
{
    if (m_currentWorkspaceId == currentWorkspaceId) {
        return;
    }
    m_currentWorkspaceId = currentWorkspaceId;

    Q_EMIT currentWorkspaceIdChanged();
}

QString Helper::waylandSocket() const
{
    return m_waylandSocket;
}

QString Helper::xwaylandSocket() const
{
    return m_xwaylandSocket;
}

void Helper::setWaylandSocket(const QString &socketFile)
{
    m_waylandSocket = socketFile;

    emit socketFileChanged();
}

void Helper::setXWaylandSocket(const QString &socketFile)
{
    m_xwaylandSocket = socketFile;

    emit socketFileChanged();
}

QString Helper::clientName(Waylib::Server::WSurface *surface) const
{
    wl_client *client = surface->handle()->handle()->resource->client;
    pid_t pid;
    uid_t uid;
    gid_t gid;
    wl_client_get_credentials(client, &pid, &uid, &gid);

    QString programName;
    QFile file(QString("/proc/%1/status").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        programName =
            QString(file.readLine()).section(QRegularExpression("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
    }

    qDebug() << "Program name for PID" << pid << "is" << programName;
    return programName;
}

void Helper::closeSurface(Waylib::Server::WSurface *surface)
{
    if (auto s = Waylib::Server::WXdgSurface::fromSurface(surface)) {
        if (!s->isPopup()) {
            s->handle()->topToplevel()->sendClose();
        }
    } else if (auto s = Waylib::Server::WXWaylandSurface::fromSurface(surface)) {
        s->close();
    }
}

bool Helper::addAction(const QString &user, QAction *action)
{
    if (!m_actions.count(user)) {
        m_actions[user] = {};
    }

    auto find = std::ranges::find_if(m_actions[user], [action](QAction *a) {
        return a->shortcut() == action->shortcut();
    });

    if (find == m_actions[user].end()) {
        m_actions[user].push_back(action);
    }

    return find == m_actions[user].end();
}

void Helper::removeAction(const QString &user, QAction *action)
{
    if (!m_actions.count(user)) {
        return;
    }

    std::erase_if(m_actions[user], [action](QAction *a) {
        return a == action;
    });
}

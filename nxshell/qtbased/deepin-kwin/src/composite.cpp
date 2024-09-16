/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2006 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "composite.h"

#include <config-kwin.h>

#include "core/output.h"
#include "core/outputbackend.h"
#include "core/outputlayer.h"
#include "core/overlaywindow.h"
#include "core/renderlayer.h"
#include "core/renderloop.h"
#include "cursordelegate_opengl.h"
#include "cursordelegate_qpainter.h"
#include "dbusinterface.h"
#include "decorations/decoratedclient.h"
#include "deleted.h"
#include "effects.h"
#include "ftrace.h"
#include "internalwindow.h"
#include "openglbackend.h"
#include "qpainterbackend.h"
#include "xrenderbackend.h"
#include "scene/cursorscene.h"
#include "scene/itemrenderer_opengl.h"
#include "scene/itemrenderer_qpainter.h"
#include "scene/itemrenderer_xrender.h"
#include "scene/surfaceitem_x11.h"
#include "scene/workspacescene_opengl.h"
#include "scene/workspacescene_qpainter.h"
#include "scene/workspacescene_xrender.h"
#include "shadow.h"
#include "unmanaged.h"
#include "useractions.h"
#include "utils/common.h"
#include "utils/xcbutils.h"
#include "wayland/surface_interface.h"
#include "wayland_server.h"
#include "workspace.h"
#include "x11syncmanager.h"
#include "x11window.h"

#include <kwinglplatform.h>
#include <kwingltexture.h>

#include <KCrash>
#include <KGlobalAccel>
#include <KLocalizedString>
#if KWIN_BUILD_NOTIFICATIONS
#include <KNotification>
#endif
#include <KSelectionOwner>

#if (QT_VERSION > QT_VERSION_CHECK(5, 11, 3))
#include <QScopeGuard>
#else
#include "utils/qscopeguard.h"
#endif

#include <QDateTime>
#include <QFutureWatcher>
#include <QGSettings/QGSettings>
#include <QMenu>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QTextStream>
#include <QTimerEvent>
#include <QtConcurrentRun>

#include <xcb/composite.h>
#include <xcb/damage.h>

#include <cstdio>
#include <dlfcn.h>
#include <unistd.h>

Q_DECLARE_METATYPE(KWin::X11Compositor::SuspendReason)

#define GSETTINGS_DDE_APPEARANCE "com.deepin.dde.appearance"

#define CONFIGMANAGER_SERVICE   "org.desktopspec.ConfigManager"
#define CONFIGMANAGER_INTERFACE "org.desktopspec.ConfigManager"
#define CONFIGMANAGER_MANAGER_INTERFACE "org.desktopspec.ConfigManager.Manager"

namespace KWin
{
template <typename T> using ScopedCPointer = QScopedPointer<T, QScopedPointerPodDeleter>;
static QByteArray PROP_COMPOSITE_TOGGLING("_NET_KDE_COMPOSITE_TOGGLING");
static xcb_atom_t ATOM_COMPOSITE_TOGGLING = 0;

static void reportCompositeChangeFinished()
{
    xcb_delete_property(kwinApp()->x11Connection(), kwinApp()->x11RootWindow(), ATOM_COMPOSITE_TOGGLING);
}

static void reportCompositeIsAboutToChange(int value)
{
    auto c = kwinApp()->x11Connection();
    if (!c) {
        return;
    }

    if (!ATOM_COMPOSITE_TOGGLING) {
        // get the atom for the propertyName
        ScopedCPointer<xcb_intern_atom_reply_t> atomReply(xcb_intern_atom_reply(c,
                    xcb_intern_atom_unchecked(c, false, PROP_COMPOSITE_TOGGLING.size(),
                        PROP_COMPOSITE_TOGGLING.constData()), NULL));
        if (atomReply.isNull()) {
            return;
        }

        ATOM_COMPOSITE_TOGGLING = atomReply->atom;
    }

    // announce property on root window
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, kwinApp()->x11RootWindow(),
            ATOM_COMPOSITE_TOGGLING, ATOM_COMPOSITE_TOGGLING, 8, 4, &value);
}

static QString DConfigCompositingReplyPath()
{
    static QString path;
    if (!path.isEmpty())
        return path;

    QDBusInterface interfaceRequire(CONFIGMANAGER_SERVICE, "/", CONFIGMANAGER_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> reply = interfaceRequire.call("acquireManager", "org.kde.kwin", "org.kde.kwin.compositing", "");
    if (!reply.isValid()) {
        qCWarning(KWIN_CORE) << "Error in DConfig reply:" << reply.error();
        return "";
    }
    path = reply.value().path();
    return path;
}

static EffectType getDConfigUserEffectType()
{
    QString path = DConfigCompositingReplyPath();
    if (path.isEmpty()) {
        qWarning(KWIN_CORE) << "Fail to get DConfig user type";
        return EffectType::AutoSelect;
    }

    QDBusInterface interfaceValue(CONFIGMANAGER_SERVICE, path, CONFIGMANAGER_MANAGER_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<QVariant> replyValue = interfaceValue.call("value", "user_type");
    int type = replyValue.value().toInt();
    if (type < EffectType::AutoSelect || type > EffectType::NoneCompositor) {
        qCWarning(KWIN_CORE) << "Unknown effect type:" << type;
        return EffectType::AutoSelect;
    }
    return static_cast<EffectType>(type);
}

static void setDConfigUserEffectType(EffectType type)
{
    QString path = DConfigCompositingReplyPath();
    if (path.isEmpty()) {
        qWarning(KWIN_CORE) << "Fail to set DConfig user type";
        return;
    }

    QDBusInterface interfaceValue(CONFIGMANAGER_SERVICE, path, CONFIGMANAGER_MANAGER_INTERFACE, QDBusConnection::systemBus());
    interfaceValue.asyncCall("setValue", "user_type", QVariant::fromValue(QDBusVariant(static_cast<int>(type))));
}

static int devicePerformanceLevel()
{
    static int level = -1;
    if (level != -1) {
        return level;
    }

    level = 0;
    std::unique_ptr<void, int(*)(void*)> file_handle(dlopen("libdtkwmjack.so", RTLD_LAZY), &dlclose);
    if (!file_handle) {
        qCWarning(KWIN_CORE) << "Errors in function dlopen:" << dlerror();
        return 0;
    }

    void *function_handle = dlsym(file_handle.get(), "InitDtkWmDisplay");
    if (!function_handle) {
        qCWarning(KWIN_CORE) << "Fail to find function 'InitDtkWmDisplay':" << dlerror();
        return 0;
    }
    int (*InitDtkWmDisplay)() = reinterpret_cast<int(*)()>(function_handle);

    function_handle = dlsym(file_handle.get(), "GetDevicePerformanceLevel");
    if (!function_handle) {
        qCWarning(KWIN_CORE) << "Fail to find function 'GetDevicePerformanceLevel':" << dlerror();
        return 0;
    }
    int (*GetDevicePerformanceLevle)() = reinterpret_cast<int(*)()>(function_handle);

    function_handle = dlsym(file_handle.get(), "DestoryDtkWmDisplay");
    if (!function_handle) {
        qCWarning(KWIN_CORE) << "Fail to find function 'DestoryDtkWmDisplay':" << dlerror();
        return 0;
    }
    void (*DestoryDtkWmDisplay)() = reinterpret_cast<void(*)()>(function_handle);

    if (InitDtkWmDisplay() != 0) {
        return 0;
    }
    level = GetDevicePerformanceLevle();
    DestoryDtkWmDisplay();

    qCDebug(KWIN_CORE) << "Device performance level:" << level;
    return level;
}

Compositor *Compositor::s_compositor = nullptr;
Compositor *Compositor::self()
{
    return s_compositor;
}

WaylandCompositor *WaylandCompositor::create(QObject *parent)
{
    Q_ASSERT(!s_compositor);
    auto *compositor = new WaylandCompositor(parent);
    s_compositor = compositor;
    return compositor;
}
X11Compositor *X11Compositor::create(QObject *parent)
{
    Q_ASSERT(!s_compositor);
    auto *compositor = new X11Compositor(parent);
    s_compositor = compositor;
    return compositor;
}

class CompositorSelectionOwner : public KSelectionOwner
{
    Q_OBJECT
public:
    CompositorSelectionOwner(const char *selection)
        : KSelectionOwner(selection, kwinApp()->x11Connection(), kwinApp()->x11RootWindow())
        , m_owning(false)
    {
        connect(this, &CompositorSelectionOwner::lostOwnership,
                this, [this]() {
                    m_owning = false;
                });
    }
    bool owning() const
    {
        return m_owning;
    }
    void setOwning(bool own)
    {
        m_owning = own;
    }

private:
    bool m_owning;
};

Compositor::Compositor(QObject *workspace)
    : QObject(workspace)
{
    // 2 sec which should be enough to restart the compositor.
    static const int compositorLostMessageDelay = 100;

    m_releaseSelectionTimer.setSingleShot(true);
    m_releaseSelectionTimer.setInterval(compositorLostMessageDelay);
    connect(&m_releaseSelectionTimer, &QTimer::timeout,
            this, &Compositor::releaseCompositorSelection);

    m_unusedSupportPropertyTimer.setInterval(compositorLostMessageDelay);
    m_unusedSupportPropertyTimer.setSingleShot(true);
    connect(&m_unusedSupportPropertyTimer, &QTimer::timeout,
            this, &Compositor::deleteUnusedSupportProperties);

    // Delay the call to start by one event cycle.
    // The ctor of this class is invoked from the Workspace ctor, that means before
    // Workspace is completely constructed, so calling Workspace::self() would result
    // in undefined behavior. This is fixed by using a delayed invocation.
    QTimer::singleShot(0, this, &Compositor::start);

    connect(kwinApp(), &Application::x11ConnectionChanged, this, &Compositor::initializeX11);
    connect(kwinApp(), &Application::x11ConnectionAboutToBeDestroyed, this, &Compositor::cleanupX11);

    if (!waylandServer()) {
        QString path = DConfigCompositingReplyPath();
        if (!path.isEmpty()) {
            QDBusConnection::systemBus().connect(CONFIGMANAGER_SERVICE, path, CONFIGMANAGER_MANAGER_INTERFACE,
                    "valueChanged", this, SLOT(handleDConfigUserTypeChanged(QString)));
        } else {
            qCWarning(KWIN_CORE) << "Fail to connect to DConfig valueChanged";
        }
    }

    // register DBus
    new CompositorDBusInterface(this);
    FTraceLogger::create();
}

Compositor::~Compositor()
{
    deleteUnusedSupportProperties();
    destroyCompositorSelection();
    s_compositor = nullptr;
}

bool Compositor::attemptOpenGLCompositing()
{
    // Some broken drivers crash on glXQuery() so to prevent constant KWin crashes:
    if (openGLCompositingIsBroken()) {
        qCWarning(KWIN_CORE) << "KWin has detected that your OpenGL library is unsafe to use";
        return false;
    }

    createOpenGLSafePoint(OpenGLSafePoint::PreInit);
    auto safePointScope = qScopeGuard([this]() {
        createOpenGLSafePoint(OpenGLSafePoint::PostInit);
    });

    std::unique_ptr<OpenGLBackend> backend = kwinApp()->outputBackend()->createOpenGLBackend();
    if (!backend) {
        return false;
    }
    if (!backend->isFailed()) {
        backend->init();
    }
    if (backend->isFailed()) {
        return false;
    }

    const QByteArray forceEnv = qgetenv("KWIN_COMPOSE");
    if (!forceEnv.isEmpty()) {
        if (qstrcmp(forceEnv, "O2") == 0 || qstrcmp(forceEnv, "O2ES") == 0) {
            qCDebug(KWIN_CORE) << "OpenGL 2 compositing enforced by environment variable";
        } else {
            // OpenGL 2 disabled by environment variable
            return false;
        }
    } else {
        if (!backend->isDirectRendering()) {
            return false;
        }
        if (GLPlatform::instance()->recommendedCompositor() < OpenGLCompositing) {
            qCDebug(KWIN_CORE) << "Driver does not recommend OpenGL compositing";
            return false;
        }
    }

    // We only support the OpenGL 2+ shader API, not GL_ARB_shader_objects
    if (!hasGLVersion(2, 0)) {
        qCDebug(KWIN_CORE) << "OpenGL 2.0 is not supported";
        return false;
    }

    m_scene = std::make_unique<WorkspaceSceneOpenGL>(backend.get());
    m_cursorScene = std::make_unique<CursorScene>(std::make_unique<ItemRendererOpenGL>());
    m_backend = std::move(backend);

    // set strict binding
    if (options->isGlStrictBindingFollowsDriver()) {
        options->setGlStrictBinding(!GLPlatform::instance()->supports(LooseBinding));
    }

    qCDebug(KWIN_CORE) << "OpenGL compositing has been successfully initialized";
    return true;
}

bool Compositor::attemptQPainterCompositing()
{
    std::unique_ptr<QPainterBackend> backend(kwinApp()->outputBackend()->createQPainterBackend());
    if (!backend || backend->isFailed()) {
        return false;
    }

    m_scene = std::make_unique<WorkspaceSceneQPainter>(backend.get());
    m_cursorScene = std::make_unique<CursorScene>(std::make_unique<ItemRendererQPainter>());
    m_backend = std::move(backend);

    qCDebug(KWIN_CORE) << "QPainter compositing has been successfully initialized";
    return true;
}

bool Compositor::attemptXRenderCompositing()
{
    std::unique_ptr<XRenderBackend> backend(kwinApp()->outputBackend()->createXRenderBackend());
    if (!backend || backend->isFailed()) {
        return false;
    }

    m_scene = std::make_unique<WorkspaceSceneXRender>(backend.get());
    m_cursorScene = std::make_unique<CursorScene>(std::make_unique<ItemRendererXRender>());
    m_backend = std::move(backend);

    qCDebug(KWIN_CORE) << "XRender compositing has been successfully initialized";
    return true;
}

bool Compositor::setupStart()
{
    if (kwinApp()->isTerminating()) {
        // Don't start while KWin is terminating. An event to restart might be lingering
        // in the event queue due to graphics reset.
        return false;
    }
    if (m_state != State::Off) {
        return false;
    }
    m_state = State::Starting;

    options->reloadCompositingSettings(true);

    initializeX11();

    // There might still be a deleted around, needs to be cleared before
    // creating the scene (BUG 333275).
    if (Workspace::self()) {
        while (!Workspace::self()->deletedList().isEmpty()) {
            Workspace::self()->deletedList().first()->discard();
        }
    }

    Q_EMIT aboutToToggleCompositing();

    const QVector<CompositingType> availableCompositors = kwinApp()->outputBackend()->supportedCompositors();
    QVector<CompositingType> candidateCompositors;

    CompositingType configSelected = options->compositingMode();
    if (!waylandServer()) {
        configSelected = m_effectType == EffectType::XRenderComplete ? XRenderCompositing : OpenGLCompositing;
    }

    // If compositing has been restarted, try to use the last used compositing type.
    if (m_selectedCompositor != NoCompositing) {
        candidateCompositors.append(m_selectedCompositor);
    } else {
        candidateCompositors = availableCompositors;
        if (!waylandServer()) {
            const auto userConfigIt = std::find(candidateCompositors.begin(), candidateCompositors.end(), configSelected);
            if (userConfigIt != candidateCompositors.end()) {
                candidateCompositors.erase(userConfigIt);
                candidateCompositors.prepend(configSelected);
            } else {
                qCWarning(KWIN_CORE) << "Configured compositor not supported by Platform. Falling back to defaults";
            }
        }
    }

    for (auto type : std::as_const(candidateCompositors)) {
        bool stop = false;
        switch (type) {
        case XRenderCompositing:
            qCDebug(KWIN_CORE) << "Attempting to load the XRender scene";
            stop = attemptXRenderCompositing();
            break;
        case OpenGLCompositing:
            qCDebug(KWIN_CORE) << "Attempting to load the OpenGL scene";
            stop = attemptOpenGLCompositing();
            break;
        case QPainterCompositing:
            qCDebug(KWIN_CORE) << "Attempting to load the QPainter scene";
            stop = attemptQPainterCompositing();
            break;
        case NoCompositing:
            qCDebug(KWIN_CORE) << "Starting without compositing...";
            stop = true;
            break;
        }

        if (stop) {
            break;
        }
    }

    if (!m_backend) {
        m_state = State::Off;

        if (m_selectionOwner) {
            m_selectionOwner->setOwning(false);
            m_selectionOwner->release();
        }
        if (!availableCompositors.contains(NoCompositing)) {
            qCCritical(KWIN_CORE) << "The used windowing system requires compositing";
            qCCritical(KWIN_CORE) << "We are going to quit KWin now as it is broken";
            qApp->quit();
        }
        return false;
    }

    if (waylandServer()) {
        m_selectedCompositor = m_backend->compositingType();
    }

    if (!Workspace::self() && m_backend && m_backend->compositingType() == QPainterCompositing) {
        // Force Software QtQuick on first startup with QPainter.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#else
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#endif
    }

    if (!waylandServer() && m_backend && QGSettings::isSchemaInstalled(GSETTINGS_DDE_APPEARANCE)) {
        // When change compositor to other type, set the opacity to the recorded opacity.
        if (m_backend->compositingType() == XRenderCompositing) {
            QGSettings(GSETTINGS_DDE_APPEARANCE).set("opacity", 1);
        } else {
            KConfigGroup config(KSharedConfig::openConfig("deepin-kwinrc"), "Compositing");
            float opacity = config.readEntry("OpacityRecord", QVariant(-1.0f)).toFloat();
            if (opacity != -1.0f) {
                QGSettings(GSETTINGS_DDE_APPEARANCE).set("opacity", opacity);
            }
        }
    }

    Q_EMIT sceneCreated();
    reportCompositeIsAboutToChange(1);

    return true;
}

void Compositor::initializeX11()
{
    xcb_connection_t *connection = kwinApp()->x11Connection();
    if (!connection) {
        return;
    }

    if (!m_selectionOwner) {
        m_selectionOwner = std::make_unique<CompositorSelectionOwner>("_NET_WM_CM_S0");
        connect(m_selectionOwner.get(), &CompositorSelectionOwner::lostOwnership, this, &Compositor::stop);
    }
    if (!m_selectionOwner->owning()) {
        // Force claim ownership.
        m_selectionOwner->claim(true);
        m_selectionOwner->setOwning(true);
    }

    if (waylandServer()) {
        xcb_composite_redirect_subwindows(connection, kwinApp()->x11RootWindow(),
                                          XCB_COMPOSITE_REDIRECT_MANUAL);
    }
}

void Compositor::cleanupX11()
{
    m_selectionOwner.reset();
}

void Compositor::startupWithWorkspace()
{
    Q_ASSERT(m_scene);
    m_scene->initialize();
    m_cursorScene->initialize();

    const QList<Output *> outputs = workspace()->outputs();
    if (kwinApp()->operationMode() == Application::OperationModeX11) {
        auto workspaceLayer = new RenderLayer(outputs.constFirst()->renderLoop());
        workspaceLayer->setDelegate(std::make_unique<SceneDelegate>(m_scene.get()));
        workspaceLayer->setGeometry(workspace()->geometry());
        connect(workspace(), &Workspace::geometryChanged, workspaceLayer, [workspaceLayer]() {
            workspaceLayer->setGeometry(workspace()->geometry());
        });
        addSuperLayer(workspaceLayer);
    } else {
        for (Output *output : outputs) {
            addOutput(output);
        }
        connect(workspace(), &Workspace::outputAdded, this, &Compositor::addOutput);
        connect(workspace(), &Workspace::outputRemoved, this, &Compositor::removeOutput);
    }

    m_state = State::On;

    for (X11Window *window : Workspace::self()->clientList()) {
        window->setupCompositing();
    }
    for (Unmanaged *window : Workspace::self()->unmanagedList()) {
        window->setupCompositing();
    }
    for (InternalWindow *window : workspace()->internalWindows()) {
        window->setupCompositing();
    }

    if (auto *server = waylandServer()) {
        const auto windows = server->windows();
        for (Window *window : windows) {
            window->setupCompositing();
        }
    }

    // Sets also the 'effects' pointer.
    kwinApp()->createEffectsHandler(this, m_scene.get());

    Q_EMIT compositingToggled(true);

    static int lastCompositingType = -1;
    int currentCompositingType = m_backend ? m_backend->compositingType() : CompositingType::NoCompositing;
    if (currentCompositingType != lastCompositingType && lastCompositingType != -1) {
        Q_EMIT effectsEnabledChanged(currentCompositingType == CompositingType::OpenGLCompositing);
    }
    lastCompositingType = currentCompositingType;

    if (m_releaseSelectionTimer.isActive()) {
        m_releaseSelectionTimer.stop();
    }

    xcb_connection_t *connection = kwinApp()->x11Connection();
    if (connection) {
        xcb_composite_redirect_subwindows(connection, kwinApp()->x11RootWindow(),
                                          XCB_COMPOSITE_REDIRECT_MANUAL);
    }
}

Output *Compositor::findOutput(RenderLoop *loop) const
{
    const auto outputs = workspace()->outputs();
    for (Output *output : outputs) {
        if (output->renderLoop() == loop) {
            return output;
        }
    }
    return nullptr;
}

void Compositor::addOutput(Output *output)
{
    Q_ASSERT(kwinApp()->operationMode() != Application::OperationModeX11);

    auto workspaceLayer = new RenderLayer(output->renderLoop());
    workspaceLayer->setDelegate(std::make_unique<SceneDelegate>(m_scene.get(), output));
    workspaceLayer->setGeometry(output->rect());
    connect(output, &Output::geometryChanged, workspaceLayer, [output, workspaceLayer]() {
        workspaceLayer->setGeometry(output->rect());
    });

    auto cursorLayer = new RenderLayer(output->renderLoop());
    cursorLayer->setVisible(false);
    if (m_backend->compositingType() == OpenGLCompositing) {
        cursorLayer->setDelegate(std::make_unique<CursorDelegateOpenGL>());
    } else {
        cursorLayer->setDelegate(std::make_unique<CursorDelegateQPainter>());
    }
    cursorLayer->setParent(workspaceLayer);
    cursorLayer->setSuperlayer(workspaceLayer);

    auto updateCursorLayer = [output, cursorLayer]() {
        const Cursor *cursor = Cursors::self()->currentCursor();
        const QRect layerRect = output->mapFromGlobal(cursor->geometry());
        bool usesHardwareCursor = false;
        if (!Cursors::self()->isCursorHidden()) {
            usesHardwareCursor = output->setCursor(cursor->source()) && output->moveCursor(layerRect.topLeft());
        } else {
            usesHardwareCursor = output->setCursor(nullptr);
        }
        cursorLayer->setVisible(cursor->isOnOutput(output) && !usesHardwareCursor);
        cursorLayer->setGeometry(layerRect);
        cursorLayer->addRepaintFull();
    };
    auto moveCursorLayer = [output, cursorLayer]() {
        const Cursor *cursor = Cursors::self()->currentCursor();
        const QRect layerRect = output->mapFromGlobal(cursor->geometry());
        const bool usesHardwareCursor = output->moveCursor(layerRect.topLeft());
        cursorLayer->setVisible(cursor->isOnOutput(output) && !usesHardwareCursor);
        cursorLayer->setGeometry(layerRect);
        cursorLayer->addRepaintFull();
    };
    updateCursorLayer();
    connect(output, &Output::geometryChanged, cursorLayer, updateCursorLayer);
    connect(Cursors::self(), &Cursors::currentCursorChanged, cursorLayer, updateCursorLayer);
    connect(Cursors::self(), &Cursors::hiddenChanged, cursorLayer, updateCursorLayer);
    connect(Cursors::self(), &Cursors::positionChanged, cursorLayer, moveCursorLayer);

    addSuperLayer(workspaceLayer);
}

void Compositor::removeOutput(Output *output)
{
    removeSuperLayer(m_superlayers[output->renderLoop()]);
}

void Compositor::addSuperLayer(RenderLayer *layer)
{
    m_superlayers.insert(layer->loop(), layer);
    connect(layer->loop(), &RenderLoop::frameRequested, this, &Compositor::handleFrameRequested);
}

void Compositor::removeSuperLayer(RenderLayer *layer)
{
    m_superlayers.remove(layer->loop());
    disconnect(layer->loop(), &RenderLoop::frameRequested, this, &Compositor::handleFrameRequested);
    delete layer;
}

void Compositor::scheduleRepaint()
{
    for (auto it = m_superlayers.constBegin(); it != m_superlayers.constEnd(); ++it) {
        it.key()->scheduleRepaint();
    }
}

void Compositor::stop()
{
    if (m_state == State::Off || m_state == State::Stopping) {
        return;
    }
    reportCompositeIsAboutToChange(0);
    m_state = State::Stopping;
    Q_EMIT aboutToToggleCompositing();

    // Opacity is logged to restore when exit from OpenGL compositing.
    if (!waylandServer() && m_backend && m_backend->compositingType() & OpenGLCompositing
            && QGSettings::isSchemaInstalled(GSETTINGS_DDE_APPEARANCE)) {
        KConfigGroup config(KSharedConfig::openConfig("deepin-kwinrc"), "Compositing");
        float opacity = QGSettings(GSETTINGS_DDE_APPEARANCE).get("opacity").toFloat();
        config.writeEntry("OpacityRecord", opacity);
        config.sync();
    }

    m_releaseSelectionTimer.start();

    // Some effects might need access to effect windows when they are about to
    // be destroyed, for example to unreference deleted windows, so we have to
    // make sure that effect windows outlive effects.
    delete effects;
    effects = nullptr;

    if (Workspace::self()) {
        for (X11Window *window : Workspace::self()->clientList()) {
            window->finishCompositing();
        }
        for (Unmanaged *window : Workspace::self()->unmanagedList()) {
            window->finishCompositing();
        }
        for (InternalWindow *window : workspace()->internalWindows()) {
            window->finishCompositing();
        }
        if (auto *con = kwinApp()->x11Connection()) {
            xcb_composite_unredirect_subwindows(con, kwinApp()->x11RootWindow(),
                                                XCB_COMPOSITE_REDIRECT_MANUAL);
        }
        while (!workspace()->deletedList().isEmpty()) {
            workspace()->deletedList().first()->discard();
        }

        disconnect(workspace(), &Workspace::outputAdded, this, &Compositor::addOutput);
        disconnect(workspace(), &Workspace::outputRemoved, this, &Compositor::removeOutput);
    }

    if (waylandServer()) {
        const QList<Window *> toFinishCompositing = waylandServer()->windows();
        for (Window *window : toFinishCompositing) {
            window->finishCompositing();
        }
    }

    const auto superlayers = m_superlayers;
    for (auto it = superlayers.begin(); it != superlayers.end(); ++it) {
        removeSuperLayer(*it);
    }

    m_scene.reset();
    m_cursorScene.reset();
    m_backend.reset();

    m_state = State::Off;
    Q_EMIT compositingToggled(false);
}

void Compositor::destroyCompositorSelection()
{
    m_selectionOwner.reset();
}

void Compositor::releaseCompositorSelection()
{
    switch (m_state) {
    case State::On:
        // We are compositing at the moment. Don't release.
        break;
    case State::Off:
        if (m_selectionOwner) {
            qCDebug(KWIN_CORE) << "Releasing compositor selection";
            m_selectionOwner->setOwning(false);
            m_selectionOwner->release();
        }
        break;
    case State::Starting:
    case State::Stopping:
        // Still starting or shutting down the compositor. Starting might fail
        // or after stopping a restart might follow. So test again later on.
        m_releaseSelectionTimer.start();
        break;
    }
}

void Compositor::keepSupportProperty(xcb_atom_t atom)
{
    m_unusedSupportProperties.removeAll(atom);
}

void Compositor::removeSupportProperty(xcb_atom_t atom)
{
    m_unusedSupportProperties << atom;
    m_unusedSupportPropertyTimer.start();
}

void Compositor::deleteUnusedSupportProperties()
{
    if (m_state == State::Starting || m_state == State::Stopping) {
        // Currently still maybe restarting the compositor.
        m_unusedSupportPropertyTimer.start();
        return;
    }
    if (auto *con = kwinApp()->x11Connection()) {
        for (const xcb_atom_t &atom : std::as_const(m_unusedSupportProperties)) {
            // remove property from root window
            xcb_delete_property(con, kwinApp()->x11RootWindow(), atom);
        }
        m_unusedSupportProperties.clear();
    }
}

void Compositor::reinitialize()
{
    // Reparse config. Config options will be reloaded by start()
    kwinApp()->config()->reparseConfiguration();

    // Restart compositing
    stop();
    start();

    if (effects) { // start() may fail
        effects->reconfigure();
    }
}

void Compositor::handleFrameRequested(RenderLoop *renderLoop)
{
    static quint32 frame = 0;
    frame++;
    if (m_benchWindowNum > 0 && frame % 2 == 0) {
        return;
    }
    composite(renderLoop);
}

void Compositor::handleDConfigUserTypeChanged(const QString &type)
{
    if (type == "user_type") {
        EffectType effectType = getDConfigUserEffectType();
        if (effectType ==  EffectType::AutoSelect || effectType == m_effectType) {
            return;
        }

        // from opengl to opengl
        if ((m_effectType == EffectType::OpenGLComplete || m_effectType == EffectType::OpenGLNoMotion) &&
                (effectType == EffectType::OpenGLComplete || effectType == EffectType::OpenGLNoMotion)) {
            m_effectType = effectType;
            EffectsHandlerImpl *e = static_cast<EffectsHandlerImpl *>(effects);
            if (m_effectType == EffectType::OpenGLComplete) {
                KConfigGroup config(kwinApp()->config(), "Plugins");
                for (const QString &name : EffectsHandlerEx::motionEffectList) {
                    if (!e->isEffectLoaded(name) && config.readEntry(name + "Enabled", true))
                        e->loadEffect(name, false);
                }
            } else {
                for (const QString &name : EffectsHandlerEx::motionEffectList) {
                    if (e->isEffectLoaded(name))
                        e->unloadEffect(name, false);
                }
            }

            e->reconfigureEffect("multitaskview");
            e->reconfigureEffect("alttabthumbnaillist");
            return;
        }

        reinitialize();
    }
}

void Compositor::composite(RenderLoop *renderLoop)
{
    if (m_backend->checkGraphicsReset()) {
        qCDebug(KWIN_CORE) << "Graphics reset occurred...";
        reinitialize();
        return;
    }

    Output *output = findOutput(renderLoop);
    OutputLayer *primaryLayer = m_backend->primaryLayer(output);
    fTraceDuration("Paint (", output->name(), ")");

    RenderLayer *superLayer = m_superlayers[renderLoop];
    prePaintPass(superLayer);
    superLayer->setOutputLayer(primaryLayer);

    SurfaceItem *scanoutCandidate = superLayer->delegate()->scanoutCandidate();
    renderLoop->setFullscreenSurface(scanoutCandidate);
    output->setContentType(scanoutCandidate ? scanoutCandidate->contentType() : ContentType::None);

    renderLoop->beginFrame();
    bool directScanout = false;
    if (scanoutCandidate) {
        const auto sublayers = superLayer->sublayers();
        const bool scanoutPossible = std::none_of(sublayers.begin(), sublayers.end(), [](RenderLayer *sublayer) {
            return sublayer->isVisible();
        });
        if (scanoutPossible && !output->directScanoutInhibited()) {
            directScanout = primaryLayer->scanout(scanoutCandidate);
        }
    }

    if (!directScanout) {
        QRegion surfaceDamage = primaryLayer->repaints();
        primaryLayer->resetRepaints();
        preparePaintPass(superLayer, &surfaceDamage);

        if (auto beginInfo = primaryLayer->beginFrame()) {
            auto &[renderTarget, repaint] = beginInfo.value();
            renderTarget.setDevicePixelRatio(output->scale());

            const QRegion bufferDamage = surfaceDamage.united(repaint).intersected(superLayer->rect());
            primaryLayer->aboutToStartPainting(bufferDamage);

            // TODO: fixme
            if (waylandServer() && workspace()->outputs().size() > 1) {
                surfaceDamage = bufferDamage;
            }
            paintPass(superLayer, &renderTarget, bufferDamage);
            primaryLayer->endFrame(bufferDamage, surfaceDamage);
        }
    }

    postPaintPass(superLayer);
    renderLoop->endFrame();

    m_backend->present(output);

    // TODO: Put it inside the cursor layer once the cursor layer can be backed by a real output layer.
    if (waylandServer()) {
        const std::chrono::milliseconds frameTime =
            std::chrono::duration_cast<std::chrono::milliseconds>(output->renderLoop()->lastPresentationTimestamp());

        if (!Cursors::self()->isCursorHidden()) {
            Cursor *cursor = Cursors::self()->currentCursor();
            if (cursor->geometry().intersects(output->geometry())) {
                cursor->markAsRendered(frameTime);
            }
        }
    }
}

void Compositor::prePaintPass(RenderLayer *layer)
{
    layer->delegate()->prePaint();
    const auto sublayers = layer->sublayers();
    for (RenderLayer *sublayer : sublayers) {
        prePaintPass(sublayer);
    }
}

void Compositor::postPaintPass(RenderLayer *layer)
{
    layer->delegate()->postPaint();
    const auto sublayers = layer->sublayers();
    for (RenderLayer *sublayer : sublayers) {
        postPaintPass(sublayer);
    }
}

void Compositor::preparePaintPass(RenderLayer *layer, QRegion *repaint)
{
    // TODO: Cull opaque region.
    *repaint += layer->mapToGlobal(layer->repaints() + layer->delegate()->repaints());
    layer->resetRepaints();
    const auto sublayers = layer->sublayers();
    for (RenderLayer *sublayer : sublayers) {
        if (sublayer->isVisible()) {
            preparePaintPass(sublayer, repaint);
        }
    }
}

void Compositor::paintPass(RenderLayer *layer, RenderTarget *target, const QRegion &region)
{
    layer->delegate()->paint(target, region);

    const auto sublayers = layer->sublayers();
    for (RenderLayer *sublayer : sublayers) {
        if (sublayer->isVisible()) {
            paintPass(sublayer, target, region);
        }
    }
}

bool Compositor::isActive()
{
    return m_state == State::On;
}

bool Compositor::compositingPossible() const
{
    return true;
}

QString Compositor::compositingNotPossibleReason() const
{
    return QString();
}

bool Compositor::openGLCompositingIsBroken() const
{
    return false;
}

void Compositor::createOpenGLSafePoint(OpenGLSafePoint safePoint)
{
}

bool Compositor::isOpenGLCompositing()
{
    if (m_backend)
        return m_backend->compositingType() == OpenGLCompositing;
    return false;
}

bool Compositor::isXrenderCompositing()
{
    if (m_backend)
        return m_backend->compositingType() == XRenderCompositing;
    return false;
}

WaylandCompositor::WaylandCompositor(QObject *parent)
    : Compositor(parent)
{
    connect(kwinApp(), &Application::x11ConnectionAboutToBeDestroyed,
            this, &WaylandCompositor::destroyCompositorSelection);
}

WaylandCompositor::~WaylandCompositor()
{
    Q_EMIT aboutToDestroy();
    stop(); // this can't be called in the destructor of Compositor
}

void WaylandCompositor::toggleCompositing()
{
    // For the shortcut. Not possible on Wayland because we always composite.
}

void WaylandCompositor::start()
{
    if (!Compositor::setupStart()) {
        // Internal setup failed, abort.
        return;
    }

    if (Workspace::self()) {
        startupWithWorkspace();
    } else {
        connect(kwinApp(), &Application::workspaceCreated,
                this, &WaylandCompositor::startupWithWorkspace);
    }
}

X11Compositor::X11Compositor(QObject *parent)
    : Compositor(parent)
    , m_suspended(options->isUseCompositing() ? NoReasonSuspend : UserSuspend)
{
    if (qEnvironmentVariableIsSet("KWIN_MAX_FRAMES_TESTED")) {
        m_framesToTestForSafety = qEnvironmentVariableIntValue("KWIN_MAX_FRAMES_TESTED");
    }

    connect(options, &Options::configChanged, this, [this]() {
        if (m_suspended) {
            stop();
        } else {
            start();
            if (effects)   // setupCompositing() may fail
                effects->reconfigure();
            if (WorkspaceScene *s = scene())
                s->addRepaintFull();
        }
    });
}

X11Compositor::~X11Compositor()
{
    Q_EMIT aboutToDestroy();
    if (m_openGLFreezeProtectionThread) {
        m_openGLFreezeProtectionThread->quit();
        m_openGLFreezeProtectionThread->wait();
    }
    stop(); // this can't be called in the destructor of Compositor
}

X11SyncManager *X11Compositor::syncManager() const
{
    return m_syncManager.get();
}

void X11Compositor::toggleCompositing()
{
    if (m_suspended) {
        // Direct user call; clear all bits.
        resume(AllReasonSuspend);
    } else {
        // But only set the user one (sufficient to suspend).
        suspend(UserSuspend);
    }
}

void X11Compositor::reinitialize()
{
    // Resume compositing if suspended.
    m_suspended = NoReasonSuspend;
    Compositor::reinitialize();
}

void X11Compositor::suspend(X11Compositor::SuspendReason reason)
{
    Q_ASSERT(reason != NoReasonSuspend);
    m_suspended |= reason;

    if (reason & ScriptSuspend) {
        // When disabled show a shortcut how the user can get back compositing.
        const auto shortcuts = KGlobalAccel::self()->shortcut(
            workspace()->findChild<QAction *>(QStringLiteral("Suspend Compositing")));
        if (!shortcuts.isEmpty()) {
            // Display notification only if there is the shortcut.
            const QString message =
                i18n("Desktop effects have been suspended by another application.<br/>"
                     "You can resume using the '%1' shortcut.",
                     shortcuts.first().toString(QKeySequence::NativeText));
#if KWIN_BUILD_NOTIFICATIONS
            KNotification::event(QStringLiteral("compositingsuspendeddbus"), message);
#endif
        }
    }
    stop();
}

void X11Compositor::resume(X11Compositor::SuspendReason reason)
{
    Q_ASSERT(reason != NoReasonSuspend);
    m_suspended &= ~reason;
    start();
}

void X11Compositor::start()
{
    if (m_suspended) {
        QStringList reasons;
        if (m_suspended & UserSuspend) {
            reasons << QStringLiteral("Disabled by User");
        }
        if (m_suspended & BlockRuleSuspend) {
            reasons << QStringLiteral("Disabled by Window");
        }
        if (m_suspended & ScriptSuspend) {
            reasons << QStringLiteral("Disabled by Script");
        }
        m_effectType = EffectType::NoneCompositor;
        setDConfigUserEffectType(m_effectType);
        qCInfo(KWIN_CORE) << "Compositing is suspended, reason:" << reasons;
        return;
    } else if (!compositingPossible()) {
        m_effectType = EffectType::NoneCompositor;
        setDConfigUserEffectType(m_effectType);
        qCWarning(KWIN_CORE) << "Compositing is not possible";
        return;
    }

    KConfigGroup config(kwinApp()->config(), "Compositing");
    if (config.hasKey("Enabled")) {
        m_effectType = config.readEntry("Enabled", true) ? EffectType::OpenGLComplete : EffectType::XRenderComplete;
        setDConfigUserEffectType(m_effectType);
        config.deleteEntry("Enabled");
        config.sync();
    } else {
        m_effectType = getDConfigUserEffectType();
    }

    if (m_effectType == EffectType::AutoSelect) {
        int level = devicePerformanceLevel();
        if (level == 1) {
            m_effectType = EffectType::OpenGLComplete;
        } else if (level == 2) {
            m_effectType = EffectType::OpenGLNoMotion;
        } else {
            m_effectType = EffectType::XRenderComplete;
        }
        setDConfigUserEffectType(m_effectType);
        qCDebug(KWIN_CORE) << "Effect type is automatically set to" << m_effectType;
    }

    qCDebug(KWIN_CORE) << "Current effect type:" << m_effectType;

    if (m_effectType == EffectType::NoneCompositor) {
        qCDebug(KWIN_CORE) << "Compositing is disabled by DConfig";
        return;
    }

    if (!Compositor::setupStart()) {
        // Internal setup failed, abort.
        return;
    }
    startupWithWorkspace();
    m_syncManager.reset(X11SyncManager::create());
}

void X11Compositor::stop()
{
    m_syncManager.reset();
    Compositor::stop();
}

void X11Compositor::composite(RenderLoop *renderLoop)
{
    if (backend()->overlayWindow() && !isOverlayWindowVisible()) {
        // Return since nothing is visible.
        return;
    }

    QList<Window *> windows = workspace()->stackingOrder();
    QList<SurfaceItemX11 *> dirtyItems;

    // Reset the damage state of each window and fetch the damage region
    // without waiting for a reply
    for (Window *window : std::as_const(windows)) {
        SurfaceItemX11 *surfaceItem = static_cast<SurfaceItemX11 *>(window->surfaceItem());
        if (surfaceItem->fetchDamage()) {
            dirtyItems.append(surfaceItem);
        }
    }

    if (dirtyItems.count() > 0) {
        if (m_syncManager) {
            m_syncManager->triggerFence();
        }
        xcb_flush(kwinApp()->x11Connection());
    }

    // Get the replies
    for (SurfaceItemX11 *item : std::as_const(dirtyItems)) {
        item->waitForDamage();
    }

    if (m_framesToTestForSafety > 0 && (backend()->compositingType() & OpenGLCompositing)) {
        createOpenGLSafePoint(OpenGLSafePoint::PreFrame);
    }

    Compositor::composite(renderLoop);

    if (m_syncManager) {
        if (!m_syncManager->endFrame()) {
            qCDebug(KWIN_CORE) << "Aborting explicit synchronization with the X command stream.";
            qCDebug(KWIN_CORE) << "Future frames will be rendered unsynchronized.";
            m_syncManager.reset();
        }
    }

    if (m_framesToTestForSafety > 0) {
        if (backend()->compositingType() & OpenGLCompositing) {
            createOpenGLSafePoint(OpenGLSafePoint::PostFrame);
        }
        m_framesToTestForSafety--;
        if (m_framesToTestForSafety == 0 && (backend()->compositingType() & OpenGLCompositing)) {
            createOpenGLSafePoint(OpenGLSafePoint::PostLastGuardedFrame);
        }
    }
}

bool X11Compositor::checkForOverlayWindow(WId w) const
{
    if (!backend()) {
        // No backend, so it cannot be the overlay window.
        return false;
    }
    if (!backend()->overlayWindow()) {
        // No overlay window, it cannot be the overlay.
        return false;
    }
    // Compare the window ID's.
    return w == backend()->overlayWindow()->window();
}

bool X11Compositor::isOverlayWindowVisible() const
{
    if (!backend()) {
        return false;
    }
    if (!backend()->overlayWindow()) {
        return false;
    }
    return backend()->overlayWindow()->isVisible();
}

void X11Compositor::updateClientCompositeBlocking(X11Window *c)
{
    if (c) {
        if (c->isBlockingCompositing()) {
            // Do NOT attempt to call suspend(true) from within the eventchain!
            if (!(m_suspended & BlockRuleSuspend)) {
                QMetaObject::invokeMethod(
                    this, [this]() {
                        suspend(BlockRuleSuspend);
                    },
                    Qt::QueuedConnection);
            }
        }
    } else if (m_suspended & BlockRuleSuspend) {
        // If !c we just check if we can resume in case a blocking client was lost.
        bool shouldResume = true;

        for (auto it = Workspace::self()->clientList().constBegin();
             it != Workspace::self()->clientList().constEnd(); ++it) {
            if ((*it)->isBlockingCompositing()) {
                shouldResume = false;
                break;
            }
        }
        if (shouldResume) {
            // Do NOT attempt to call suspend(false) from within the eventchain!
            QMetaObject::invokeMethod(
                this, [this]() {
                    resume(BlockRuleSuspend);
                },
                Qt::QueuedConnection);
        }
    }
}

X11Compositor *X11Compositor::self()
{
    return qobject_cast<X11Compositor *>(Compositor::self());
}

bool X11Compositor::openGLCompositingIsBroken() const
{
    auto timestamp = KConfigGroup(kwinApp()->config(), "Compositing").readEntry(QLatin1String("LastFailureTimestamp"), 0);
    if (timestamp > 0) {
        if (QDateTime::currentSecsSinceEpoch() - timestamp < 60) {
            return true;
        }
    }

    return false;
}

QString X11Compositor::compositingNotPossibleReason() const
{
    // first off, check whether we figured that we'll crash on detection because of a buggy driver
    KConfigGroup gl_workaround_group(kwinApp()->config(), "Compositing");
    if (gl_workaround_group.readEntry("Backend", "OpenGL") == QLatin1String("OpenGL") && openGLCompositingIsBroken()) {
        return i18n("<b>OpenGL compositing (the default) has crashed KWin in the past.</b><br>"
                    "This was most likely due to a driver bug."
                    "<p>If you think that you have meanwhile upgraded to a stable driver,<br>"
                    "you can reset this protection but <b>be aware that this might result in an immediate crash!</b></p>");
    }

    if (!Xcb::Extensions::self()->isCompositeAvailable() || !Xcb::Extensions::self()->isDamageAvailable()) {
        return i18n("Required X extensions (XComposite and XDamage) are not available.");
    }
    if (!Xcb::Extensions::self()->hasGlx()) {
        return i18n("GLX/OpenGL is not available.");
    }
    return QString();
}

bool X11Compositor::compositingPossible() const
{
    // first off, check whether we figured that we'll crash on detection because of a buggy driver
    KConfigGroup gl_workaround_group(kwinApp()->config(), "Compositing");
    if (gl_workaround_group.readEntry("Backend", "OpenGL") == QLatin1String("OpenGL") && openGLCompositingIsBroken()) {
        qCWarning(KWIN_CORE) << "Compositing disabled: video driver seems unstable. If you think it's a false positive, please try again in a few minutes.";
        return false;
    }

    if (!Xcb::Extensions::self()->isCompositeAvailable()) {
        qCWarning(KWIN_CORE) << "Compositing disabled: no composite extension available";
        return false;
    }
    if (!Xcb::Extensions::self()->isDamageAvailable()) {
        qCWarning(KWIN_CORE) << "Compositing disabled: no damage extension available";
        return false;
    }
    if (Xcb::Extensions::self()->hasGlx()) {
        return true;
    }
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGLES) {
        return true;
    } else if (qstrcmp(qgetenv("KWIN_COMPOSE"), "O2ES") == 0) {
        return true;
    }
    qCWarning(KWIN_CORE) << "Compositing disabled: no OpenGL support";
    return false;
}

void X11Compositor::createOpenGLSafePoint(OpenGLSafePoint safePoint)
{
    auto group = KConfigGroup(kwinApp()->config(), "Compositing");
    switch (safePoint) {
    case OpenGLSafePoint::PreInit:
        // Explicitly write the failure timestamp so that if we crash during
        // OpenGL init, we know we should not try again.
        group.writeEntry(QLatin1String("LastFailureTimestamp"), QDateTime::currentSecsSinceEpoch());
        group.sync();
        // Deliberately continue with PreFrame
        Q_FALLTHROUGH();
    case OpenGLSafePoint::PreFrame:
        if (m_openGLFreezeProtectionThread == nullptr) {
            Q_ASSERT(m_openGLFreezeProtection == nullptr);
            m_openGLFreezeProtectionThread = std::make_unique<QThread>();
            m_openGLFreezeProtectionThread->setObjectName("FreezeDetector");
            m_openGLFreezeProtectionThread->start();
            m_openGLFreezeProtection = std::make_unique<QTimer>();
            m_openGLFreezeProtection->setInterval(15000);
            m_openGLFreezeProtection->setSingleShot(true);
            m_openGLFreezeProtection->start();
            const QString configName = kwinApp()->config()->name();
            m_openGLFreezeProtection->moveToThread(m_openGLFreezeProtectionThread.get());
            connect(
                m_openGLFreezeProtection.get(), &QTimer::timeout, m_openGLFreezeProtection.get(),
                [configName] {
                    auto group = KConfigGroup(KSharedConfig::openConfig(configName), "Compositing");
                    group.writeEntry(QLatin1String("LastFailureTimestamp"), QDateTime::currentSecsSinceEpoch());
                    group.sync();
                    KCrash::setDrKonqiEnabled(false);
                    qFatal("Freeze in OpenGL initialization detected");
                },
                Qt::DirectConnection);
        } else {
            Q_ASSERT(m_openGLFreezeProtection);
            QMetaObject::invokeMethod(m_openGLFreezeProtection.get(), QOverload<>::of(&QTimer::start), Qt::QueuedConnection);
        }
        break;
    case OpenGLSafePoint::PostInit:
        group.deleteEntry(QLatin1String("LastFailureTimestamp"));
        group.sync();
        // Deliberately continue with PostFrame
        Q_FALLTHROUGH();
    case OpenGLSafePoint::PostFrame:
        QMetaObject::invokeMethod(m_openGLFreezeProtection.get(), &QTimer::stop, Qt::QueuedConnection);
        break;
    case OpenGLSafePoint::PostLastGuardedFrame:
        m_openGLFreezeProtectionThread->quit();
        m_openGLFreezeProtectionThread->wait();
        m_openGLFreezeProtectionThread.reset();
        m_openGLFreezeProtection.reset();
        break;
    }
}

}

// included for CompositorSelectionOwner
#include "composite.moc"

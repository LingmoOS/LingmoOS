/*
SPDX-FileCopyrightText: 2004 Chris Howells <howells@kde.org>
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "greeterapp.h"
#include "kscreensaversettingsbase.h"
#include "noaccessnetworkaccessmanagerfactory.h"
#include "powermanagement.h"
#include "shell_integration.h"
#include "wallpaper_integration.h"

#include <config-kscreenlocker.h>
#include <iostream>
#include <kscreenlocker_greet_logging.h>

#include <LayerShellQt/Window>

// KDE
#include <KAuthorized>
#include <KConfigPropertyMap>
#include <KCrash>
#include <KLocalizedContext>
#include <KScreenDpms/Dpms>
#include <KWindowSystem>
#include <PlasmaQuick/QuickViewSharedEngine>

#include <KUser>
// Plasma
#include <KPackage/Package>
#include <KPackage/PackageLoader>
// Qt
#include <QAbstractNativeEventFilter>
#include <QClipboard>
#include <QDBusConnection>
#include <QKeyEvent>
#include <QMimeData>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>
#include <qscreen.h>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickView>

#include <private/qtx11extras_p.h>
// Wayland
#include <wayland-client.h>
#include <wayland-ksld-client-protocol.h>
// X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
//
#include <xcb/xcb.h>

#include "pamauthenticator.h"
#include "pamauthenticators.h"

// this is usable to fake a "screensaver" installation for testing
// *must* be "0" for every public commit!
#define TEST_SCREENSAVER 0

static const QString s_plasmaShellService = QStringLiteral("org.kde.plasmashell");
static const QString s_osdServicePath = QStringLiteral("/org/kde/osdService");
static const QString s_osdServiceInterface = QStringLiteral("org.kde.osdService");

using namespace Qt::Literals;

namespace ScreenLocker
{
// disable DrKonqi as the crash dialog blocks the restart of the locker
void disableDrKonqi()
{
    KCrash::setDrKonqiEnabled(false);
}
// run immediately, before Q_CORE_STARTUP functions
// that would enable drkonqi
Q_CONSTRUCTOR_FUNCTION(disableDrKonqi)

// Verify that a package or its fallback is using the right API
bool verifyPackageApi(const KPackage::Package &package)
{
    if (package.metadata().value("X-Plasma-APIVersion", QStringLiteral("1")).toInt() >= 2) {
        return true;
    }

    if (!package.filePath("lockscreenmainscript").contains(package.path())) {
        // The current package does not contain the lock screen and we are
        // using the fallback package. So check to see if that package has
        // the right version instead.
        if (package.fallbackPackage().metadata().value("X-Plasma-APIVersion", QStringLiteral("1")).toInt() >= 2) {
            return true;
        }
    }

    return false;
}

class FocusOutEventFilter : public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override
    {
        Q_UNUSED(result)
        if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
            return false;
        }
        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
        if ((event->response_type & ~0x80) == XCB_FOCUS_OUT) {
            return true;
        }
        return false;
    }
};

class WallpaperItem : public WallpaperIntegration
{
    Q_OBJECT
public:
    explicit WallpaperItem(QQuickItem *parent = nullptr)
        : WallpaperIntegration(parent)
    {
        setConfig(KScreenSaverSettingsBase::self()->sharedConfig());
        setPluginName(KScreenSaverSettingsBase::self()->wallpaperPluginId());
        init();
    }
};

// App
UnlockApp::UnlockApp(int &argc, char **argv)
    : QGuiApplication(argc, argv)
    , m_resetRequestIgnoreTimer(new QTimer(this))
    , m_delayedLockTimer(nullptr)
    , m_testing(false)
    , m_ignoreRequests(false)
    , m_immediateLock(false)
    , m_graceTime(0)
    , m_noLock(false)
    , m_defaultToSwitchUser(false)
    , m_shellIntegration(new ShellIntegration(this))
{
    auto interactive = std::make_unique<PamAuthenticator>(KSCREENLOCKER_PAM_SERVICE, KUser().loginName());
    std::vector<std::unique_ptr<PamAuthenticator>> noninteractive;
    noninteractive.push_back(std::make_unique<PamAuthenticator>(KSCREENLOCKER_PAM_FINGERPRINT_SERVICE, KUser().loginName(), PamAuthenticator::Fingerprint));
    noninteractive.push_back(std::make_unique<PamAuthenticator>(KSCREENLOCKER_PAM_SMARTCARD_SERVICE, KUser().loginName(), PamAuthenticator::Smartcard));
    m_authenticators = new PamAuthenticators(std::move(interactive), std::move(noninteractive), this);
    initialize();

    if (QX11Info::isPlatformX11()) {
        installNativeEventFilter(new FocusOutEventFilter);
    }
}

UnlockApp::~UnlockApp()
{
    // workaround QTBUG-55460
    // will be fixed when themes port to QQC2
    for (auto view : std::as_const(m_views)) {
        if (QQuickItem *focusItem = view->activeFocusItem()) {
            focusItem->setFocus(false);
        }
    }
    qDeleteAll(m_views);

    if (m_ksldInterface) {
        org_kde_ksld_destroy(m_ksldInterface);
    }
    if (m_display) {
        wl_display_disconnect(m_display);
    }
}

void UnlockApp::initialize()
{
    // set up the request ignore timeout, so that multiple requests to sleep/suspend/shutdown
    // are not processed in quick (and confusing) succession)
    m_resetRequestIgnoreTimer->setSingleShot(true);
    m_resetRequestIgnoreTimer->setInterval(2000);
    connect(m_resetRequestIgnoreTimer, &QTimer::timeout, this, &UnlockApp::resetRequestIgnore);

    KScreenSaverSettingsBase::self()->load();

    setShell(m_shellIntegration->defaultShell());

    // The root of wallpaper packages will be a WallpaperItem we provide the same API via WallpaperIntegration
    // although with most things nooping
    constexpr const char *uri = "org.kde.plasma.plasmoid";
    qmlRegisterType<WallpaperItem>(uri, 2, 0, "WallpaperItem");

    m_wallpaperPackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper"));
    m_wallpaperPackage.setPath(KScreenSaverSettingsBase::self()->wallpaperPluginId());

    const KUser user;
    const QString fullName = user.property(KUser::FullName).toString();

    m_userName = fullName.isEmpty() ? user.loginName() : fullName;
    m_userImage = user.faceIconPath();

    installEventFilter(this);

    QDBusConnection::sessionBus().connect(s_plasmaShellService,
                                          s_osdServicePath,
                                          s_osdServiceInterface,
                                          QStringLiteral("osdProgress"),
                                          this,
                                          SLOT(osdProgress(QString, int, int, QString)));
    QDBusConnection::sessionBus()
        .connect(s_plasmaShellService, s_osdServicePath, s_osdServiceInterface, QStringLiteral("osdText"), this, SLOT(osdText(QString, QString)));

    connect(PowerManagement::instance(), &PowerManagement::canSuspendChanged, this, &UnlockApp::updateCanSuspend);
    connect(PowerManagement::instance(), &PowerManagement::canHibernateChanged, this, &UnlockApp::updateCanHibernate);
}

QWindow *UnlockApp::getActiveScreen()
{
    QWindow *activeScreen = nullptr;

    if (m_views.isEmpty()) {
        return activeScreen;
    }

    for (PlasmaQuick::QuickViewSharedEngine *view : std::as_const(m_views)) {
        if (view->geometry().contains(QCursor::pos())) {
            activeScreen = view;
            break;
        }
    }
    if (!activeScreen) {
        activeScreen = m_views.first();
    }

    return activeScreen;
}

PlasmaQuick::SharedQmlEngine *UnlockApp::loadWallpaperPlugin(PlasmaQuick::QuickViewSharedEngine *view)
{
    if (!m_wallpaperPackage.isValid()) {
        qCWarning(KSCREENLOCKER_GREET) << "Error loading the wallpaper, no valid package loaded";
        return nullptr;
    }

    auto qmlObject = new PlasmaQuick::SharedQmlEngine(view);
    qmlObject->setInitializationDelayed(true);
    qmlObject->setSource(QUrl::fromLocalFile(m_wallpaperPackage.filePath("mainscript")));
    view->setProperty("wallpaperGraphicsObject", QVariant::fromValue(qmlObject));

    auto item = qobject_cast<WallpaperItem *>(qmlObject->rootObject());
    if (item) {
        qmlObject->rootContext()->setContextProperty(QStringLiteral("wallpaper"), item);
        view->rootContext()->setContextProperty(QStringLiteral("wallpaper"), item);
        view->rootContext()->setContextProperty(QStringLiteral("wallpaperIntegration"), item);
    } else {
        qCWarning(KSCREENLOCKER_GREET) << "Root item not a WallpaperItem";
    }
    return qmlObject;
}

void UnlockApp::setWallpaperItemProperties(PlasmaQuick::SharedQmlEngine *wallpaperObject, PlasmaQuick::QuickViewSharedEngine *view)
{
    if (!wallpaperObject) {
        return;
    }

    auto item = qobject_cast<QQuickItem *>(wallpaperObject->rootObject());
    if (!item) {
        qCWarning(KSCREENLOCKER_GREET) << "Wallpaper needs to be a QtQuick Item";
        return;
    }
    item->setParentItem(view->rootObject());
    item->setZ(-1000);

    // set anchors
    QQmlExpression expr(wallpaperObject->engine()->rootContext(), item, QStringLiteral("parent"));
    QQmlProperty prop(item, QStringLiteral("anchors.fill"));
    prop.write(expr.evaluate());
}

void UnlockApp::initialViewSetup()
{
    qmlRegisterUncreatableType<PamAuthenticator>("org.kde.kscreenlocker",
                                                 1,
                                                 0,
                                                 "Authenticator",
                                                 QStringLiteral("authenticators must be obtained from the context"));
    qmlRegisterUncreatableType<PamAuthenticators>("org.kde.kscreenlocker",
                                                  1,
                                                  0,
                                                  "Authenticators",
                                                  QStringLiteral("authenticators must be obtained from the context"));
    for (QScreen *screen : screens()) {
        handleScreen(screen);
    }
    connect(this, &UnlockApp::screenAdded, this, &UnlockApp::handleScreen);
}

void UnlockApp::handleScreen(QScreen *screen)
{
    if (screen->geometry().isNull()) {
        return;
    }
    auto *view = createViewForScreen(screen);
    m_views << view;
    connect(this, &QGuiApplication::screenRemoved, view, [this, view, screen](QScreen *removedScreen) {
        if (removedScreen != screen) {
            return;
        }
        m_views.removeOne(view);
        delete view;
    });
}

PlasmaQuick::QuickViewSharedEngine *UnlockApp::createViewForScreen(QScreen *screen)
{
    // create the view
    auto *view = new PlasmaQuick::QuickViewSharedEngine();

    view->setColor(Qt::black);
    view->setScreen(screen);
    view->setGeometry(screen->geometry());

    connect(screen, &QScreen::geometryChanged, view, [view](const QRect &geo) {
        view->setGeometry(geo);
    });

    view->engine()->setProperty("_kirigamiTheme", QStringLiteral("KirigamiPlasmaStyle"));
    view->engine()->rootContext()->setContextObject(new KLocalizedContext(view->engine().get()));
    auto oldFactory = view->engine()->networkAccessManagerFactory();
    view->engine()->setNetworkAccessManagerFactory(nullptr);
    delete oldFactory;
    view->engine()->setNetworkAccessManagerFactory(new NoAccessNetworkAccessManagerFactory);

    if (!m_testing) {
        if (QX11Info::isPlatformX11()) {
            view->setFlags(Qt::X11BypassWindowManagerHint);
        } else {
            view->setFlags(Qt::FramelessWindowHint);
        }
    }

    if (m_ksldInterface) {
        view->create();
        org_kde_ksld_x11window(m_ksldInterface, view->winId());
        wl_display_flush(m_display);
    }

    // engine stuff
    QQmlContext *context = view->engine()->rootContext();
    connect(view->engine().get(), &QQmlEngine::quit, this, [this]() {
        if (m_authenticators->isUnlocked()) {
            std::cout << "Unlocked" << std::endl;
            QCoreApplication::quit();
        } else {
            qCWarning(KSCREENLOCKER_GREET) << "Greeter tried to quit without being unlocked";
        }
    });

    context->setContextProperty(QStringLiteral("kscreenlocker_userName"), m_userName);
    context->setContextProperty(QStringLiteral("kscreenlocker_userImage"), m_userImage);
    context->setContextProperty(QStringLiteral("authenticator"), m_authenticators);
    context->setContextProperty(QStringLiteral("org_kde_plasma_screenlocker_greeter_interfaceVersion"), 2);
    context->setContextProperty(QStringLiteral("org_kde_plasma_screenlocker_greeter_view"), view);
    context->setContextProperty(QStringLiteral("defaultToSwitchUser"), m_defaultToSwitchUser);
    context->setContextProperty(QStringLiteral("config"), m_shellIntegration->configuration());

    auto wallpaperObj = loadWallpaperPlugin(view);
    if (auto object = view->property("wallpaperGraphicsObject").value<PlasmaQuick::SharedQmlEngine *>()) {
        // initialize with our size to avoid as much resize events as possible
        object->completeInitialization({
            {QStringLiteral("width"), view->width()},
            {QStringLiteral("height"), view->height()},
        });
    }

    view->setSource(m_mainQmlPath);
    // on error, load the fallback lockscreen to not lock the user out of the system
    if (view->status() != QQmlComponent::Ready) {
        static const QUrl fallbackUrl(QUrl(QStringLiteral("qrc:/fallbacktheme/LockScreen.qml")));

        qCWarning(KSCREENLOCKER_GREET) << "Failed to load lockscreen QML, falling back to built-in locker";
        for (const auto &error : view->errors()) {
            qCWarning(KSCREENLOCKER_GREET) << error;
        }

        m_mainQmlPath = fallbackUrl;
        view->setSource(fallbackUrl);

        if (view->status() != QQmlComponent::Ready) {
            qCWarning(KSCREENLOCKER_GREET) << "Failed to load the fallback lockscreen QML, something went really wrong! Terminating...";
            for (const auto &error : view->errors()) {
                qCWarning(KSCREENLOCKER_GREET) << error;
            }
            std::terminate();
        }
    }
    view->setResizeMode(PlasmaQuick::QuickViewSharedEngine::SizeRootObjectToView);

    // we need to set this wallpaper properties separately after the lockscreen QML is loaded
    // this is because we need to anchor to the view that gets loaded
    setWallpaperItemProperties(wallpaperObj, view);

    QQmlProperty lockProperty(view->rootObject(), QStringLiteral("locked"));
    lockProperty.write(m_immediateLock || (!m_noLock && !m_delayedLockTimer));

    QQmlProperty sleepProperty(view->rootObject(), QStringLiteral("suspendToRamSupported"));
    sleepProperty.write(PowerManagement::instance()->canSuspend());
    if (view->rootObject() && view->rootObject()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("suspendToRam()").constData()) != -1) {
        connect(view->rootObject(), SIGNAL(suspendToRam()), SLOT(suspendToRam()));
    }

    QQmlProperty hibernateProperty(view->rootObject(), QStringLiteral("suspendToDiskSupported"));
    hibernateProperty.write(PowerManagement::instance()->canHibernate());
    if (view->rootObject() && view->rootObject()->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("suspendToDisk()").constData()) != -1) {
        connect(view->rootObject(), SIGNAL(suspendToDisk()), SLOT(suspendToDisk()));
    }

    // verify that the engine's controller didn't change
    Q_ASSERT(dynamic_cast<NoAccessNetworkAccessManagerFactory *>(view->engine()->networkAccessManagerFactory()));

    if (KWindowSystem::isPlatformWayland()) {
        if (auto layerShellWindow = LayerShellQt::Window::get(view)) {
            layerShellWindow->setExclusiveZone(-1);
            layerShellWindow->setLayer(LayerShellQt::Window::LayerOverlay);
            layerShellWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
        }
    }

    // showFullScreen is implicit on X11 (through geometry and hints) and Wayland (layer-shell)
    view->show();
    view->raise();

    auto onFrameSwapped = [this, view] {
        markViewsAsVisible(view);
    };
    connect(view, &QQuickWindow::frameSwapped, this, onFrameSwapped, static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::SingleShotConnection));

    return view;
}

void UnlockApp::markViewsAsVisible(PlasmaQuick::QuickViewSharedEngine *view)
{
    QQmlProperty showProperty(view->rootObject(), QStringLiteral("viewVisible"));
    showProperty.write(true);
    // random state update, actually rather required on init only
    QMetaObject::invokeMethod(this, "getFocus", Qt::QueuedConnection);

    auto mime1 = new QMimeData;
    // Effectively we want to clear the clipboard
    // however some clipboard managers (like klipper with it's default settings)
    // will prevent an empty clipboard
    // we need some non-empty non-text mimeData to replace the clipboard so we don't leak real data to a user pasting into the text field
    // as the clipboard is cleared on close, klipper will then put the original text back when we exit
    mime1->setData(QStringLiteral("x-kde-lockscreen"), QByteArrayLiteral("empty"));
    // ownership is transferred
    QGuiApplication::clipboard()->setMimeData(mime1, QClipboard::Clipboard);

    auto mime2 = new QMimeData;
    mime2->setData(QStringLiteral("x-kde-lockscreen"), QByteArrayLiteral("empty"));
    QGuiApplication::clipboard()->setMimeData(mime2, QClipboard::Selection);
}

void UnlockApp::getFocus()
{
    QWindow *activeScreen = getActiveScreen();

    if (!activeScreen) {
        return;
    }
    // this loop is required to make the qml/graphicsscene properly handle the shared keyboard input
    // ie. "type something into the box of every greeter"
    for (PlasmaQuick::QuickViewSharedEngine *view : std::as_const(m_views)) {
        if (!m_testing) {
            view->setKeyboardGrabEnabled(true); // TODO - check whether this still works in master!
        }
    }
    // activate window and grab input to be sure it really ends up there.
    // focus setting is still required for proper internal QWidget state (and eg. visual reflection)
    if (!m_testing) {
        activeScreen->setKeyboardGrabEnabled(true); // TODO - check whether this still works in master!
    }
    activeScreen->requestActivate();
}

void UnlockApp::graceLockEnded()
{
    m_authenticators->setGraceLocked(false);

    delete m_delayedLockTimer;
    m_delayedLockTimer = nullptr;

    for (PlasmaQuick::QuickViewSharedEngine *view : std::as_const(m_views)) {
        QQmlProperty lockProperty(view->rootObject(), QStringLiteral("locked"));
        lockProperty.write(true);
    }
}

void UnlockApp::resetRequestIgnore()
{
    m_ignoreRequests = false;
}

void UnlockApp::suspendToRam()
{
    if (m_ignoreRequests) {
        return;
    }

    m_ignoreRequests = true;
    m_resetRequestIgnoreTimer->start();
    m_authenticators->cancel();

    PowerManagement::instance()->suspend();
}

void UnlockApp::suspendToDisk()
{
    if (m_ignoreRequests) {
        return;
    }

    m_ignoreRequests = true;
    m_resetRequestIgnoreTimer->start();
    m_authenticators->cancel();

    PowerManagement::instance()->hibernate();
}

void UnlockApp::setTesting(bool enable)
{
    qCDebug(KSCREENLOCKER_GREET) << "Testing mode enabled:" << enable;

    m_testing = enable;
    if (m_views.isEmpty()) {
        return;
    }
    if (enable) {
        // remove bypass window manager hint
        for (PlasmaQuick::QuickViewSharedEngine *view : std::as_const(m_views)) {
            view->setFlags(view->flags() & ~Qt::X11BypassWindowManagerHint);
        }
    } else {
        for (PlasmaQuick::QuickViewSharedEngine *view : std::as_const(m_views)) {
            view->setFlags(view->flags() | Qt::X11BypassWindowManagerHint);
        }
    }
}

void UnlockApp::setShell(const QString &shell)
{
    m_packageName = shell;
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Shell"));

    if (!m_packageName.isEmpty()) {
        package.setPath(m_packageName);
    }

    if (!verifyPackageApi(package)) {
        qCWarning(KSCREENLOCKER_GREET) << "Lockscreen QML outdated, falling back to default";
        package.setPath(QStringLiteral("org.kde.plasma.desktop"));
    }

    m_mainQmlPath = package.fileUrl("lockscreenmainscript");

    m_shellIntegration->setPackage(package);
    m_shellIntegration->setConfig(KScreenSaverSettingsBase::self()->sharedConfig());
    m_shellIntegration->init();
}

void UnlockApp::setImmediateLock(bool immediate)
{
    m_immediateLock = immediate;
}

void UnlockApp::lockImmediately()
{
    setImmediateLock(true);
    graceLockEnded();
}

bool UnlockApp::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != this && event->type() == QEvent::Show) {
        PlasmaQuick::QuickViewSharedEngine *view = nullptr;
        for (PlasmaQuick::QuickViewSharedEngine *v : std::as_const(m_views)) {
            if (v == obj) {
                view = v;
                break;
            }
        }
        if (view && view->winId() && QX11Info::isPlatformX11()) {
            // showing greeter view window, set property
            static Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
            XChangeProperty(QX11Info::display(), view->winId(), tag, tag, 32, PropModeReplace, nullptr, 0);
        }
        // no further processing
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress && QX11Info::isPlatformX11()) {
        if (getActiveScreen()) {
            getActiveScreen()->requestActivate();
        }
        return false;
    }

    if (event->type() == QEvent::KeyPress) { // react if saver is visible
        shareEvent(event, qobject_cast<PlasmaQuick::QuickViewSharedEngine *>(obj));
        return false; // we don't care
    } else if (event->type() == QEvent::KeyRelease) { // conditionally reshow the saver
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        if (ke->key() != Qt::Key_Escape) {
            shareEvent(event, qobject_cast<PlasmaQuick::QuickViewSharedEngine *>(obj));
            return false; // irrelevant
        } else {
            auto dpms = new KScreen::Dpms(this);
            if (dpms->isSupported()) {
                connect(dpms, &KScreen::Dpms::hasPendingChangesChanged, this, [dpms](bool hasPendingChanges) {
                    if (!hasPendingChanges) {
                        dpms->deleteLater();
                    }
                });
                dpms->switchMode(KScreen::Dpms::Off);
            } else {
                dpms->deleteLater();
            }
        }
        return true; // don't pass
    }

    return false;
}

/*
 * This function forwards an event from one greeter window to all others
 * It's used to have the keyboard operate on all greeter windows (on every screen)
 * at once so that the user gets visual feedback on the screen he's looking at -
 * even if the focus is actually on a powered off screen.
 */

void UnlockApp::shareEvent(QEvent *e, PlasmaQuick::QuickViewSharedEngine *from)
{
    // from can be NULL any time (because the parameter is passed as qobject_cast)
    // m_views.contains(from) is atm. supposed to be true but required if any further
    // QQuickView are added (which are not part of m_views)
    // this makes "from" an optimization (nullptr check aversion)
    if (from && m_views.contains(from)) {
        // NOTICE any recursion in the event sharing will prevent authentication on multiscreen setups!
        // Any change in regarded event processing shall be tested thoroughly!
        removeEventFilter(this); // prevent recursion!
        const bool accepted = e->isAccepted(); // store state
        for (PlasmaQuick::QuickViewSharedEngine *view : std::as_const(m_views)) {
            if (view != from) {
                QCoreApplication::sendEvent(view, e);
                e->setAccepted(accepted);
            }
        }
        installEventFilter(this);
    }
}

void UnlockApp::setGraceTime(int milliseconds)
{
    m_graceTime = milliseconds;
    if (milliseconds < 0 || m_delayedLockTimer || m_noLock || m_immediateLock) {
        return;
    }
    m_authenticators->setGraceLocked(true);
    m_delayedLockTimer = new QTimer(this);
    m_delayedLockTimer->setSingleShot(true);
    connect(m_delayedLockTimer, &QTimer::timeout, this, &UnlockApp::graceLockEnded);
    m_delayedLockTimer->start(m_graceTime);
}

void UnlockApp::setNoLock(bool noLock)
{
    m_noLock = noLock;
}

void UnlockApp::setDefaultToSwitchUser(bool defaultToSwitchUser)
{
    m_defaultToSwitchUser = defaultToSwitchUser;
}

void UnlockApp::setKsldSocket(int socket)
{
    m_display = wl_display_connect_to_fd(socket);
    auto socketnotifier = new QSocketNotifier(socket, QSocketNotifier::Read, this);
    connect(socketnotifier, &QSocketNotifier::activated, this, [this] {
        wl_display_dispatch(m_display);
    });
    auto registry = wl_display_get_registry(m_display);
    auto globalAdded = [](void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
        Q_UNUSED(version)
        if (interface != "org_kde_ksld"_ba) {
            return;
        }
        auto self = static_cast<UnlockApp *>(data);
        // bind version 1 as we dropped all the V2 features
        self->m_ksldInterface = static_cast<org_kde_ksld *>(wl_registry_bind(registry, name, &org_kde_ksld_interface, 1));
        for (auto v : std::as_const(self->m_views)) {
            org_kde_ksld_x11window(self->m_ksldInterface, v->winId());
            wl_display_flush(self->m_display);
        }
    };
    auto noopGlobalRemove = [](void *, struct wl_registry *, uint32_t) {};
    static const wl_registry_listener registryListener = wl_registry_listener{globalAdded, noopGlobalRemove};
    wl_registry_add_listener(registry, &registryListener, this);
    wl_display_flush(m_display);
}

void UnlockApp::osdProgress(const QString &icon, int percent, int maximumPercent, const QString &additionalText)
{
    for (auto v : std::as_const(m_views)) {
        auto osd = v->rootObject()->findChild<QQuickItem *>(QStringLiteral("onScreenDisplay"));
        if (!osd) {
            continue;
        }
        // Update max value first to prevent value from being clamped
        osd->setProperty("osdMaxValue", maximumPercent);
        osd->setProperty("osdValue", percent);
        osd->setProperty("osdAdditionalText", additionalText);
        osd->setProperty("showingProgress", true);
        osd->setProperty("icon", icon);
        QMetaObject::invokeMethod(osd, "show");
    }
}

void UnlockApp::osdText(const QString &icon, const QString &additionalText)
{
    for (auto v : std::as_const(m_views)) {
        auto osd = v->rootObject()->findChild<QQuickItem *>(QStringLiteral("onScreenDisplay"));
        if (!osd) {
            continue;
        }
        osd->setProperty("showingProgress", false);
        osd->setProperty("osdValue", additionalText);
        osd->setProperty("icon", icon);
        QMetaObject::invokeMethod(osd, "show");
    }
}

void UnlockApp::updateCanSuspend()
{
    for (auto it = m_views.constBegin(), end = m_views.constEnd(); it != end; ++it) {
        QQmlProperty sleepProperty((*it)->rootObject(), QStringLiteral("suspendToRamSupported"));
        sleepProperty.write(PowerManagement::instance()->canSuspend());
    }
}

void UnlockApp::updateCanHibernate()
{
    for (auto it = m_views.constBegin(), end = m_views.constEnd(); it != end; ++it) {
        QQmlProperty hibernateProperty((*it)->rootObject(), QStringLiteral("suspendToDiskSupported"));
        hibernateProperty.write(PowerManagement::instance()->canHibernate());
    }
}

} // namespace

#include "greeterapp.moc"

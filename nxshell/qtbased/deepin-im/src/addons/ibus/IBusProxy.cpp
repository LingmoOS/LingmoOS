// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "IBusProxy.h"

#include "IBUSIface.h"
#include "IBUSPortIface.h"
#include "addons/waylandserver/WaylandServer_public.h"
#include "addons/wlfrontend/WLFrontend_public.h"
#include "dimcore/Dim.h"
#include "dimcore/InputContext.h"
#include "ibustypes.h"
#include "wl/client/Compositor.h"
#include "wl/client/ConnectionBase.h"
#include "wl/client/ConnectionRaw.h"
#include "wl/client/ZwpInputMethodV2.h"
#include "wladdonsbase/InputMethodContextV1.h"
#include "wladdonsbase/InputMethodV1.h"
#include "wladdonsbase/Keyboard.h"

#include <gio/gsettingsschema.h>

#include <functional>

#include <signal.h>

#ifndef IBUS_RELEASE_MASK
#  define IBUS_RELEASE_MASK (1 << 30)
#  define IBUS_SHIFT_MASK (1 << 0)
#  define IBUS_CONTROL_MASK (1 << 2)
#  define IBUS_MOD1_MASK (1 << 3)
#  define IBUS_META_MASK (1 << 28)
#endif

static const char *SOCKET_NAME = "dim";

using namespace org::deepin::dim;
WL_ADDONS_BASE_USE_NAMESPACE

DIM_ADDON_FACTORY(DimIBusProxy);

static const std::string KEYBOARD_PREFIX = "xkb:";

class DimIBusInputContextPrivate
{
public:
    DimIBusInputContextPrivate();

    ~DimIBusInputContextPrivate()
    {
        delete busInterface_;
        delete portalBus_;
        delete dbusConn_;
    }

    static QString getSocketPath();
    QDBusConnection *createConnection();
    void initBus();
    void createBusProxy();

    bool usePortal_;
    bool isValid_;
    bool busConnected_;
    QString ibusService_;
    QDBusConnection *dbusConn_;
    OrgFreedesktopIBusPortalInterface *portalBus_;
    OrgFreedesktopIBusInterface *busInterface_;
    QDBusServiceWatcher serviceWatcher_;
};

DimIBusInputContextPrivate::DimIBusInputContextPrivate()
    : usePortal_(false)
    , isValid_(false)
    , busConnected_(false)
    , dbusConn_(nullptr)
    , portalBus_(nullptr)
    , busInterface_(nullptr)
{
    if (qEnvironmentVariableIsSet("IBUS_USE_PORTAL")) {
        bool ok;
        int enableSync = qEnvironmentVariableIntValue("IBUS_USE_PORTAL", &ok);
        if (ok && enableSync == 1)
            usePortal_ = true;
    }
    if (usePortal_) {
        isValid_ = true;
    } else {
        isValid_ =
            !QStandardPaths::findExecutable(QString::fromLocal8Bit("ibus-daemon"), QStringList())
                 .isEmpty();
    }
    if (!isValid_)
        return;
}

void DimIBusInputContextPrivate::initBus()
{
    dbusConn_ = createConnection();
    createBusProxy();
}

void DimIBusInputContextPrivate::createBusProxy()
{
    if (!dbusConn_ || !dbusConn_->isConnected())
        return;

    ibusService_ = usePortal_ ? QLatin1String("org.freedesktop.portal.IBus")
                              : QLatin1String("org.freedesktop.IBus");
    if (usePortal_) {
        portalBus_ = new OrgFreedesktopIBusPortalInterface((ibusService_),
                                                           QLatin1String("/org/freedesktop/IBus"),
                                                           *dbusConn_);
        if (!portalBus_->isValid()) {
            qWarning("ibus proxy: invalid portal bus");
            return;
        }
    } else {
        busInterface_ = new OrgFreedesktopIBusInterface(ibusService_,
                                                        QLatin1String("/org/freedesktop/IBus"),
                                                        *dbusConn_);
        if (!busInterface_->isValid()) {
            qWarning("ibus proxy: invalid bus.");
            return;
        }
    }

    serviceWatcher_.removeWatchedService(ibusService_);
    serviceWatcher_.setConnection(*dbusConn_);
    serviceWatcher_.addWatchedService(ibusService_);

    busConnected_ = true;
}

QString DimIBusInputContextPrivate::getSocketPath()
{
    QByteArray display;
    QByteArray displayNumber = "0";
    bool isWayland = false;

    if (qEnvironmentVariableIsSet("IBUS_ADDRESS_FILE")) {
        QByteArray path = qgetenv("IBUS_ADDRESS_FILE");
        return QString::fromLocal8Bit(path);
    } else if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")) {
        display = qgetenv("WAYLAND_DISPLAY");
        isWayland = true;
    } else {
        display = qgetenv("DISPLAY");
    }
    QByteArray host = "unix";

    if (isWayland) {
        displayNumber = display;
    } else {
        int pos = display.indexOf(':');
        if (pos > 0)
            host = display.left(pos);
        ++pos;
        int pos2 = display.indexOf('.', pos);
        displayNumber = pos2 > 0 ? display.mid(pos, pos2 - pos) : display.mid(pos);
    }

    displayNumber = SOCKET_NAME;

    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QLatin1String("/ibus/bus/") + QLatin1String(QDBusConnection::localMachineId())
        + QLatin1Char('-') + QString::fromLocal8Bit(host) + QLatin1Char('-')
        + QString::fromLocal8Bit(displayNumber);
}

QDBusConnection *DimIBusInputContextPrivate::createConnection()
{
    if (usePortal_)
        return new QDBusConnection(QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                                                 QLatin1String("DimIBusProxy")));
    QFile file(getSocketPath());

    if (!file.open(QFile::ReadOnly))
        return 0;

    QByteArray address;
    int pid = -1;

    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        if (line.startsWith('#'))
            continue;

        if (line.startsWith("IBUS_ADDRESS="))
            address = line.mid(sizeof("IBUS_ADDRESS=") - 1);
        if (line.startsWith("IBUS_DAEMON_PID="))
            pid = line.mid(sizeof("IBUS_DAEMON_PID=") - 1).toInt();
    }

    if (address.isEmpty() || pid < 0 || kill(pid, 0) != 0)
        return 0;

    return new QDBusConnection(
        QDBusConnection::connectToBus(QString::fromLatin1(address), QLatin1String("DimIBusProxy")));
}

DimIBusProxy::DimIBusProxy(Dim *dim)
    : ProxyAddon(dim, "ibus", "ibus")
    , d(new DimIBusInputContextPrivate())
    , useSyncMode_(false)
    , daemonLaunched_(false)
{
    Addon *wls = dim->addons().at("waylandserver");
    auto wl = waylandserver::getServer(wls);
    wl_ = wl;
    surface_ = waylandserver::getSurface(wls);

    auto im = wl_->inputMethodV1();

    if (im) {
        im->inputMethodContextV1ForwardKeyCallback_ = std::bind(&DimIBusProxy::forwardKey,
                                                                this,
                                                                std::placeholders::_1,
                                                                std::placeholders::_2);
        im->inputMethodContextV1CommitCallback_ =
            std::bind(&DimIBusProxy::commit, this, std::placeholders::_1, std::placeholders::_2);
        im->inputMethodContextV1PreeditCallback_ = std::bind(&DimIBusProxy::preedit,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3);
        im->inputPanelV1CreateCallback_ = std::bind(&DimIBusProxy::panelCreate, this);
        im->inputPanelV1DestoryCallback_ = std::bind(&DimIBusProxy::panelDestroy, this);
    }

    // Stop restarting the ibus daemon if it starts crashing very frequently
    daemonCrashTimer_.setInterval(20000);
    daemonCrashTimer_.setSingleShot(true);
    connect(&daemonCrashTimer_, &QTimer::timeout, this, [this] {
        daemonCrashes_ = 0;
    });

    qDBusRegisterMetaType<IBusText>();
    qDBusRegisterMetaType<IBusEngineDesc>();

    auto schema = g_settings_schema_source_lookup(g_settings_schema_source_get_default(),
                                                  "org.freedesktop.ibus.general",
                                                  TRUE);

    gsettings_.reset();
    if (schema) {
        gsettings_.reset(g_settings_new_full(schema, nullptr, nullptr));
        g_settings_schema_unref(schema);
    }

    if (qEnvironmentVariableIsSet("IBUS_ENABLE_SYNC_MODE")) {
        bool ok;
        int enableSync = qEnvironmentVariableIntValue("IBUS_ENABLE_SYNC_MODE", &ok);
        if (ok && enableSync == 1)
            useSyncMode_ = true;
    }

    if (!d->usePortal_) {
        QString socketPath = DimIBusInputContextPrivate::getSocketPath();
        QFile file(socketPath);
        if (file.open(QFile::ReadOnly)) {
            qDebug() << "socketWatcher.addPath" << socketPath;
            // If restart ibus-daemon,
            // the applications could run before ibus-daemon runs.
            // We watch the getSocketPath() to get the launching ibus-daemon.
            socketWatcher_.addPath(socketPath);
            connect(&socketWatcher_,
                    &QFileSystemWatcher::fileChanged,
                    this,
                    &DimIBusProxy::socketChanged);
        }
        timer_.setSingleShot(true);
        connect(&timer_, &QTimer::timeout, this, &DimIBusProxy::connectToBus);
    }

    connect(&d->serviceWatcher_,
            &QDBusServiceWatcher::serviceRegistered,
            this,
            &DimIBusProxy::busRegistered);
    connect(&d->serviceWatcher_,
            &QDBusServiceWatcher::serviceUnregistered,
            this,
            &DimIBusProxy::busUnregistered);
    launchDaemon();
}

void DimIBusProxy::forwardKey(uint32_t keycode, uint32_t state)
{
    auto *ic = getFocusedIC(focusedId_);
    if (!ic) {
        return;
    }

    ic->forwardKey(keycode, state);
}

void DimIBusProxy::commit(uint32_t serial, const char *text)
{
    auto *ic = getFocusedIC(focusedId_);
    if (!ic) {
        return;
    }

    if (text) {
        ic->commitString(text);
    }
    ic->commit();
}

void DimIBusProxy::preedit(uint32_t serial, const char *text, const char *commit)
{
    auto *ic = getFocusedIC(focusedId_);
    if (!ic) {
        return;
    }

    if (text) {
        ic->updatePreedit(text, 0, 0);
    }

    ic->commit();
}

void DimIBusProxy::panelCreate()
{
    popup_.reset();

    auto *ic = getFocusedIC(focusedId_);
    assert(ic);

    auto *im = wlfrontend::getInputMethodV2(ic);
    auto *pop = im->get_input_popup_surface(surface_);
    if (!pop) {
        qWarning() << "failed to get popup surface";
    }

    popup_.reset(new InputPopupSurfaceV2(pop));
}

void DimIBusProxy::panelDestroy()
{
    popup_.reset();
}

void DimIBusProxy::connectToBus()
{
    qDebug() << "connect to ibus";

    d->initBus();
    initEngines();

    if (!d->usePortal_ && socketWatcher_.files().size() == 0) {
        socketWatcher_.addPath(DimIBusInputContextPrivate::getSocketPath());
    }
}

void DimIBusProxy::socketChanged(const QString &str)
{
    Q_UNUSED(str);
    qDebug() << "socketChanged";

    for (auto it = iBusICMap_.begin(); it != iBusICMap_.end(); ++it) {
        if (it.value()) {
            disconnect(it.value().get());
        }
    }
    if (d->busInterface_ && d->busInterface_->isValid())
        disconnect(d->busInterface_);
    if (d->dbusConn_)
        d->dbusConn_->disconnectFromBus(QLatin1String("DimIBusProxy"));

    timer_.stop();
    timer_.start(100);
}

void DimIBusProxy::busRegistered(const QString &str)
{
    qDebug() << "bus registered";
    Q_UNUSED(str);
    if (d->usePortal_) {
        connectToBus();
    }
}

void DimIBusProxy::busUnregistered(const QString &str)
{
    qDebug() << "bus unregistered";
    Q_UNUSED(str);
    d->busConnected_ = false;
}

DimIBusProxy::~DimIBusProxy()
{
    stopInputMethod();
    iBusICMap_.clear();
}

void DimIBusProxy::initInputMethods()
{
    Q_EMIT addonInitFinished(this);

    std::unique_ptr<gchar *, Deleter<g_strfreev>> list(nullptr);
    if (gsettings_ && gsettings_.get()) {
        list.reset(g_settings_get_strv(gsettings_.get(), "preload-engines"));
    }

    if (!list) {
        return;
    }

    std::vector<std::string> activeInputMethods;
    auto **listPtr = list.get();
    for (int i = 0; listPtr[i] != nullptr; i++) {
        std::string key = listPtr[i];
        if (shouldBeIgnored(key)) {
            continue;
        }

        activeInputMethods.emplace_back(key);
    }
    updateActiveInputMethods(activeInputMethods);
}

const QList<InputMethodEntry> &DimIBusProxy::getInputMethods()
{
    return inputMethods_;
}

void DimIBusProxy::focusIn(uint32_t id)
{
    auto im = wl_->inputMethodV1();

    if (!im) {
        return;
    }

    focusedId_ = id;

    im->sendActivate();
}

void DimIBusProxy::focusOut(uint32_t id)
{
    if (focusedId_ != id) {
        return;
    }

    auto im = wl_->inputMethodV1();

    if (!im) {
        return;
    }

    im->sendDeactivate();
}

void DimIBusProxy::destroyed(uint32_t id)
{
    if (isICDBusInterfaceValid(id)) {
        iBusICMap_[id]->Destroy();
    }
}

void DimIBusProxy::done() { }

void DimIBusProxy::contentType(uint32_t hint, uint32_t purpose)
{
    if (!daemonLaunched_) {
        return;
    }

    auto *context = wl_->inputMethodContextV1();
    if (!context) {
        return;
    }

    context->sendContentType(hint, purpose);
}

void DimIBusProxy::setCurrentIM(const std::string &im)
{
    if (!d->busConnected_ || d->usePortal_ || !d->busInterface_) {
        return;
    }

    QDBusPendingReply<> reply = d->busInterface_->SetGlobalEngine(QString::fromStdString(im));
    reply.waitForFinished();

    if (reply.isError()) {
        qWarning() << "set global engine error" << reply.error();
    }
}

bool DimIBusProxy::keyEvent([[maybe_unused]] const InputMethodEntry &entry,
                            InputContextKeyEvent &keyEvent)
{
    auto id = keyEvent.ic()->id();

    if (focusedId_ != id || !daemonLaunched_) {
        return false;
    }

    auto *context = wl_->inputMethodContextV1();
    if (!context) {
        return false;
    }

    context->sendKey(keyEvent.keycode(), keyEvent.isRelease());

    return true;
}

void DimIBusProxy::cursorRectangleChangeEvent(InputContextCursorRectChangeEvent &event)
{
    if (!daemonLaunched_) {
        return;
    }

    auto id = event.ic()->id();

    if (!isICDBusInterfaceValid(id)) {
        return;
    }

    iBusICMap_[id]->SetCursorLocationRelative(event.x, event.y, event.w, event.h);
}

void DimIBusProxy::updateSurroundingText(InputContextEvent &event)
{
    if (!daemonLaunched_) {
        return;
    }

    auto &surroundingText = event.ic()->surroundingText();

    auto *context = wl_->inputMethodContextV1();
    if (!context) {
        return;
    }

    context->sendSurroundingText(surroundingText.text().toUtf8().data(),
                                 surroundingText.cursor(),
                                 surroundingText.anchor());
}

void DimIBusProxy::initEngines()
{
    QList<InputMethodEntry> inputMethods;
    for (auto &engine : listEngines()) {
        std::string imEntryName = engine.engine_name.toStdString();

        // 过滤掉键盘布局
        if (shouldBeIgnored(imEntryName)) {
            continue;
        }

        inputMethods.append(InputMethodEntry(key(),
                                             imEntryName,
                                             engine.longname.toStdString(),
                                             engine.description.toStdString(),
                                             engine.symbol.toStdString(),
                                             engine.icon.toStdString()));
    }

    inputMethods_.swap(inputMethods);
    Q_EMIT addonInitFinished(this);
}

void DimIBusProxy::globalEngineChanged(const QString &engineName)
{
    if (d->usePortal_ || !d->busConnected_) {
        return;
    }

    initEngines();
}

QList<IBusEngineDesc> DimIBusProxy::listEngines()
{
    QList<IBusEngineDesc> engines;

    if (d->usePortal_) {
        return engines;
    }

    if (!d->busConnected_) {
        qWarning() << "IBus is not connected!";
        return engines;
    }

    QDBusPendingReply<QVariantList> reply = d->busInterface_->ListEngines();
    reply.waitForFinished();

    if (reply.isError()) {
        qWarning() << "Bus::listEngines:" << reply.error();
        return engines;
    }

    QVariantList ret = reply.value();
    for (int i = 0; i < ret.size(); i++) {
        IBusEngineDesc e;
        ret.at(i).value<QDBusArgument>() >> e;
        engines << e;
    }

    return engines;
}

bool DimIBusProxy::shouldBeIgnored(const std::string &uniqueName) const
{
    return std::mismatch(KEYBOARD_PREFIX.begin(), KEYBOARD_PREFIX.end(), uniqueName.begin()).first
        == KEYBOARD_PREFIX.end();
}

void DimIBusProxy::launchDaemon()
{
    if (!isExecutableExisted(QStringLiteral("ibus-daemon"))) {
        qDebug() << "can not find ibus daemon executable, maybe it should be installed";
        return;
    }

    if (ibusDaemonProc_) {
        stopInputMethod();
    }

    ibusDaemonProc_ = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("WAYLAND_DISPLAY", SOCKET_NAME);

    ibusDaemonProc_->setProgram(QStringLiteral("/usr/libexec/ibus-ui-gtk3"));
    ibusDaemonProc_->setArguments(QStringList{ QStringLiteral("-i"),
                                               QStringLiteral("-d"),
                                               QStringLiteral("-g"),
                                               QStringLiteral("--xim --panel disable") });
    ibusDaemonProc_->setProcessEnvironment(env);
    ibusDaemonProc_->setStandardOutputFile("/tmp/ibus.log");
    ibusDaemonProc_->setStandardErrorFile("/tmp/ibus.log");
    ibusDaemonProc_->start();

    connect(ibusDaemonProc_,
            &QProcess::stateChanged,
            this,
            [this](const QProcess::ProcessState &state) {
                daemonLaunched_ = state == QProcess::ProcessState::Running;
            });

    connect(ibusDaemonProc_,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus == QProcess::CrashExit) {
                    qWarning() << "Input Method crashed" << ibusDaemonProc_->program()
                               << ibusDaemonProc_->arguments() << exitCode << exitStatus;

                    daemonCrashes_++;
                    daemonCrashTimer_.start();
                    if (daemonCrashes_ < 5) {
                        launchDaemon();
                    } else {
                        qWarning() << "ibus daemon keeps crashing, please fix"
                                   << ibusDaemonProc_->program() << ibusDaemonProc_->arguments();
                        stopInputMethod();
                    }
                }
            });
}

void DimIBusProxy::stopInputMethod()
{
    if (!ibusDaemonProc_) {
        return;
    }

    disconnect(ibusDaemonProc_, nullptr, this, nullptr);

    ibusDaemonProc_->terminate();
    if (!ibusDaemonProc_->waitForFinished()) {
        ibusDaemonProc_->kill();
        ibusDaemonProc_->waitForFinished();
    }

    ibusDaemonProc_->deleteLater();
    ibusDaemonProc_ = nullptr;
}

InputContext *DimIBusProxy::getFocusedIC(uint32_t id) const
{
    return dim()->getFocusedIC(id);
}

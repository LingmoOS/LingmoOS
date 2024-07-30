/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "backendmanager_p.h"

#include "abstractbackend.h"
#include "backendinterface.h"
#include "configmonitor.h"
#include "configserializer_p.h"
#include "getconfigoperation.h"
#include "kscreen_debug.h"
#include "log.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QThread>
#include <QtGui/private/qtx11extras_p.h>

#include <memory>

using namespace KScreen;

const int BackendManager::sMaxCrashCount = 4;

BackendManager *BackendManager::sInstance = nullptr;

BackendManager *BackendManager::instance()
{
    if (!sInstance) {
        sInstance = new BackendManager();
    }

    return sInstance;
}

BackendManager::BackendManager()
    : mInterface(nullptr)
    , mCrashCount(0)
    , mShuttingDown(false)
    , mRequestsCounter(0)
    , mLoader(nullptr)
    , mInProcessBackend(nullptr)
    , mMethod(OutOfProcess)
{
    Log::instance();
    // Decide whether to run in, or out-of-process

    // if KSCREEN_BACKEND_INPROCESS is set explicitly, we respect that
    const auto _inprocess = qgetenv("KSCREEN_BACKEND_INPROCESS");
    if (!_inprocess.isEmpty()) {
        const QByteArrayList falses({QByteArray("0"), QByteArray("false")});
        if (!falses.contains(_inprocess.toLower())) {
            mMethod = InProcess;
        } else {
            mMethod = OutOfProcess;
        }
    } else {
        // For XRandR backends, use out of process
        if (preferredBackend().fileName().startsWith(QLatin1String("KSC_XRandR"))) {
            mMethod = OutOfProcess;
        } else {
            mMethod = InProcess;
        }
    }
    initMethod();
}

void BackendManager::initMethod()
{
    if (mMethod == OutOfProcess) {
        qRegisterMetaType<org::kde::kscreen::Backend *>("OrgKdeKscreenBackendInterface");

        mServiceWatcher.setConnection(QDBusConnection::sessionBus());
        connect(&mServiceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &BackendManager::backendServiceUnregistered);

        mResetCrashCountTimer.setSingleShot(true);
        mResetCrashCountTimer.setInterval(60000);
        connect(&mResetCrashCountTimer, &QTimer::timeout, this, [this]() {
            mCrashCount = 0;
        });
    }
}

void BackendManager::setMethod(BackendManager::Method m)
{
    if (mMethod == m) {
        return;
    }
    shutdownBackend();
    mMethod = m;
    initMethod();
}

BackendManager::Method BackendManager::method() const
{
    return mMethod;
}

BackendManager::~BackendManager()
{
    if (mMethod == InProcess) {
        shutdownBackend();
    }
}

QFileInfo BackendManager::preferredBackend(const QString &backend)
{
    /** this is the logic to pick a backend, in order of priority
     *
     * - backend argument is used if not empty
     * - env var KSCREEN_BACKEND is considered
     * - if platform is X11, the XRandR backend is picked
     * - if platform is wayland, KWayland backend is picked
     * - if neither is the case, QScreen backend is picked
     * - the QScreen backend is also used as fallback
     *
     */
    QString backendFilter;
    const auto env_kscreen_backend = QString::fromUtf8(qgetenv("KSCREEN_BACKEND"));
    if (!backend.isEmpty()) {
        backendFilter = backend;
    } else if (!env_kscreen_backend.isEmpty()) {
        backendFilter = env_kscreen_backend;
    } else {
        if (QX11Info::isPlatformX11()) {
            backendFilter = QStringLiteral("XRandR");
        } else if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"))) {
            backendFilter = QStringLiteral("KWayland");
        } else {
            backendFilter = QStringLiteral("QScreen");
        }
    }
    QFileInfo fallback;
    const auto backends = listBackends();
    for (const QFileInfo &f : backends) {
        // Here's the part where we do the match case-insensitive
        if (f.baseName().toLower() == QStringLiteral("ksc_%1").arg(backendFilter.toLower())) {
            return f;
        }
        if (f.baseName() == QLatin1String("KSC_QScreen")) {
            fallback = f;
        }
    }
    //     qCWarning(KSCREEN) << "No preferred backend found. KSCREEN_BACKEND is set to " << env_kscreen_backend;
    //     qCWarning(KSCREEN) << "falling back to " << fallback.fileName();
    return fallback;
}

QFileInfoList BackendManager::listBackends()
{
    // Compile a list of installed backends first
    const QString backendFilter = QStringLiteral("KSC_*");
    const QStringList paths = QCoreApplication::libraryPaths();
    QFileInfoList finfos;
    for (const QString &path : paths) {
        const QDir dir(path + QStringLiteral("/kf6/kscreen/"), backendFilter, QDir::SortFlags(QDir::QDir::Name), QDir::NoDotAndDotDot | QDir::Files);
        finfos.append(dir.entryInfoList());
    }
    return finfos;
}

void BackendManager::setBackendArgs(const QVariantMap &arguments)
{
    if (mBackendArguments != arguments) {
        mBackendArguments = arguments;
    }
}

QVariantMap BackendManager::getBackendArgs()
{
    return mBackendArguments;
}

KScreen::AbstractBackend *BackendManager::loadBackendPlugin(QPluginLoader *loader, const QString &name, const QVariantMap &arguments)
{
    const auto finfo = preferredBackend(name);
    loader->setFileName(finfo.filePath());
    QObject *instance = loader->instance();
    if (!instance) {
        qCDebug(KSCREEN) << loader->errorString();
        return nullptr;
    }

    auto backend = qobject_cast<KScreen::AbstractBackend *>(instance);
    if (backend) {
        backend->init(arguments);
        if (!backend->isValid()) {
            qCDebug(KSCREEN) << "Skipping" << backend->name() << "backend";
            delete backend;
            return nullptr;
        }
        // qCDebug(KSCREEN) << "Loaded" << backend->name() << "backend";
        return backend;
    } else {
        qCDebug(KSCREEN) << finfo.fileName() << "does not provide valid KScreen backend";
    }

    return nullptr;
}

KScreen::AbstractBackend *BackendManager::loadBackendInProcess(const QString &name)
{
    Q_ASSERT(mMethod == InProcess);
    if (mMethod == OutOfProcess) {
        qCWarning(KSCREEN) << "You are trying to load a backend in process, while the BackendManager is set to use OutOfProcess communication. Use "
                              "loadBackendPlugin() instead.";
        return nullptr;
    }
    if (mInProcessBackend != nullptr && (name.isEmpty() || mInProcessBackend->name() == name)) {
        return mInProcessBackend;
    } else if (mInProcessBackend != nullptr && mInProcessBackend->name() != name) {
        shutdownBackend();
    }

    if (mLoader == nullptr) {
        mLoader = new QPluginLoader(this);
    }

    auto backend = BackendManager::loadBackendPlugin(mLoader, name, mBackendArguments);
    if (!backend) {
        return nullptr;
    }
    // qCDebug(KSCREEN) << "Connecting ConfigMonitor to backend.";
    ConfigMonitor::instance()->connectInProcessBackend(backend);
    mInProcessBackend = backend;
    setConfig(backend->config());
    return backend;
}

void BackendManager::requestBackend()
{
    Q_ASSERT(mMethod == OutOfProcess);
    if (mInterface && mInterface->isValid()) {
        ++mRequestsCounter;
        QMetaObject::invokeMethod(this, "emitBackendReady", Qt::QueuedConnection);
        return;
    }

    // Another request already pending
    if (mRequestsCounter > 0) {
        return;
    }
    ++mRequestsCounter;

    startBackend(QString::fromLatin1(qgetenv("KSCREEN_BACKEND")), mBackendArguments);
}

void BackendManager::emitBackendReady()
{
    Q_ASSERT(mMethod == OutOfProcess);
    Q_EMIT backendReady(mInterface);
    --mRequestsCounter;
    if (mShutdownLoop.isRunning()) {
        mShutdownLoop.quit();
    }
}

void BackendManager::startBackend(const QString &backend, const QVariantMap &arguments)
{
    // This will autostart the launcher if it's not running already, calling
    // requestBackend(backend) will:
    //   a) if the launcher is started it will force it to load the correct backend,
    //   b) if the launcher is already running it will make sure it's running with
    //      the same backend as the one we requested and send an error otherwise
    QDBusConnection conn = QDBusConnection::sessionBus();
    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KScreen"),
                                                       QStringLiteral("/"),
                                                       QStringLiteral("org.kde.KScreen"),
                                                       QStringLiteral("requestBackend"));
    call.setArguments({backend, arguments});
    QDBusPendingCall pending = conn.asyncCall(call);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pending);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &BackendManager::onBackendRequestDone);
}

void BackendManager::onBackendRequestDone(QDBusPendingCallWatcher *watcher)
{
    Q_ASSERT(mMethod == OutOfProcess);
    watcher->deleteLater();
    QDBusPendingReply<bool> reply = *watcher;
    // Most probably we requested an explicit backend that is different than the
    // one already loaded in the launcher
    if (reply.isError()) {
        qCWarning(KSCREEN) << "Failed to request backend:" << reply.error().name() << ":" << reply.error().message();
        invalidateInterface();
        emitBackendReady();
        return;
    }

    // Most probably request and explicit backend which is not available or failed
    // to initialize, or the launcher did not find any suitable backend for the
    // current platform.
    if (!reply.value()) {
        qCWarning(KSCREEN) << "Failed to request backend: unknown error";
        invalidateInterface();
        emitBackendReady();
        return;
    }

    // The launcher has successfully loaded the backend we wanted and registered
    // it to DBus (hopefuly), let's try to get an interface for the backend.
    if (mInterface) {
        invalidateInterface();
    }
    mInterface = new org::kde::kscreen::Backend(QStringLiteral("org.kde.KScreen"), QStringLiteral("/backend"), QDBusConnection::sessionBus());
    if (!mInterface->isValid()) {
        qCWarning(KSCREEN) << "Backend successfully requested, but we failed to obtain a valid DBus interface for it";
        invalidateInterface();
        emitBackendReady();
        return;
    }

    // The backend is GO, so let's watch for it's possible disappearance, so we
    // can invalidate the interface
    mServiceWatcher.addWatchedService(mBackendService);

    // Immediatelly request config
    connect(new GetConfigOperation(GetConfigOperation::NoEDID), &GetConfigOperation::finished, [&](ConfigOperation *op) {
        mConfig = qobject_cast<GetConfigOperation *>(op)->config();
        emitBackendReady();
    });
    // And listen for its change.
    connect(mInterface, &org::kde::kscreen::Backend::configChanged, [&](const QVariantMap &newConfig) {
        mConfig = KScreen::ConfigSerializer::deserializeConfig(newConfig);
    });
}

void BackendManager::backendServiceUnregistered(const QString &serviceName)
{
    Q_ASSERT(mMethod == OutOfProcess);
    mServiceWatcher.removeWatchedService(serviceName);

    invalidateInterface();
    requestBackend();
}

void BackendManager::invalidateInterface()
{
    Q_ASSERT(mMethod == OutOfProcess);
    delete mInterface;
    mInterface = nullptr;
    mBackendService.clear();
}

ConfigPtr BackendManager::config() const
{
    return mConfig;
}

void BackendManager::setConfig(ConfigPtr c)
{
    // qCDebug(KSCREEN) << "BackendManager::setConfig, outputs:" << c->outputs().count();
    mConfig = c;
}

void BackendManager::shutdownBackend()
{
    if (mMethod == InProcess) {
        delete mLoader;
        mLoader = nullptr;
        delete mInProcessBackend;
        mInProcessBackend = nullptr;
    } else {
        if (mBackendService.isEmpty() && !mInterface) {
            return;
        }

        // If there are some currently pending requests, then wait for them to
        // finish before quitting
        while (mRequestsCounter > 0) {
            mShutdownLoop.exec();
        }

        mServiceWatcher.removeWatchedService(mBackendService);
        mShuttingDown = true;

        QDBusMessage call =
            QDBusMessage::createMethodCall(QStringLiteral("org.kde.KScreen"), QStringLiteral("/"), QStringLiteral("org.kde.KScreen"), QStringLiteral("quit"));
        // Call synchronously
        QDBusConnection::sessionBus().call(call);
        invalidateInterface();

        while (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.KScreen"))) {
            QThread::msleep(100);
        }
    }
}

#include "moc_backendmanager_p.cpp"

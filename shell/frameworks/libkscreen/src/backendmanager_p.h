/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

/**
 * WARNING: This header is *not* part of public API and is subject to change.
 * There are not guarantees or API or ABI stability or compatibility between
 * releases
 */

#pragma once

#include <QDBusServiceWatcher>
#include <QEventLoop>
#include <QFileInfoList>
#include <QObject>
#include <QPluginLoader>
#include <QProcess>
#include <QTimer>

#include "kscreen_export.h"
#include "types.h"

class QDBusPendingCallWatcher;
class OrgKdeKscreenBackendInterface;

namespace KScreen
{
class AbstractBackend;

class KSCREEN_EXPORT BackendManager : public QObject
{
    Q_OBJECT

public:
    enum Method {
        InProcess,
        OutOfProcess,
    };

    static BackendManager *instance();
    ~BackendManager() override;

    KScreen::ConfigPtr config() const;
    void setConfig(KScreen::ConfigPtr c);

    /** Choose which backend to use
     *
     * This method uses a couple of heuristics to pick the backend to be loaded:
     * - If the @p backend argument is specified and not empty it's used to filter the
     *   available backend list
     * - If specified, the KSCREEN_BACKEND env var is considered (case insensitive)
     * - Otherwise, the wayland backend is picked when the runtime platform is Wayland
     *   (we assume kwin in this case
     * - Otherwise, if the runtime platform is X11, the XRandR backend is picked
     * - If neither is the case, we fall back to the QScreen backend, since that is the
     *   most generally applicable and may work on platforms not explicitly supported
     *
     * @return the backend plugin to load
     * @since 5.7
     */
    static QFileInfo preferredBackend(const QString &backend = QString());

    /** List installed backends
     * @return a list of installed backend plugins
     * @since 5.7
     */
    static QFileInfoList listBackends();

    /** Set arguments map which a backend may use on initialization.
     *
     * Calling this method after a backend has been initialized will have no effect.
     * Arguments map will NOT be automatically cleared on backend shutdown; which
     * makes possible setting arguments before restarting backend in tests.
     *
     * @param map of arbitrary arguments for backends; each backend is free to interpret
     * them as it sees fit.
     * @since 5.27
     */
    void setBackendArgs(const QVariantMap &arguments);

    /** Get arguments map which a backend may use on initialization.
     *
     * @return map of arbitrary arguments for backends.
     * @since 5.27
     */
    QVariantMap getBackendArgs();

    /** Encapsulates the plugin loading logic.
     *
     * @param loader a pointer to the QPluginLoader, the caller is
     * responsible for its memory management.
     * @param name name of the backend plugin
     * @param arguments arguments, used for unit tests
     * @return a pointer to the backend loaded from the plugin
     * @since 5.6
     */
    static KScreen::AbstractBackend *loadBackendPlugin(QPluginLoader *loader, const QString &name, const QVariantMap &arguments);

    KScreen::AbstractBackend *loadBackendInProcess(const QString &name);

    BackendManager::Method method() const;
    void setMethod(BackendManager::Method m);

    // For out-of-process operation
    void requestBackend();
    void shutdownBackend();

Q_SIGNALS:
    void backendReady(OrgKdeKscreenBackendInterface *backend);

private Q_SLOTS:
    void emitBackendReady();

    void startBackend(const QString &backend = QString(), const QVariantMap &arguments = QVariantMap());
    void onBackendRequestDone(QDBusPendingCallWatcher *watcher);

    void backendServiceUnregistered(const QString &serviceName);

private:
    friend class SetInProcessOperation;
    friend class InProcessConfigOperationPrivate;
    friend class SetConfigOperation;
    friend class SetConfigOperationPrivate;

    explicit BackendManager();
    static BackendManager *sInstance;

    void initMethod();

    // For out-of-process operation
    void invalidateInterface();
    void backendServiceReady();

    static const int sMaxCrashCount;
    OrgKdeKscreenBackendInterface *mInterface;
    int mCrashCount;

    QString mBackendService;
    QDBusServiceWatcher mServiceWatcher;
    KScreen::ConfigPtr mConfig;
    QVariantMap mBackendArguments;
    QTimer mResetCrashCountTimer;
    bool mShuttingDown;
    int mRequestsCounter;
    QEventLoop mShutdownLoop;

    // For in-process operation
    QPluginLoader *mLoader;
    KScreen::AbstractBackend *mInProcessBackend;

    Method mMethod;
};

}

/*
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "BackendsManager.h"

#include "BackendsConfig.h"

// Include fake backends
#include "backends/fake/FakeBackend.h"
#include "backends/fakehelper/FakeHelperProxy.h"
#include "kauthdebug.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

namespace KAuth
{
AuthBackend *BackendsManager::auth = nullptr;
HelperProxy *BackendsManager::helper = nullptr;

BackendsManager::BackendsManager()
{
}

QList<QObject *> BackendsManager::retrieveInstancesIn(const QString &path)
{
    QList<QObject *> retlist;
    QDir pluginPath(path);
    if (!pluginPath.exists() || path.isEmpty()) {
        return retlist;
    }

    const QFileInfoList entryList = pluginPath.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);

    for (const QFileInfo &fi : entryList) {
        const QString filePath = fi.filePath(); // file name with path
        // QString fileName = fi.fileName(); // just file name

        if (!QLibrary::isLibrary(filePath)) {
            continue;
        }

        QPluginLoader loader(filePath);
        QObject *instance = loader.instance();
        if (instance) {
            retlist.append(instance);
        } else {
            qCWarning(KAUTH) << "Couldn't load" << filePath << "error:" << loader.errorString();
        }
    }
    return retlist;
}

void BackendsManager::init()
{
    // Backend plugin
    const QList<QObject *> backends = retrieveInstancesIn(QFile::decodeName(KAUTH_BACKEND_PLUGIN_DIR));

    for (QObject *instance : backends) {
        auth = qobject_cast<KAuth::AuthBackend *>(instance);
        if (auth) {
            break;
        }
    }

    // Helper plugin
    const QList<QObject *> helpers = retrieveInstancesIn(QFile::decodeName(KAUTH_HELPER_PLUGIN_DIR));

    for (QObject *instance : helpers) {
        helper = qobject_cast<KAuth::HelperProxy *>(instance);
        if (helper) {
            break;
        }
    }

    if (!auth) {
        // Load the fake auth backend then
        auth = new FakeBackend;
#if !KAUTH_COMPILING_FAKE_BACKEND
        // Spit a fat warning
        qCWarning(KAUTH) << "WARNING: KAuth was compiled with a working backend, but was unable to load it! Check your installation!";
#endif
    }

    if (!helper) {
        // Load the fake helper backend then
        helper = new FakeHelperProxy;
#if !KAUTH_COMPILING_FAKE_BACKEND
        // Spit a fat warning
        qCWarning(KAUTH) << "WARNING: KAuth was compiled with a working helper backend, but was unable to load it! "
                            "Check your installation!";
#endif
    }
}

AuthBackend *BackendsManager::authBackend()
{
    if (!auth) {
        init();
    }

    return auth;
}

HelperProxy *BackendsManager::helperProxy()
{
    if (!helper) {
        init();
    }

    return helper;
}

} // namespace Auth

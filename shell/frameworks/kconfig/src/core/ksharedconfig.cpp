/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ksharedconfig.h"
#include "kconfig_core_log_settings.h"
#include "kconfig_p.h"
#include "kconfiggroup.h"
#include <QCoreApplication>
#include <QThread>
#include <QThreadStorage>

using namespace Qt::StringLiterals;

void _k_globalMainConfigSync();

using SharedConfigList = QList<KSharedConfig *>;

class GlobalSharedConfig
{
public:
    GlobalSharedConfig()
        : wasTestModeEnabled(false)
    {
        if (!qApp || QThread::currentThread() == qApp->thread()) {
            // We want to force the sync() before the QCoreApplication
            // instance is gone. Otherwise we trigger a QLockFile::lock()
            // after QCoreApplication is gone, calling qAppName() for a non
            // existent app...
            qAddPostRoutine(&_k_globalMainConfigSync);
        }
        // In other threads, QThreadStorage takes care of deleting the GlobalSharedConfigList when
        // the thread exits.
    }

    SharedConfigList configList;
    // in addition to the list, we need to hold the main config,
    // so that it's not created and destroyed all the time.
    KSharedConfigPtr mainConfig;
    bool wasTestModeEnabled;
};

static QThreadStorage<GlobalSharedConfig *> s_storage;
template<typename T>
T *perThreadGlobalStatic()
{
    if (!s_storage.hasLocalData()) {
        s_storage.setLocalData(new T);
    }
    return s_storage.localData();
}

// Q_GLOBAL_STATIC(GlobalSharedConfigList, globalSharedConfigList), but per thread:
static GlobalSharedConfig *globalSharedConfig()
{
    return perThreadGlobalStatic<GlobalSharedConfig>();
}

namespace
{
[[nodiscard]] QString migrateStateRc(const QString &fileName)
{
    // Migrate from an old legacy path to new spec compliant ~/.local/state/
    // https://gitlab.freedesktop.org/xdg/xdg-specs/-/blob/master/basedir/basedir-spec.xml
    // TODO KF7: refactor openStateConfig so it always opens from XDG_STATE_HOME instead of the legacy when on an XDG platform

#if !defined(Q_OS_WINDOWS) && !defined(Q_OS_ANDROID) && !defined(Q_OS_MACOS)
    if (QFileInfo(fileName).isAbsolute()) {
        return fileName;
    }

    static auto xdgStateHome = qEnvironmentVariable("XDG_STATE_HOME", QDir::homePath() + "/.local/state"_L1);
    if (fileName.startsWith(xdgStateHome)) [[unlikely]] {
        return fileName;
    }

    QString newPath = xdgStateHome + "/"_L1 + fileName; // intentionally not const so it can be move returned
    QString oldPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName);
    if (QFile::exists(oldPath) && QFile::exists(newPath)) {
        qCDebug(KCONFIG_CORE_LOG) << "Old staterc and new staterc found. Not migrating! Using new path" << newPath;
        return newPath;
    }

    if (QFile::exists(newPath)) { // already migrated
        return newPath;
    }

    // Migrate legacy files.
    // On failure we return the new path because we want higher level technology to surface the new path for read/write errors.
    if (!QDir().exists(xdgStateHome)) {
        if (!QDir().mkpath(xdgStateHome)) {
            qCWarning(KCONFIG_CORE_LOG) << "Failed to make state directory" << xdgStateHome;
            return newPath;
        }
    }
    qCInfo(KCONFIG_CORE_LOG) << "Migrating old staterc" << oldPath << "->" << newPath;
    if (!QFile::rename(oldPath, newPath)) {
        qCWarning(KCONFIG_CORE_LOG) << "Failed to migrate" << oldPath << "->" << newPath;
        return newPath;
    }

    return newPath;
#else
    return fileName;
#endif
}
} // namespace

void _k_globalMainConfigSync()
{
    if (KSharedConfigPtr mainConfig = globalSharedConfig()->mainConfig) {
        mainConfig->sync();
    }
}

KSharedConfigPtr KSharedConfig::openConfig(const QString &_fileName, OpenFlags flags, QStandardPaths::StandardLocation resType)
{
    QString fileName(_fileName);
    GlobalSharedConfig *global = globalSharedConfig();
    if (fileName.isEmpty() && !flags.testFlag(KConfig::SimpleConfig)) {
        // Determine the config file name that KConfig will make up (see KConfigPrivate::changeFileName)
        fileName = KConfig::mainConfigName();
    }

    if (!global->wasTestModeEnabled && QStandardPaths::isTestModeEnabled()) {
        global->wasTestModeEnabled = true;
        global->configList.clear();
        global->mainConfig = nullptr;
    }

    for (auto *cfg : std::as_const(global->configList)) {
        if (cfg->name() == fileName && cfg->d_ptr->openFlags == flags && cfg->locationType() == resType) {
            return KSharedConfigPtr(cfg);
        }
    }

    KSharedConfigPtr ptr(new KSharedConfig(fileName, flags, resType));

    if (_fileName.isEmpty() && flags == FullConfig && resType == QStandardPaths::GenericConfigLocation) {
        global->mainConfig = ptr;

        const bool isMainThread = !qApp || QThread::currentThread() == qApp->thread();
        static bool userWarned = false;
        if (isMainThread && !userWarned) {
            userWarned = true;
            const bool isReadOnly = qEnvironmentVariableIsEmpty("KDE_HOME_READONLY");
            if (isReadOnly && QCoreApplication::applicationName() != QLatin1String("kdialog")) {
                if (ptr->group(QStringLiteral("General")).readEntry(QStringLiteral("warn_unwritable_config"), true)) {
                    ptr->isConfigWritable(true);
                }
            }
        }
    }

    return ptr;
}

KSharedConfig::Ptr KSharedConfig::openStateConfig(const QString &_fileName)
{
    QString fileName(_fileName);

    if (fileName.isEmpty()) {
        fileName = QCoreApplication::applicationName() + QLatin1String("staterc");
    }

    return openConfig(migrateStateRc(fileName),
                      SimpleConfig,
                      QStandardPaths::AppDataLocation /* only used on !XDG platform, on XDG we resolve an absolute path (unless there are problems) */);
}

KSharedConfig::KSharedConfig(const QString &fileName, OpenFlags flags, QStandardPaths::StandardLocation resType)
    : KConfig(fileName, flags, resType)
{
    globalSharedConfig()->configList.append(this);
}

KSharedConfig::~KSharedConfig()
{
    if (s_storage.hasLocalData()) {
        globalSharedConfig()->configList.removeAll(this);
    }
}

KConfigGroup KSharedConfig::groupImpl(const QString &groupName)
{
    KSharedConfigPtr ptr(this);
    return KConfigGroup(ptr, groupName);
}

const KConfigGroup KSharedConfig::groupImpl(const QString &groupName) const
{
    const KSharedConfigPtr ptr(const_cast<KSharedConfig *>(this));
    return KConfigGroup(ptr, groupName);
}

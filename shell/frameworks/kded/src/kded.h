/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDED_H
#define KDED_H

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTimer>

#include <QDBusAbstractAdaptor>

#include <KDEDModule>
#include <ksycoca.h>

class QDBusMessage;
class QDBusServiceWatcher;
class KPluginMetaData;
class KDirWatch;

class Kded : public QObject
{
    Q_OBJECT
public:
    Kded();
    ~Kded() override;

    static Kded *self()
    {
        return _self;
    }
    static void messageFilter(const QDBusMessage &);

    void noDemandLoad(const QString &obj); // Don't load obj on demand

    QStringList loadedModules();
    bool unloadModule(const QString &obj);
    // bool isWindowRegistered(qlonglong windowId) const;
    /**
     * Applications can register/unregister their windows with kded modules.
     * This allows kpasswdserver and kcookiejar to delete authentication
     * and cookies that are local to a particular window when closing it.
     */
    //@{
    /**
     * Register a window with KDED
     */
    void registerWindowId(qlonglong windowId, const QString &sender);
    /**
     * Unregister a window previously registered with KDED
     */
    void unregisterWindowId(qlonglong windowId, const QString &sender);
    //@}
    void recreate(bool initial);

    //@{
    /**
     * Check if a module should be loaded on startup.
     *
     * @param module the name of the desktop file for the module, without the .desktop extension
     * @return @c true if the module will be loaded at startup, @c false otherwise
     */
    bool isModuleAutoloaded(const QString &module) const;
    //@}

    //@{
    /**
     * Check if a module is supported on the current QPA
     */
    bool platformSupportsModule(const KPluginMetaData &module) const;
    //@}

    //@{
    /**
     * Check if a module should be loaded on demand
     *
     * @param module the name of the desktop file for the module, without the .desktop extension
     * @return @c true if the module will be loaded when its D-Bus interface
     *         is requested, @c false otherwise
     */
    bool isModuleLoadedOnDemand(const QString &module) const;
    //@}

    /**
     * Configure whether a module should be loaded on startup
     *
     * If a module is set to be auto-loaded, it will be loaded at the start of a KDE
     * session.  Depending on the phase it is set to load in, it may also be loaded
     * when the first KDE application is run outside of a KDE session.
     *
     * @param module the name of the desktop file for the module, without the .desktop extension
     * @param autoload if @c true, the module will be loaded at startup,
     *                 otherwise it will not
     */
    void setModuleAutoloading(const QString &module, bool autoload);

public Q_SLOTS:
    KDEDModule *loadModule(const QString &obj, bool onDemand);

    /**
     * Loads / unloads modules according to config
     */
    void initModules();

    /**
     * Recreate the database file
     */
    void recreate();

    /**
     * Recreating finished
     */
    void recreateDone();

    /**
     * Collect all directories to watch
     */
    void updateDirWatch();

    /**
     * Update directories to watch
     */
    void updateResourceList();

    /**
     * An application unregistered itself from DBus
     */
    void slotApplicationRemoved(const QString &);

protected Q_SLOTS:

    /**
     * @internal Triggers rebuilding
     */
    void dirDeleted(const QString &path);

    /**
     * @internal Triggers rebuilding
     */
    void update(const QString &dir);

    void runDelayedCheck();

protected:
    /**
     * Scans dir for new files and new subdirectories.
     */
    void readDirectory(const QString &dir);
    /**
     * Check if a module should be loaded on demand
     *
     * @param module a service description for the module
     * @return @c true if the module will be loaded when its D-Bus interface
     *         is requested, @c false otherwise
     */
    bool isModuleLoadedOnDemand(const KPluginMetaData &module) const;

    /**
     * Check if a module should be loaded on startup.
     *
     * @param module a service description for the module
     * @return @c true if the module will be loaded at startup, @c false otherwise
     */
    bool isModuleAutoloaded(const KPluginMetaData &module) const;

    KDEDModule *loadModule(const KPluginMetaData &module, bool onDemand);

    QList<KPluginMetaData> availableModules() const;
    /**
     * Pointer to the dirwatch class which tells us, when some directories
     * changed.
     * Slower polling for remote file systems is now done in KDirWatch (JW).
     */
    KDirWatch *m_pDirWatch = nullptr;

    /**
     * When a desktop file is updated, a timer is started (5 sec)
     * before rebuilding the binary - so that multiple updates result
     * in only one rebuilding.
     */
    QTimer *m_pTimer;

    QHash<QString, KDEDModule *> m_modules;
    // QHash<QString,KLibrary *> m_libs;
    QHash<QString, QObject *> m_dontLoad;

    // window id tracking, with a QDBusServiceWatcher to remove them as needed
    QDBusServiceWatcher *m_serviceWatcher;
    QHash<QString, QList<qlonglong>> m_windowIdList;
    QSet<long> m_globalWindowIdList;

    QStringList m_allResourceDirs;
    bool m_needDelayedCheck;

    static Kded *_self;
};

class KUpdateD : public QObject
{
    Q_OBJECT
public:
    KUpdateD();
    ~KUpdateD() override;

public Q_SLOTS:
    void runKonfUpdate();
    void slotNewUpdateFile(const QString &);

private:
    /**
     * Pointer to the dirwatch class which tells us, when some directories
     * changed.
     * Slower polling for remote file systems is now done in KDirWatch (JW).
     */
    KDirWatch *m_pDirWatch = nullptr;

    /**
     * When a desktop file is updated, a timer is started (5 sec)
     * before rebuilding the binary - so that multiple updates result
     * in only one rebuilding.
     */
    QTimer *m_pTimer;
};

#endif

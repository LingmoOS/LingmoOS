/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDED_KDEDADAPTOR_H
#define KDED_KDEDADAPTOR_H

#include <QDBusAbstractAdaptor>
class QDBusMessage;

class KdedAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kded6")
public:
    KdedAdaptor(QObject *parent);

public Q_SLOTS:
    bool loadModule(const QString &obj);
    QStringList loadedModules();
    bool unloadModule(const QString &obj);
    // bool isWindowRegistered(qlonglong windowId) const;
    void registerWindowId(qlonglong windowId, const QDBusMessage &);
    void unregisterWindowId(qlonglong windowId, const QDBusMessage &);
    void reconfigure();
    void quit();

    /**
     * Check if module @a module has @c X-KDE-Kded-autoload=True.
     */
    bool isModuleAutoloaded(const QString &module);

    /**
     * Check if module @a module has @c X-KDE-Kded-load-on-demand=True.
     */
    bool isModuleLoadedOnDemand(const QString &module);

    /**
     * Set @c X-KDE-Kded-autoload to @a autoload for module @a module.
     */
    void setModuleAutoloading(const QString &module, bool autoload);
};

#endif

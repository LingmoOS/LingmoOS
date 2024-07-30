/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

*/
#ifndef __KDEDMODULE_H__
#define __KDEDMODULE_H__

#include <kdbusaddons_export.h>

#include <QObject>
#include <memory>

class KDEDModulePrivate;
class Kded;

class QDBusObjectPath;
class QDBusMessage;

/**
 * \class KDEDModule kdedmodule.h <KDEDModule>
 *
 * The base class for KDED modules.
 *
 * KDED modules are constructed as shared libraries that are loaded on-demand
 * into the kded daemon at runtime.
 *
 * See https://invent.kde.org/frameworks/kded/-/blob/master/docs/HOWTO
 * for documentation about writing kded modules.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class KDBUSADDONS_EXPORT KDEDModule : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KDEDModule")

    friend class Kded;

public:
    /**
     * Constructor
     */
    explicit KDEDModule(QObject *parent = nullptr);

    ~KDEDModule() override;

    /**
     * Sets the name of the module, and uses it to register the module to D-Bus.
     *
     * For modules loaded as plugins by a daemon, this is called automatically
     * by the daemon after loading the module. Module authors should NOT call this.
     */
    void setModuleName(const QString &name);

    QString moduleName() const;

    /**
     * Returns the module being called by this D-Bus message.
     * Useful for autoloading modules in kded and similar daemons.
     * @since 5.7
     */
    static QString moduleForMessage(const QDBusMessage &message);

Q_SIGNALS:
    /**
     * Emitted when a mainwindow registers itself.
     */
    void windowRegistered(qlonglong windowId);

    /**
     * Emitted when a mainwindow unregisters itself.
     */
    void windowUnregistered(qlonglong windowId);

    /**
     * Emitted after the module is registered successfully with D-Bus
     *
     * @since 4.2
     */
    void moduleRegistered(const QDBusObjectPath &path);

private:
    std::unique_ptr<KDEDModulePrivate> const d;
};

#endif

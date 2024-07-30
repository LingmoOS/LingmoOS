/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdedmodule.h"
#include "kdbusaddons_debug.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>

class KDEDModulePrivate
{
public:
    QString moduleName;
};

KDEDModule::KDEDModule(QObject *parent)
    : QObject(parent)
    , d(new KDEDModulePrivate)
{
}

KDEDModule::~KDEDModule()
{
}

void KDEDModule::setModuleName(const QString &name)
{
    d->moduleName = name;
    QDBusObjectPath realPath(QLatin1String("/modules/") + d->moduleName);

    if (realPath.path().isEmpty()) {
        qCWarning(KDBUSADDONS_LOG) << "The kded module name" << name << "is invalid!";
        return;
    }

    QDBusConnection::RegisterOptions regOptions;

    if (this->metaObject()->indexOfClassInfo("D-Bus Interface") != -1) {
        // 1. There are kded modules that don't have a D-Bus interface.
        // 2. qt 4.4.3 crashes when trying to emit signals on class without
        //    Q_CLASSINFO("D-Bus Interface", "<your interface>") but
        //    ExportSignal set.
        // We try to solve that for now with just registering Properties and
        // Adaptors. But we should investigate where the sense is in registering
        // the module at all. Just for autoload? Is there a better solution?
        regOptions = QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors;
    } else {
        // Full functional module. Register everything.
        regOptions = QDBusConnection::ExportScriptableSlots //
            | QDBusConnection::ExportScriptableProperties //
            | QDBusConnection::ExportAdaptors;
        qCDebug(KDBUSADDONS_LOG) << "Registration of kded module" << d->moduleName << "without D-Bus interface.";
    }

    if (!QDBusConnection::sessionBus().registerObject(realPath.path(), this, regOptions)) {
        // Happens for khotkeys but the module works. Need some time to investigate.
        qCDebug(KDBUSADDONS_LOG) << "registerObject() returned false for" << d->moduleName;
    } else {
        // qCDebug(KDBUSADDONS_LOG) << "registerObject() successful for" << d->moduleName;
        // Fix deadlock with Qt 5.6: this has to be delayed until the dbus thread is unlocked
        auto registeredSignal = [this, realPath]() {
            moduleRegistered(realPath);
        };
        QMetaObject::invokeMethod(this, registeredSignal, Qt::QueuedConnection);
    }
}

QString KDEDModule::moduleName() const
{
    return d->moduleName;
}

static const char s_modules_path[] = "/modules/";

QString KDEDModule::moduleForMessage(const QDBusMessage &message)
{
    if (message.type() != QDBusMessage::MethodCallMessage) {
        return QString();
    }

    QString obj = message.path();
    if (!obj.startsWith(QLatin1String(s_modules_path))) {
        return QString();
    }

    // Remove the <s_modules_path> part
    obj = obj.mid(strlen(s_modules_path));

    // Remove the part after the modules name
    const int index = obj.indexOf(QLatin1Char('/'));
    if (index != -1) {
        obj = obj.left(index);
    }

    return obj;
}

#include "moc_kdedmodule.cpp"

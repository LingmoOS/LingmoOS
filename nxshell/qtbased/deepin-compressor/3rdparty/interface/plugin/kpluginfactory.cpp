/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpluginfactory.h"
#include "kpluginfactory_p.h"
#include "commonstruct.h"

#include <QObjectCleanupHandler>
#include <QDebug>

Q_GLOBAL_STATIC(QObjectCleanupHandler, factorycleanup)

extern int kLibraryDebugArea();

KPluginFactory::KPluginFactory()
    : d_ptr(new KPluginFactoryPrivate)
{
    Q_D(KPluginFactory);
    d->q_ptr = this;

    factorycleanup()->add(this);
}

KPluginFactory::KPluginFactory(KPluginFactoryPrivate &d)
    : d_ptr(&d)
{
    factorycleanup()->add(this);
}

KPluginFactory::~KPluginFactory()
{
    delete d_ptr;
}

void KPluginFactory::registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction)
{
    Q_D(KPluginFactory);

    Q_ASSERT(metaObject);

    // we allow different interfaces to be registered without keyword
    if (!keyword.isEmpty()) {
        if (d->createInstanceHash.contains(keyword)) {
        }
        d->createInstanceHash.insert(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    } else {
        QList<KPluginFactoryPrivate::Plugin> clashes(d->createInstanceHash.values(keyword));
        const QMetaObject *superClass = metaObject->superClass();
        if (superClass) {
            foreach (const KPluginFactoryPrivate::Plugin &plugin, clashes) {
                for (const QMetaObject *otherSuper = plugin.first->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                    }
                }
            }
        }

        foreach (const KPluginFactoryPrivate::Plugin &plugin, clashes) {
            superClass = plugin.first->superClass();
            if (superClass) {
                for (const QMetaObject *otherSuper = metaObject->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                    }
                }
            }
        }

        d->createInstanceHash.insertMulti(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    }
}

#ifndef KCOREADDONS_NO_DEPRECATED
QObject *KPluginFactory::createObject(QObject *parent, const char *className, const QStringList &args)
{
    Q_UNUSED(parent);
    Q_UNUSED(className);
    Q_UNUSED(args);
    return nullptr;
}
#endif

#ifndef KCOREADDONS_NO_DEPRECATED
KParts::Part *KPluginFactory::createPartObject(QWidget *parentWidget, QObject *parent, const char *classname, const QStringList &args)
{
    Q_UNUSED(parent);
    Q_UNUSED(parentWidget);
    Q_UNUSED(classname);
    Q_UNUSED(args);
    return nullptr;
}
#endif

QObject *KPluginFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    Q_D(KPluginFactory);

    QObject *obj = nullptr;

#ifndef KCOREADDONS_NO_DEPRECATED
    if (keyword.isEmpty()) {
        const QStringList argsStringList = variantListToStringList(args);

        if ((obj = reinterpret_cast<QObject *>(createPartObject(parentWidget, parent, iface, argsStringList)))) {
            objectCreated(obj);
            return obj;
        }

        if ((obj = createObject(parent, iface, argsStringList))) {
            objectCreated(obj);
            return obj;
        }
    }
#endif

    const QList<KPluginFactoryPrivate::Plugin> candidates(d->createInstanceHash.values(keyword));
    // for !keyword.isEmpty() candidates.count() is 0 or 1

    foreach (const KPluginFactoryPrivate::Plugin &plugin, candidates) {
        for (const QMetaObject *current = plugin.first; current; current = current->superClass()) {
            if (0 == qstrcmp(iface, current->className())) {
                if (!obj) {
                    obj = plugin.second(parentWidget, parent, args);
                }
                break;
            }
        }
    }

    if (obj) {
        emit objectCreated(obj);
    }

    return obj;
}

QStringList KPluginFactory::variantListToStringList(const QVariantList &list)
{
    QStringList stringlist;
    for (const QVariant &var : list) {
        stringlist << var.toString();
    }

    return stringlist;
}

QVariantList KPluginFactory::stringListToVariantList(const QStringList &list)
{
    QVariantList variantlist;
    for (const QString &str : list) {
        variantlist << QVariant(str);
    }

    return variantlist;
}

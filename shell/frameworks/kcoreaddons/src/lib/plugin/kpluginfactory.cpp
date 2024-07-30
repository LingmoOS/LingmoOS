/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginfactory.h"
#include "kpluginfactory_p.h"

#include "kcoreaddons_debug.h"
#include <QPluginLoader>
#include <algorithm>

KPluginFactory::KPluginFactory()
    : d(new KPluginFactoryPrivate)
{
}

KPluginFactory::~KPluginFactory() = default;

KPluginFactory::Result<KPluginFactory> KPluginFactory::loadFactory(const KPluginMetaData &data)
{
    Result<KPluginFactory> result;
    QObject *obj = nullptr;
    if (data.isStaticPlugin()) {
        obj = data.staticPlugin().instance();
    } else {
        if (data.fileName().isEmpty()) {
            result.errorString = tr("Could not find plugin %1").arg(data.requestedFileName());
            result.errorText = QStringLiteral("Could not find plugin %1").arg(data.requestedFileName());
            result.errorReason = INVALID_PLUGIN;
            qCWarning(KCOREADDONS_DEBUG) << result.errorText;
            return result;
        }
        QPluginLoader loader(data.fileName());
        obj = loader.instance();
        if (!obj) {
            result.errorString = tr("Could not load plugin from %1: %2").arg(data.fileName(), loader.errorString());
            result.errorText = QStringLiteral("Could not load plugin from %1: %2").arg(data.fileName(), loader.errorString());
            result.errorReason = INVALID_PLUGIN;
            qCWarning(KCOREADDONS_DEBUG) << result.errorText;
            return result;
        }
    }

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == nullptr) {
        result.errorString = tr("The library %1 does not offer a KPluginFactory.").arg(data.fileName());
        result.errorReason = INVALID_FACTORY;
        qCWarning(KCOREADDONS_DEBUG) << "Expected a KPluginFactory, got a" << obj->metaObject()->className();
        delete obj;
        return result;
    }

    factory->setMetaData(data);
    result.plugin = factory;
    return result;
}

KPluginMetaData KPluginFactory::metaData() const
{
    return d->metaData;
}

void KPluginFactory::setMetaData(const KPluginMetaData &metaData)
{
    d->metaData = metaData;
}

void KPluginFactory::registerPlugin(const QMetaObject *metaObject, CreateInstanceWithMetaDataFunction instanceFunction)
{
    Q_ASSERT(metaObject);
    const QMetaObject *superClass = metaObject->superClass();
    Q_ASSERT(superClass);

    for (const KPluginFactoryPrivate::PluginWithMetadata &plugin : d->createInstanceWithMetaDataHash) {
        for (const QMetaObject *otherSuper = plugin.first->superClass(); otherSuper; otherSuper = otherSuper->superClass()) {
            if (superClass == otherSuper) {
                qCWarning(KCOREADDONS_DEBUG).nospace() << "Two plugins with the same interface (" << superClass->className()
                                                       << ") were registered in the KPluginFactory " << this->metaObject()->className() << ". "
                                                       << "This might be due to a missing Q_OBJECT macro in one of the registered classes";
            }
        }
    }
    // check hierarchy of newly newly registered plugin against all registered classes
    for (const KPluginFactoryPrivate::PluginWithMetadata &plugin : d->createInstanceWithMetaDataHash) {
        superClass = plugin.first->superClass();
        for (const QMetaObject *otherSuper = metaObject->superClass(); otherSuper; otherSuper = otherSuper->superClass()) {
            if (superClass == otherSuper) {
                qCWarning(KCOREADDONS_DEBUG).nospace() << "Two plugins with the same interface (" << superClass->className()
                                                       << ") were registered in the KPluginFactory " << this->metaObject()->className() << ". "
                                                       << "This might be due to a missing Q_OBJECT macro in one of the registered classes";
            }
        }
    }
    d->createInstanceWithMetaDataHash.push_back({metaObject, instanceFunction});
}

void KPluginFactory::logFailedInstantiationMessage(KPluginMetaData data)
{
    qCWarning(KCOREADDONS_DEBUG) << "KPluginFactory could not load the plugin" << data.fileName();
}
void KPluginFactory::logFailedInstantiationMessage(const char *className, KPluginMetaData data)
{
    qCWarning(KCOREADDONS_DEBUG) << "KPluginFactory could not create a" << className << "instance from" << data.fileName();
}

QObject *KPluginFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args)
{
    for (const KPluginFactoryPrivate::PluginWithMetadata &plugin : d->createInstanceWithMetaDataHash) {
        for (const QMetaObject *current = plugin.first; current; current = current->superClass()) {
            if (0 == qstrcmp(iface, current->className())) {
                return plugin.second(parentWidget, parent, d->metaData, args);
            }
        }
    }

    return nullptr;
}

#include "moc_kpluginfactory.cpp"

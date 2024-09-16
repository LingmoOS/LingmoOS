/*  This file is part of the KDE project
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpluginloader.h"
#include "kpluginfactory.h"
#include "kpluginmetadata.h"

#include <QLibrary>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QCoreApplication>
#include <QMutex>
#include <QDebug>

// TODO: Upstream the versioning stuff to Qt
// TODO: Patch for Qt to expose plugin-finding code directly
// TODO: Add a convenience method to KFactory to replace KPluginLoader::factory()
// TODO: (after the above) deprecate this class

class KPluginLoaderPrivate
{
    Q_DECLARE_PUBLIC(KPluginLoader)
protected:
    explicit KPluginLoaderPrivate(const QString &libname)
        : q_ptr(nullptr)
        , name(libname)
        , loader(nullptr)
        , pluginVersion(~0U)
        , pluginVersionResolved(false)
    {
    }

    ~KPluginLoaderPrivate()
    {
    }

    KPluginLoader *q_ptr;
    const QString name;
    QString errorString;
    QPluginLoader *loader;
    quint32 pluginVersion;
    bool pluginVersionResolved;
};

QString KPluginLoader::findPlugin(const QString &name)
{
    // We just defer to Qt; unfortunately, QPluginLoader's searching code is not
    // accessible without creating a QPluginLoader object.

    // Workaround for QTBUG-39642
    static QMutex s_qtWorkaroundMutex;
    QMutexLocker lock(&s_qtWorkaroundMutex);

    QPluginLoader loader(name);
    return loader.fileName();
}

KPluginLoader::KPluginLoader(const QString &plugin, QObject *parent)
    : QObject(parent)
    , d_ptr(new KPluginLoaderPrivate(plugin))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    d->loader = new QPluginLoader(plugin, this);
}

KPluginLoader::KPluginLoader(const KPluginName &pluginName, QObject *parent)
    : QObject(parent)
    , d_ptr(new KPluginLoaderPrivate(pluginName.name()))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    d->loader = new QPluginLoader(this);

    if (pluginName.isValid()) {
        d->loader->setFileName(pluginName.name());
        if (d->loader->fileName().isEmpty()) {
        }
    } else {
        d->errorString = pluginName.errorString();
    }
}

KPluginLoader::~KPluginLoader()
{
    delete d_ptr;
}

KPluginFactory *KPluginLoader::factory()
{
    Q_D(KPluginLoader);

    QObject *obj = instance();

    if (!obj) {
        return nullptr;
    }

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == nullptr) {
        delete obj;
        //d->errorString = tr("The library %1 does not offer a KPluginFactory.").arg(d->name);
        d->errorString = QString("The library %1 does not offer a KPluginFactory.").arg(d->name);
    }

    return factory;
}

quint32 KPluginLoader::pluginVersion()
{
    Q_D(const KPluginLoader);

    if (!load()) {
        return qint32(-1);
    }

    return d->pluginVersion;
}

QString KPluginLoader::pluginName() const
{
    Q_D(const KPluginLoader);

    return d->name;
}

QString KPluginLoader::errorString() const
{
    Q_D(const KPluginLoader);

    if (!d->errorString.isEmpty()) {
        return d->errorString;
    }

    return d->loader->errorString();
}

QString KPluginLoader::fileName() const
{
    Q_D(const KPluginLoader);
    return d->loader->fileName();
}

QObject *KPluginLoader::instance()
{
    Q_D(const KPluginLoader);

    if (!load()) {
        return nullptr;
    }

    return d->loader->instance();
}

bool KPluginLoader::isLoaded() const
{
    Q_D(const KPluginLoader);

    return d->loader->isLoaded() && d->pluginVersionResolved;
}

bool KPluginLoader::load()
{
    Q_D(KPluginLoader);

    if (!d->loader->load()) {
        QString error = d->loader->errorString();
        qInfo() << error;
        return false;
    }

    if (d->pluginVersionResolved) {
        return true;
    }

    Q_ASSERT(!fileName().isEmpty());
    QLibrary lib(fileName());
    Q_ASSERT(lib.isLoaded()); // already loaded by QPluginLoader::load()

    // TODO: this messes up KPluginLoader::errorString(): it will change from unknown error to could not resolve kde_plugin_version
    quint32 *version = reinterpret_cast<quint32 *>(lib.resolve("kde_plugin_version"));
    if (version) {
        d->pluginVersion = *version;
    } else {
        d->pluginVersion = ~0U;
    }

    d->pluginVersionResolved = true;

    return true;
}

QLibrary::LoadHints KPluginLoader::loadHints() const
{
    Q_D(const KPluginLoader);

    return d->loader->loadHints();
}

QJsonObject KPluginLoader::metaData() const
{
    Q_D(const KPluginLoader);

    return d->loader->metaData();
}

void KPluginLoader::setLoadHints(QLibrary::LoadHints loadHints)
{
    Q_D(KPluginLoader);

    d->loader->setLoadHints(loadHints);
}

bool KPluginLoader::unload()
{
    Q_D(KPluginLoader);

    // Even if *this* call does not unload it, another might,
    // so we err on the side of re-resolving the version.
    d->pluginVersionResolved = false;

    return d->loader->unload();
}

void KPluginLoader::forEachPlugin(const QString &directory, std::function<void(const QString &)> callback)
{
    QStringList dirsToCheck;
    qInfo() << QCoreApplication::libraryPaths();
    if (QDir::isAbsolutePath(directory)) {
        dirsToCheck << directory;
    } else {
        foreach (const QString &libDir, QCoreApplication::libraryPaths()) {
            dirsToCheck << libDir + QLatin1Char('/') + directory;
        }
    }

    foreach (const QString &dir, dirsToCheck) {
        QDirIterator it(dir, QDir::Files);
        while (it.hasNext()) {
            it.next();
            if (QLibrary::isLibrary(it.fileName())) {
                callback(it.fileInfo().absoluteFilePath());
                qInfo() << it.fileInfo().absoluteFilePath();
            }
        }
    }
}

QVector<KPluginMetaData> KPluginLoader::findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter)
{
    QVector<KPluginMetaData> ret;

    forEachPlugin(directory, [&](const QString & pluginPath) {
        KPluginMetaData metadata(pluginPath);
        if (!metadata.isValid()) {
            return;
        }

        if (filter && !filter(metadata)) {
            return;
        }

        ret.append(metadata);
    });

    return ret;
}

QVector<KPluginMetaData> KPluginLoader::findPluginsById(const QString &directory, const QString &pluginId)
{
    auto filter = [&pluginId](const KPluginMetaData & md) -> bool {
        return md.pluginId() == pluginId;
    };

    return KPluginLoader::findPlugins(directory, filter);
}

QList<QObject *> KPluginLoader::instantiatePlugins(const QString &directory,
                                                   std::function<bool(const KPluginMetaData &)> filter, QObject *parent)
{
    QList<QObject *> ret;
    QPluginLoader loader;
    foreach (const KPluginMetaData &metadata, findPlugins(directory, filter)) {
        loader.setFileName(metadata.fileName());
        QObject *obj = loader.instance();
        if (!obj) {
            continue;
        }

        obj->setParent(parent);
        ret.append(obj);
    }

    return ret;
}

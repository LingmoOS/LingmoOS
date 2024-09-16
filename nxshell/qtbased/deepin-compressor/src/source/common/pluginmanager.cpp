// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"
#include "kpluginloader.h"

#include <QPluginLoader>
#include <QCoreApplication>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

#include <algorithm>

//静态成员变量初始化。
QMutex PluginManager::m_mutex;//一个线程可以多次锁同一个互斥量
QAtomicPointer<PluginManager> PluginManager::m_instance = nullptr;//原子指针，默认初始化是0

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
    loadPlugins();
}

PluginManager &PluginManager::get_instance()
{
#ifndef Q_ATOMIC_POINTER_TEST_AND_SET_IS_SOMETIMES_NATIVE
    if (!QAtomicPointer<PluginManager>::isTestAndSetNative()) //运行时检测
        qInfo() << "Error: TestAndSetNative not supported!";
#endif

    //使用双重检测。

    /*! testAndSetOrders操作保证在原子操作前和后的的内存访问
     * 不会被重新排序。
     */
    if (m_instance.testAndSetOrdered(nullptr, nullptr)) { //第一次检测
        QMutexLocker locker(&m_mutex);//加互斥锁。

        m_instance.testAndSetOrdered(nullptr, new PluginManager);//第二次检测。
    }

    return *m_instance;
}

QVector<Plugin *> PluginManager::installedPlugins() const
{
    return m_plugins;
}

QVector<Plugin *> PluginManager::availablePlugins() const
{
    QVector<Plugin *> availablePlugins;
    for (Plugin *plugin : qAsConst(m_plugins)) {
        if (plugin->isValid()) {
            availablePlugins << plugin;
        }
    }

    return availablePlugins;
}

QVector<Plugin *> PluginManager::availableWritePlugins() const
{
    QVector<Plugin *> availableWritePlugins;
    const auto plugins = availablePlugins();
    for (Plugin *plugin : plugins) {
        if (plugin->isReadWrite()) {
            availableWritePlugins << plugin;
        }
    }

    return availableWritePlugins;
}

QVector<Plugin *> PluginManager::enabledPlugins() const
{
    QVector<Plugin *> enabledPlugins;
    for (Plugin *plugin : qAsConst(m_plugins)) {
        if (plugin->isEnabled()) {
            enabledPlugins << plugin;
        }
    }

    return enabledPlugins;
}

QVector<Plugin *> PluginManager::preferredPluginsFor(const CustomMimeType &mimeType)
{
    const auto mimeName = mimeType.name();
    if (m_preferredPluginsCache.contains(mimeName)) {
        return m_preferredPluginsCache.value(mimeName);
    }

    const auto plugins = preferredPluginsFor(mimeType, false);
    m_preferredPluginsCache.insert(mimeName, plugins);
    return plugins;
}

QVector<Plugin *> PluginManager::preferredWritePluginsFor(const CustomMimeType &mimeType) const
{
    return preferredPluginsFor(mimeType, true);
}

Plugin *PluginManager::preferredPluginFor(const CustomMimeType &mimeType)
{
    const QVector<Plugin *> preferredPlugins = preferredPluginsFor(mimeType);
    return preferredPlugins.isEmpty() ? new Plugin() : preferredPlugins.first();
}

Plugin *PluginManager::preferredWritePluginFor(const CustomMimeType &mimeType) const
{
    const QVector<Plugin *> preferredWritePlugins = preferredWritePluginsFor(mimeType);
    return preferredWritePlugins.isEmpty() ? new Plugin() : preferredWritePlugins.first();
}

QStringList PluginManager::supportedMimeTypes(MimeSortingMode mode) const
{
    QSet<QString> supported;
    QMimeDatabase db;
    const auto plugins = availablePlugins();
    for (Plugin *plugin : plugins) {
        const auto mimeTypes = plugin->metaData().mimeTypes();
        for (const auto &mimeType : mimeTypes) {
            if (db.mimeTypeForName(mimeType).isValid()) {
                supported.insert(mimeType);
            }
        }
    }

    // Remove entry for lrzipped tar if lrzip executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lrzip")).isEmpty()) {
        supported.remove(QStringLiteral("application/x-lrzip-compressed-tar"));
    }

    // Remove entry for lz4-compressed tar if lz4 executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lz4")).isEmpty()) {
        supported.remove(QStringLiteral("application/x-lz4-compressed-tar"));
    }

    // Remove entry for lzo-compressed tar if libarchive not linked against lzo and lzop executable not found in path.
    if (!libarchiveHasLzo() && QStandardPaths::findExecutable(QStringLiteral("lzop")).isEmpty()) {
        supported.remove(QStringLiteral("application/x-tzo"));
    }

    if (mode == SortByComment) {
        return sortByComment(supported);
    }

    return supported.values();
}

QStringList PluginManager::supportedWriteMimeTypes(MimeSortingMode mode) const
{
    QSet<QString> supported;
    QMimeDatabase db;
    const auto plugins = availableWritePlugins();
    for (Plugin *plugin : plugins) {
        const auto mimeTypes = plugin->metaData().mimeTypes();
        for (const auto &mimeType : mimeTypes) {
            if (db.mimeTypeForName(mimeType).isValid()) {
                supported.insert(mimeType);
            }
        }
    }

    // Remove entry for lrzipped tar if lrzip executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lrzip")).isEmpty()) {
        supported.remove(QStringLiteral("application/x-lrzip-compressed-tar"));
    }

    // Remove entry for lz4-compressed tar if lz4 executable not found in path.
    if (QStandardPaths::findExecutable(QStringLiteral("lz4")).isEmpty()) {
        supported.remove(QStringLiteral("application/x-lz4-compressed-tar"));
    }

    // Remove entry for lzo-compressed tar if libarchive not linked against lzo and lzop executable not found in path.
    if (!libarchiveHasLzo() && QStandardPaths::findExecutable(QStringLiteral("lzop")).isEmpty()) {
        supported.remove(QStringLiteral("application/x-tzo"));
    }

    supported.remove(QStringLiteral("application/x-iso9660-image"));

    supported.remove(QStringLiteral("application/vnd.rar"));
    supported.remove(QStringLiteral("application/x-rar"));
//    supported.remove(QStringLiteral("application/octet-stream"));

    if (mode == SortByComment) {
        return sortByComment(supported);
    }

    return supported.values();
}

QVector<Plugin *> PluginManager::filterBy(const QVector<Plugin *> &plugins, const CustomMimeType &mimeType) const
{
    const bool supportedMime = supportedMimeTypes().contains(mimeType.name());
    QVector<Plugin *> filteredPlugins;
    for (Plugin *plugin : plugins) {
        if (!supportedMime) {
            // Check whether the mimetype inherits from a supported mimetype.
            const QStringList mimeTypes = plugin->metaData().mimeTypes();
            for (const QString &mime : mimeTypes) {
                if (mimeType.inherits(mime)/* && (mime != "application/octet-stream")*/) {
                    filteredPlugins << plugin;
                }
            }
        } else if (plugin->metaData().mimeTypes().contains(mimeType.name())) {
            qInfo() << plugin->metaData().pluginId() << m_filesize << mimeType.name();
            // if (mimeType.name() == QString("application/x-iso9660-image") && plugin->metaData().pluginId() == QString("kerfuffle_cli7z") && m_filesize  < 4294967296) { //4294967296(4GB)
            //                continue;//when iso is more than 4G,it is udf,use 7z to extract
            // }
            if (mimeType.name() == QString("application/x-tzo") && plugin->metaData().pluginId() == QString("kerfuffle_cli7z")) {
                filteredPlugins << plugin;
                continue;
            }

            filteredPlugins << plugin;
        }
    }

    qInfo() << filteredPlugins.count();
    return filteredPlugins;
}

void PluginManager::setFileSize(qint64 size)
{
    m_filesize = size;
}

void PluginManager::loadPlugins()
{
#ifndef LINGLONG_PREFIX
#define LINGLONG_PREFIX "/usr/"
#endif

    QStringList environment = QProcess::systemEnvironment();
    QString str, t_str;
    foreach (str, environment) {
        if (str.startsWith("LD_LIBRARY_PATH=")) {
            t_str = str;
            break;
        }
    }
    if (t_str.isEmpty()) {
        qWarning() << "library path is empty!!!!";
    } else {
        qInfo() << t_str;
        QStringList liststr = t_str.split("=").at(1).split(":");
        QString libPath;
        libPath = liststr.size() >= 2 ? liststr.at(1) : liststr.at(0);
        QDir dir(libPath);
        dir.cdUp();

        QCoreApplication::addLibraryPath(dir.path());
    }

    QCoreApplication::addLibraryPath("/usr/lib");

    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("deepin-compressor/plugins"));
    QSet<QString> addedPlugins;
    for (const KPluginMetaData &metaData : plugins) {
        const auto pluginId = metaData.pluginId();
        // Filter out duplicate plugins.
        if (addedPlugins.contains(pluginId)) {
            continue;
        }

        Plugin *plugin = new Plugin(this, metaData);
        plugin->setEnabled(true);
        addedPlugins << pluginId;
        m_plugins << plugin;
    }
}

QVector<Plugin *> PluginManager::preferredPluginsFor(const CustomMimeType &mimeType, bool readWrite) const
{
    QVector<Plugin *> preferredPlugins = filterBy((readWrite ? availableWritePlugins() : availablePlugins()), mimeType);

    std::sort(preferredPlugins.begin(), preferredPlugins.end(), [](Plugin * p1, Plugin * p2) {
        return p1->priority() > p2->priority();
    });

    //arm64 use libzip for comptess *.zip cause libzip is better under huawei
    //#ifdef __arm64__
    //    if( entrySize == 1 && readWrite &&  mimeType.name() == QString("application/zip"))
    //    {
    //        foreach(Plugin* plugin, preferredPlugins)
    //        {
    //            if(plugin->metaData().name().contains("7zip"))
    //            {
    //                preferredPlugins.removeOne(plugin);
    //                break;
    //            }
    //        }
    //    }
    //#else
    //    Q_UNUSED(entrySize)
    //#endif

//    if ((!readWrite) && (mimeType.name() == QString("application/zip") || mimeType.name() == QString("application/x-tar"))) {
//        foreach (Plugin *plugin, preferredPlugins) {
//            if (plugin->metaData().name().contains("7zip")) {
//                preferredPlugins.removeOne(plugin);
//                break;
//            }
//        }
//    }

    return preferredPlugins;
}

QStringList PluginManager::sortByComment(const QSet<QString> &mimeTypes)
{
    QMap<QString, QString> map;

    // Initialize the QMap to sort by comment.
    for (const QString &mimeType : mimeTypes) {
        QMimeType mime(QMimeDatabase().mimeTypeForName(mimeType));
        map[mime.preferredSuffix().toLower()] = mime.name();
    }

    // Convert to sorted QStringList.
    QStringList sortedMimeTypes;
    for (const QString &value : qAsConst(map)) {
        sortedMimeTypes << value;
    }

    return sortedMimeTypes;
}

bool PluginManager::libarchiveHasLzo()
{
    // Step 1: look for the libarchive plugin, which is built against libarchive.
    const QString strPluginPath = []() {
        const QStringList paths = QCoreApplication::libraryPaths();
        for (const QString &path : paths) {
            const QString pluginPath = QStringLiteral("%1/kerfuffle/kerfuffle_libarchive.so").arg(path);
            if (QFileInfo::exists(pluginPath)) {
                return pluginPath;
            }
        }

        return QString();
    }();

    // Step 2: process the libarchive plugin dependencies to figure out the absolute libarchive path.
    QProcess dependencyTool;
    QStringList args;
#ifdef DEPENDENCY_TOOL_ARGS
    args << QStringLiteral(DEPENDENCY_TOOL_ARGS);
#endif
    dependencyTool.setProgram(QStringLiteral("ldd"));
    dependencyTool.setArguments(args + QStringList(strPluginPath));
    dependencyTool.start();
    dependencyTool.waitForFinished();
    const QString output = QString::fromUtf8(dependencyTool.readAllStandardOutput());
    QRegularExpression regex(QStringLiteral("/.*/libarchive.so|/.*/libarchive.*.dylib"));
    if (!regex.match(output).hasMatch()) {
        return false;
    }

    // Step 3: check whether libarchive links against liblzo.
    const QStringList libarchivePath(regex.match(output).captured(0));
    dependencyTool.setArguments(args + libarchivePath);
    dependencyTool.start();
    dependencyTool.waitForFinished();
    return dependencyTool.readAllStandardOutput().contains(QByteArrayLiteral("lzo"));
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "plugin.h"
#include "mimetypes.h"

#include <QMimeType>
#include <QVector>
#include <QMutex>
#include "dcompressornamespace.h"
DCOMPRESSOR_BEGIN_NAMESPACE
class PluginManager : public QObject
{
    Q_OBJECT

public:
    enum MimeSortingMode {
        Unsorted,
        SortByComment
    };

    explicit PluginManager(QObject *parent = nullptr);

    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;
    static PluginManager &get_instance();
    QVector<Plugin *> installedPlugins() const;
    QVector<Plugin *> availablePlugins() const;
    QVector<Plugin *> availableWritePlugins() const;
    QVector<Plugin *> enabledPlugins() const;
    QVector<Plugin *> preferredPluginsFor(const CustomMimeType &mimeType);
    QVector<Plugin *> preferredWritePluginsFor(const CustomMimeType &mimeType) const;
    Plugin *preferredPluginFor(const CustomMimeType &mimeType);
    Plugin *preferredWritePluginFor(const CustomMimeType &mimeType) const;
    QStringList supportedMimeTypes(MimeSortingMode mode = Unsorted) const;
    QStringList supportedWriteMimeTypes(MimeSortingMode mode = Unsorted) const;
    QVector<Plugin *> filterBy(const QVector<Plugin *> &plugins, const CustomMimeType &mimeType) const;
    void setFileSize(qint64 size);

private:
    void loadPlugins();
    QVector<Plugin *> preferredPluginsFor(const CustomMimeType &mimeType, bool readWrite) const;
    static QStringList sortByComment(const QSet<QString> &mimeTypes);
    static bool libarchiveHasLzo();

    QVector<Plugin *> m_plugins;
    QHash<QString, QVector<Plugin *>> m_preferredPluginsCache;
    qint64 m_filesize = 0;

    static QMutex m_mutex;
    static QAtomicPointer<PluginManager> m_instance;
};
DCOMPRESSOR_END_NAMESPACE
#endif

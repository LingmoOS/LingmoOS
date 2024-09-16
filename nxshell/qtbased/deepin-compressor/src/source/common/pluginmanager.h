// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "plugin.h"
#include "mimetypes.h"

#include <QMimeType>
#include <QVector>
#include <QMutex>

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
//    {
//        static PluginManager instance;
//        return instance;

//    }

    /**
    * @brief installedPlugins 获取已安装插件
    */
    QVector<Plugin *> installedPlugins() const;

    /**
    * @brief availablePlugins 获取有效插件
    */
    QVector<Plugin *> availablePlugins() const;

    /**
    * @brief availableWritePlugins 获取可写插件
    */
    QVector<Plugin *> availableWritePlugins() const;
    QVector<Plugin *> enabledPlugins() const;

    /**
    * @brief preferredPluginsFor 根据类型获取使用的插件
    */
    QVector<Plugin *> preferredPluginsFor(const CustomMimeType &mimeType);

    /**
    * @brief preferredWritePluginsFor 根据压缩类型获取可写插件
    */
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

    static QMutex m_mutex;//实例互斥锁。
    static QAtomicPointer<PluginManager> m_instance;/*!<使用原子指针,默认初始化为0。*/
};

#endif

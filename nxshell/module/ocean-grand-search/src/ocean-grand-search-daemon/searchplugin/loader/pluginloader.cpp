// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader.h"
#include "utils/searchpluginconf.h"
#include "global/builtinsearch.h"
#include "searchplugin/convertors/dataconvertor.h"

#include <QDir>
#include <QSettings>
#include <QDebug>

using namespace GrandSearch;

PluginLoader::PluginLoader(QObject *parent) : QObject(parent)
{

}

void PluginLoader::setPluginPath(const QStringList &dirPaths)
{
    QStringList paths;
    for (const QString &path : dirPaths) {
        QDir dir(path);
        if (dir.isReadable()) {
            qDebug() << "add plugin path:" << path;
            paths << path;
        }
        else
            qWarning() << "invaild plugin path:" << path;
    }

    qDebug() << "update plugin paths" << paths.size();
    m_paths = paths;
}

bool PluginLoader::load()
{
    m_plugins.clear();
    for (const QString &path : m_paths) {
        QDir dir(path);
        if (!dir.isReadable())
            continue;

        auto entrys = dir.entryInfoList({"*.conf"}, QDir::Files, QDir::Name);
        for (const QFileInfo &entry : entrys) {
            SearchPluginInfo info;
            if (readInfo(entry.absoluteFilePath(), info)) {
                // 检查是否与内置的搜索项目冲突
                DEF_BUILTISEARCH_NAMES;
                if (predefBuiltinSearches.contains(info.name)) {
                    qWarning() << "conflict with builtin search,plugin name" << path << info.name;
                    continue;
                }

                if (m_plugins.contains(info.name)) {
                    qWarning() << "duplicate plugin name" << path << info.name;
                    continue;
                }

                // 检查协议是否有效
                if (!DataConvIns->isSupported(info.ifsVersion)) {
                    qWarning() << "do not support this version,plugin name" << path << info.name;
                    continue;
                }

                qInfo() << "add plugin info" << entry.fileName() << info.name;
                m_plugins.insert(info.name, info);
            } else {
                qWarning() << "plugin info error:" << entry.absoluteFilePath();
            }
        }
    }

    return true;
}

QList<SearchPluginInfo> PluginLoader::plugins() const
{
    return m_plugins.values();
}

bool PluginLoader::getPlugin(const QString &name, SearchPluginInfo &plugin) const
{
    if (m_plugins.contains(name)) {
        plugin = m_plugins.value(name);
        return true;
    }

    return false;
}

bool PluginLoader::readInfo(const QString &path, SearchPluginInfo &info)
{
    qDebug() << "load conf" << path;
    QSettings conf(path, QSettings::IniFormat);

    if (!conf.childGroups().contains(PLUGININTERFACE_CONF_ROOT))
        return false;

    conf.beginGroup(PLUGININTERFACE_CONF_ROOT);

    //插件名称
    info.name = conf.value(PLUGININTERFACE_CONF_NAME, "").toString();
    if (info.name.isEmpty() || m_plugins.contains(info.name))
        return false;

    //通信接口版本
    info.ifsVersion = conf.value(PLUGININTERFACE_CONF_INTERFACEVERSION, "").toString();
    if (info.ifsVersion.isEmpty())
        return false;

    //运行方式
    {
        info.mode = SearchPluginInfo::Mode::Manual;
        QString mode = conf.value(PLUGININTERFACE_CONF_MODE, "").toString().toLower();
        if (mode == PLUGININTERFACE_CONF_MODE_AUTO)
            info.mode = SearchPluginInfo::Mode::Auto;
        else if (mode == PLUGININTERFACE_CONF_MODE_TRIGGER)
            info.mode = SearchPluginInfo::Mode::Trigger;
    }

    if (info.mode == SearchPluginInfo::Mode::Auto) {
        //优先级
        info.priority = static_cast<SearchPluginInfo::Priority>(conf.value(PLUGININTERFACE_CONF_PRIORITY, -1).toInt());
        if (info.priority < SearchPluginInfo::Priority::High || info.priority > SearchPluginInfo::Priority::Low)
            info.priority = SearchPluginInfo::Priority::Low;

        //启动命令
        info.exec = conf.value(PLUGININTERFACE_CONF_EXEC, "").toString();
        if (info.exec.isEmpty())
            return false;
    }

    //服务名
    info.service = conf.value(PLUGININTERFACE_CONF_DBUSSERVICE, "").toString();
    if (info.service.isEmpty())
        return false;

    //服务地址
    info.address = conf.value(PLUGININTERFACE_CONF_DBUSADDRESS, "").toString();
    if (info.address.isEmpty())
        return false;

    //接口名
    info.interface = conf.value(PLUGININTERFACE_CONF_DBUSINTERFACE, "").toString();
    if (info.interface.isEmpty())
        return false;

    info.from = path;
    return true;
}

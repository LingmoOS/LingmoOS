/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include <QDebug>
#include "search-plugin-manager.h"
#include "file-search-plugin.h"
#include "app-search-plugin.h"
#include "settings-search-plugin.h"
#include "note-search-plugin.h"
#include "web-search-plugin.h"
#include "ai-search-plugin.h"

#define PLUGIN_ORDER_SETTINGS QDir::homePath() + "/.config/org.lingmo/lingmo-search/lingmo-search-plugin-order.conf"
#define PLUGINS_INFO_GROUP "PluginsInfo"
#define EXTERNAL_PLUGINS_GROUP "ExternalPlugins"
#define PLUGIN_NAME_VALUE "PluginName"
#define PLUGIN_ORDER_VALUE "Order"
#define PLUGIN_ENABLE_VALUE "Enable"
#define PLUGIN_EXTERNAL_VALUE "External"
#define PLUGIN_PATH_VALUE "Path"
#define PLUGIN_FIX_VALUE "FixPos"
static const QString SETTINGS_VERSION_KEY = "Version";
static const QString SETTINGS_VERSION = "1.0";

using namespace LingmoUISearch;

static SearchPluginManager *global_instance = nullptr;
SearchPluginManager::SearchPluginManager(QObject *parent)
{
    Q_UNUSED(parent)
    initOrderSettings();
    registerPlugin(new AiSearchPlugin);
    registerPlugin(new AppSearchPlugin);
    registerPlugin(new NoteSearchPlugin);
    registerPlugin(new SettingsSearchPlugin);
    registerPlugin(new DirSearchPlugin);
    registerPlugin(new FileSearchPlugin);
    registerPlugin(new FileContentSearchPlugin);
//    registerPlugin(new MailSearchPlugin(this));
    registerPlugin(new WebSearchPlugin);
}

bool SearchPluginManager::registerPlugin(LingmoUISearch::SearchPluginIface *plugin)
{
    bool res(false);

    if (m_map.end() != m_map.find(plugin->name())) {
        qWarning() << "the plugin:" << plugin->name() << "has been registered.";
        if (plugin) {
            delete plugin;
            plugin = nullptr;
        }
        return res;
    }

    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP);
    QStringList plugins = m_orderSettings->childGroups();
    if (plugins.contains(plugin->name())) {
        m_orderSettings->beginGroup(plugin->name());
        if (!m_orderSettings->value(PLUGIN_ENABLE_VALUE).toBool()) {
            qWarning() << "plugin: " << plugin->name() << "is not available now.";
        } else {
            m_map[plugin->name()] = plugin;
            qDebug() << "register search plugin: " << plugin->name();
            res = true;
        }
        m_orderSettings->endGroup();
    } else {
        qWarning() << "Fail to register, can not find plugin:" << plugin->name() << "in config file";
    }
    m_orderSettings->endGroup();

    if (!res) {
        qWarning() << "Can not register plugin:" << plugin->name();
        if (plugin) {
            delete plugin;
            plugin = nullptr;
        }
    }
    return res;
}

bool SearchPluginManager::registerExternalPlugin(SearchPluginIface *plugin, const QString &path)
{
    //TODO: external plugin need a life cycle management!
    bool res(false);
    if (m_map.end() != m_map.find(plugin->name())) {
        qWarning() << "the plugin:" << plugin->name() << "has been registered.";
        return res;
    }

    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP);
    QStringList plugins = m_orderSettings->childGroups();
    if (!plugins.contains(plugin->name())) {
        int pluginOrder = plugins.length() + 1;
        //网页搜索为最后一项
        if (plugins.contains("Web Page")) {
            m_orderSettings->beginGroup("Web Page");
            m_orderSettings->setValue(PLUGIN_ORDER_VALUE, pluginOrder);
            m_orderSettings->endGroup();
            pluginOrder--;
        }
        //更新配置文件
        m_orderSettings->beginGroup(plugin->name());
        m_orderSettings->setValue(PLUGIN_PATH_VALUE, path);
        m_orderSettings->setValue(PLUGIN_ORDER_VALUE, pluginOrder);
        m_orderSettings->setValue(PLUGIN_ENABLE_VALUE, true);
        m_orderSettings->setValue(PLUGIN_EXTERNAL_VALUE, true);
        m_orderSettings->setValue(PLUGIN_FIX_VALUE, false);
        m_orderSettings->endGroup();
    }

    if (m_orderSettings->value(plugin->name() + QString("/") + PLUGIN_ENABLE_VALUE).toBool()) {
        m_map[plugin->name()] = plugin;
        qDebug() << "register search plugin: " << path;
        res = true;
    } else {
        qWarning() << "Plugin is not available.Fail to register: " << plugin->name();
    }
    m_orderSettings->endGroup();

    m_orderSettings->beginGroup(EXTERNAL_PLUGINS_GROUP);
    m_orderSettings->setValue(path, plugin->name());
    m_orderSettings->endGroup();

    return res;
}

bool SearchPluginManager::reRegisterPlugin(const QString &pluginName)
{
    QString name;
    if (pluginName == "Web") {
        name = pluginName + " Page";
    } else if (pluginName == "Content") {
        name = "File " + name + " Search";
    } else if (pluginName.contains(".so")) {
        m_orderSettings->beginGroup(EXTERNAL_PLUGINS_GROUP);
        name = m_orderSettings->value(pluginName).toString();
        m_orderSettings->endGroup();
    } else {
        name = pluginName + " Search";
    }
    bool res(false);
    if (m_map.end() == m_map.find(name)) {
        bool isExternal(false);
        QVariant value = m_orderSettings->value(PLUGINS_INFO_GROUP + QString("/") + name);
        if (value.isNull()) {
            isExternal = true;
        } else {
            isExternal = value.toBool();
        }
        if (pluginName == "Applications") {
            reRegisterPlugin(new AppSearchPlugin, isExternal);
            res = true;
        } else if (pluginName == "Note") {
            reRegisterPlugin(new NoteSearchPlugin, isExternal);
            res = true;
        } else if (pluginName == "Settings") {
            reRegisterPlugin(new SettingsSearchPlugin, isExternal);
            res = true;
        } else if (pluginName == "Dir") {
            reRegisterPlugin(new DirSearchPlugin, isExternal);
            res = true;
        } else if (pluginName == "File") {
            reRegisterPlugin(new FileSearchPlugin, isExternal);
            res = true;
        } else if (pluginName == "Content") {
            reRegisterPlugin(new FileContentSearchPlugin, isExternal);
            res = true;
        } else if (pluginName == "Web") {
            reRegisterPlugin(new WebSearchPlugin, isExternal);
            res = true;
        } else if (pluginName.contains(".so")) {
            //外部插件pluginName为so文件全路径
            QPluginLoader loader(pluginName);
            // version check
            QString type = loader.metaData().value("MetaData").toObject().value("type").toString();
            QString version = loader.metaData().value("MetaData").toObject().value("version").toString();
            if(type == "SEARCH_PLUGIN" and version == SEARCH_PLUGIN_IFACE_VERSION) {
                QObject *plugin = loader.instance();
                if (plugin) {
                    auto externalPlugin = dynamic_cast<SearchPluginIface *>(plugin);
                    reRegisterPlugin(externalPlugin, isExternal, pluginName);
                    res = true;
                }
            }

        } else {
            return res;
        }
    }

    return res;
}

bool SearchPluginManager::reRegisterPlugin(SearchPluginIface *plugin, bool isExternal, const QString &externalPluginPath)
{
    bool res(false);
    if (m_map.end() != m_map.find(plugin->name())) {
        qWarning() << "plugin " << plugin->name() << "has been registered.";
        if (plugin) {
            delete plugin;
            plugin = nullptr;
        }
        return res;
    }
    m_map[plugin->name()] = plugin;

    if (isExternal) {
        if (!externalPluginPath.isEmpty()) {
            m_orderSettings->beginGroup(EXTERNAL_PLUGINS_GROUP);
            m_orderSettings->setValue(externalPluginPath, plugin->name());
            m_orderSettings->endGroup();
        } else {
            qWarning() << QString("Can not identify external plugin %1 through a empty path!").arg(plugin->name());
        }
    }

    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP);
    QStringList plugins = m_orderSettings->childGroups();
    if (plugins.contains(plugin->name())) {
        m_orderSettings->beginGroup(plugin->name());
        m_orderSettings->setValue(PLUGIN_ENABLE_VALUE, true);
        m_orderSettings->endGroup();
        res = true;
    } else {
        qWarning() << "Fail to re-register plugin" << plugin->name() << " Conf file error!";
    }
    m_orderSettings->endGroup();

    if (!res) {
        if (plugin) {
            delete plugin;
            plugin = nullptr;
        }
    } else {
        Q_EMIT this->reRegistered(plugin->name());
    }
    return res;
}

bool SearchPluginManager::unregisterPlugin(const QString &pluginName)
{
    //用来测试命令行的字符串拼接
    QString name = pluginName + " Search";
    if (pluginName == "Web") {
        name = pluginName + " Page";
    } else if (pluginName == "Content") {
        name = "File " + name;
    }
    //外部插件pluginName是so文件全路径
    QString externalPluginPath;
    if (pluginName.contains(".so")) {
        externalPluginPath = pluginName;
        m_orderSettings->beginGroup(EXTERNAL_PLUGINS_GROUP);
        name = m_orderSettings->value(externalPluginPath).toString();
        m_orderSettings->endGroup();
    }

    if (m_map.end() == m_map.find(name)) {
        qWarning() << "plugin:" << name << "has not been registered.";
        return false;
    }
    Q_EMIT this->unregistered(name);

    if (m_map[name]) {
        m_map[name]->stopSearch();
        delete m_map[name];
        m_map[name] = nullptr;
    }
    m_map.erase(m_map.find(name));

    if (!externalPluginPath.isEmpty()) {
        m_orderSettings->remove(EXTERNAL_PLUGINS_GROUP + QString("/") + externalPluginPath);
    }
    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP + QString("/") + name);
    m_orderSettings->setValue(PLUGIN_ENABLE_VALUE, false);
    m_orderSettings->endGroup();

    return true;
    /*
//    m_plugin_order.clear();
//    int i = 0;
//    for (auto iter = m_map.cbegin(); iter != m_map.cend(); iter++) {
//        m_plugin_order[i] = iter->first;
//        i++;
//    }

//    std::map<int, QString>::iterator iter;
//    bool removed(false);
//    for (iter = m_plugin_order.begin(); iter != m_plugin_order.end();) {
//        if (iter->second == name) {
//            iter = m_plugin_order.erase(iter);
//            removed = true;
//        } else if (removed) {
//            m_plugin_order[iter->first - 1] = iter->second;
//            iter = m_plugin_order.erase(iter);
//        } else {
//            iter++;
//        }
//    }
//    return true;
    */
}

bool SearchPluginManager::changePluginPos(const QString &pluginName, int pos)
{
    bool res(false);
    if (pos < 1) {
        qWarning() << "Invalid destination position!";
        return res;
    }
    //用来测试命令行的字符串拼接
    QString name = pluginName + " Search";
    if (pluginName == "Web") {
        name = pluginName + " Page";
    } else if (pluginName == "Content") {
        name = "File " + name + " Search";
    } else if (pluginName.contains(".so")) {
        m_orderSettings->beginGroup(EXTERNAL_PLUGINS_GROUP);
        name = m_orderSettings->value(pluginName).toString();
        m_orderSettings->endGroup();
    }

    if (m_map.find(name) == m_map.end()) {
        qWarning() << "Fail to change pos, plugin" << name <<  "has not been registered.";
        return res;
    }

    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP);
    QStringList plugins = m_orderSettings->childGroups();
    int oldOrder = m_orderSettings->value(name + QString("/") + PLUGIN_ORDER_VALUE).toInt();

    if (oldOrder < 1) {
        qWarning() << "Invalid order value of " << name;
        m_orderSettings->endGroup();
        return res;
    }

    if (oldOrder == pos) {
        qWarning() << "Plugin's position has not been changed";
        m_orderSettings->endGroup();
        return res;
    }

    for (const QString& orderName : plugins) {
        m_orderSettings->beginGroup(orderName);
        if (orderName == name) {
            m_orderSettings->setValue(PLUGIN_ORDER_VALUE, pos);
        } else {
            int orderValue = m_orderSettings->value(PLUGIN_ORDER_VALUE).toInt();
            if (oldOrder > pos and orderValue >= pos and orderValue < oldOrder) {
                m_orderSettings->setValue(PLUGIN_ORDER_VALUE, orderValue + 1);
            } else if (oldOrder < pos and orderValue > oldOrder and orderValue <= pos) {
                m_orderSettings->setValue(PLUGIN_ORDER_VALUE, orderValue - 1);
            }
        }
        m_orderSettings->endGroup();
    }
    m_orderSettings->endGroup();

    Q_EMIT this->changePos(name, pos);
    return res;
}

bool SearchPluginManager::unregisterPlugin(SearchPluginIface *plugin, bool isExternal, const QString &externalPluginPath)
{
    bool res(false);
    if (m_map.end() == m_map.find(plugin->name())) {
        qWarning() << "plugin:" << plugin->name() << "has not been registered.";
        return res;
    }
    Q_EMIT this->unregistered(plugin->name());

    if (m_map[plugin->name()]) {
        m_map[plugin->name()]->stopSearch();
        delete m_map[plugin->name()];
        m_map[plugin->name()] = nullptr;
    }
    m_map.erase(m_map.find(plugin->name()));

    if (isExternal) {
        if (!externalPluginPath.isEmpty()) {
            m_orderSettings->beginGroup(EXTERNAL_PLUGINS_GROUP);
            m_orderSettings->remove(externalPluginPath);
            m_orderSettings->endGroup();
            res = true;
        } else {
            qWarning() << "Fail to unregister, can not identify the plugin through a empty path.";
        }
    }

    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP + QString("/") + plugin->name());
    m_orderSettings->setValue(PLUGIN_ENABLE_VALUE, false);
    m_orderSettings->endGroup();

    return res;
}

SearchPluginManager *SearchPluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new SearchPluginManager;
    }
    return global_instance;
}

const QList<PluginInfo> SearchPluginManager::getPluginIds()
{
    QList<PluginInfo> infoList;
    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP);
    QStringList plugins = m_orderSettings->childGroups();
    for (const QString& pluginName : plugins) {
        if (m_map.find(pluginName) != m_map.end()) {
            m_orderSettings->beginGroup(pluginName);
            PluginInfo info(pluginName,
                            m_orderSettings->value(PLUGIN_ENABLE_VALUE).toBool(),
                            m_orderSettings->value(PLUGIN_ORDER_VALUE).toInt(),
                            m_orderSettings->value(PLUGIN_FIX_VALUE).toBool(),
                            m_orderSettings->value(PLUGIN_EXTERNAL_VALUE).toBool(),
                            m_orderSettings->value(PLUGIN_PATH_VALUE).toString());
            m_orderSettings->endGroup();
            infoList.append(info);
        }
    }
    m_orderSettings->endGroup();
    return infoList;
}

SearchPluginIface *SearchPluginManager::getPlugin(const QString &pluginId)
{
    return m_map[pluginId];
}

SearchPluginManager::~SearchPluginManager()
{
    for (auto iter = m_map.begin(); iter != m_map.end();) {
        if (iter->second) {
            delete iter->second;
            iter->second = nullptr;
        }
    }
    m_map.clear();
}

void SearchPluginManager::initOrderSettings()
{
    m_orderSettings = new QSettings(PLUGIN_ORDER_SETTINGS, QSettings::IniFormat, this);
    bool needRefresh(false);

    m_orderSettings->beginGroup(PLUGINS_INFO_GROUP);
    if (m_orderSettings->childGroups().isEmpty()) {
        needRefresh = true;
    } else if (m_orderSettings->value(SETTINGS_VERSION_KEY).toString() != SETTINGS_VERSION) {
        m_orderSettings->setValue(SETTINGS_VERSION_KEY, SETTINGS_VERSION);
        needRefresh = true;
    }
    if (needRefresh) {
        bool isFixed(false);
        for (int i = 0; i < m_defaultPluginOrder.size(); i++) {
            QString pluginName = m_defaultPluginOrder.at(i);
            //默认Web Page固定位置
            if (pluginName == "Web Page") {
                isFixed = true;
            }
            m_orderSettings->beginGroup(pluginName);
            m_orderSettings->setValue(PLUGIN_ORDER_VALUE, i + 1);
            m_orderSettings->setValue(PLUGIN_ENABLE_VALUE, true);
            m_orderSettings->setValue(PLUGIN_EXTERNAL_VALUE, false);
            m_orderSettings->setValue(PLUGIN_FIX_VALUE, isFixed);
            m_orderSettings->endGroup();
        }
    }
    m_orderSettings->endGroup();
}
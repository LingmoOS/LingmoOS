// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewplugininterface.h"
#include "previewpluginmanager.h"
#include "generalpreviewplugin.h"
#include "utils/previewpluginconf.h"
#include "utils/utils.h"

using namespace GrandSearch;

PreviewPluginManager::PreviewPluginManager()
    : QObject ()
{
    readPluginConfig();
}

PreviewPluginManager::~PreviewPluginManager()
{
    clearPluginInfo();
}

PreviewPlugin *PreviewPluginManager::getPreviewPlugin(const MatchedItem &item)
{
    PreviewPlugin* previewPlugin = nullptr;

    QString mimeType = item.type;
    if (mimeType.isEmpty())
        mimeType = Utils::getFileMimetype(item.item);

    if (!mimeType.isEmpty()) {
        for (auto pluginInfo : m_plugins) {
            if (!pluginInfo.bValid)
                continue;

            //需支持正则表达式 todo
            if (isMimeTypeMatch(mimeType, pluginInfo.mimeTypes)) {
                if (nullptr == pluginInfo.pPlugin) {
                    // 加载预览插件
                    QPluginLoader *loader = new QPluginLoader(pluginInfo.path, this);
                    if (!loader->load()) {
                        qWarning() << QString("load %0 error: %1").arg(pluginInfo.path).arg(loader->errorString());
                        loader->deleteLater();
                        continue;
                    }

                    pluginInfo.pPlugin = loader;
                }

                // 从预览插件创建或获取预览界面
                QObject *pluginObject = pluginInfo.pPlugin->instance();
                if (PreviewPluginInterface *pluginIFace = qobject_cast<PreviewPluginInterface*>(pluginObject))
                    previewPlugin = pluginIFace->create(mimeType);
                break;
            }
        }
    }

    return previewPlugin;
}

bool PreviewPluginManager::isMimeTypeMatch(const QString &mimetype, const QStringList &supportMimeTypes)
{
    bool match = false;
    for (const QString &mt : supportMimeTypes) {
        if (mimetype.compare(mt, Qt::CaseInsensitive) == 0) {
            match = true;
            break;
        }

        int starPos = mt.indexOf("*");
        if (starPos > 0 && mimetype.size() > starPos) {
            if (mt.left(starPos).compare(mimetype.left(starPos)) == 0) {
                match = true;
                break;
            }
        }
    }
    return match;
}

void PreviewPluginManager::clearPluginInfo()
{
    for (auto pluginInfo : m_plugins) {
        if (pluginInfo.pPlugin)
            delete pluginInfo.pPlugin;
    }

    m_plugins.clear();
}

bool PreviewPluginManager::readPluginConfig()
{
    // 获取预览插件路径
#ifdef QT_DEBUG
    char path[PATH_MAX] = {0};
    const char *defaultPath = realpath("./", path);
#else
    auto defaultPath = PLUGIN_PREVIEW_DIR;
#endif
    static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_PREVIEW_DIR is not a string.");

    // 设置预览插件路径
    setPluginPath({QString(defaultPath)});

    // 从预览插件路径加载预览插件信息
    return loadConfig();
}

bool PreviewPluginManager::loadConfig()
{
    // 先清空已有插件信息
    clearPluginInfo();

    for (const QString &path : m_paths) {
        QDir dir(path);
        if (!dir.isReadable())
            continue;

        auto entrys = dir.entryInfoList({"*.conf"}, QDir::Files, QDir::Name);
        for (const QFileInfo &entry : entrys) {
            PreviewPluginInfo info;
            if (readInfo(entry.absoluteFilePath(), info)) {
                // 检查插件版本是否向下兼容
                if (!downwardCompatibility(info.version)) {
                    qWarning() << QString("do not support this version(%1), plugin name:%2 plugin version:%3").arg(m_mainVersion).arg(info.name).arg(info.version);
                    continue;
                }

                qInfo() << "add plugin info" << entry.fileName() << info.name;
                m_plugins.push_back(info);
            } else {
                qWarning() << "plugin info error:" << entry.absoluteFilePath();
            }
        }
    }

    return true;
}

bool PreviewPluginManager::readInfo(const QString &path, PreviewPluginInfo &info)
{
    qDebug() << "load conf" << path;
    QSettings conf(path, QSettings::IniFormat);

    if (!conf.childGroups().contains(PREVIEW_PLUGINIFACE_CONF_ROOT))
        return false;

    conf.beginGroup(PREVIEW_PLUGINIFACE_CONF_ROOT);

    // 插件名称
    info.name = conf.value(PREVIEW_PLUGINIFACE_CONF_NAME, "").toString();
    if (info.name.isEmpty())
        return false;
    if (getPreviewPlugin(info.name)) {
        qWarning() << "duplicate plugin name" << path << info.name;
        return false;
    }

    // 插件版本号
    info.version = conf.value(PREVIEW_PLUGINIFACE_CONF_VERSION, "").toString();
    if (info.version.isEmpty())
        return false;

    // 支持预览的mimetype列表
    info.mimeTypes = conf.value(PREVIEW_PLUGINIFACE_CONF_MIMETYPES, "").toString().split(':', QString::SkipEmptyParts);
    if (info.mimeTypes.isEmpty())
        return false;

    // 插件可用性
    info.bValid = conf.value(PREVIEW_PLUGINIFACE_CONF_VALID, "").toBool();

    // 填入插件所属路径
    QFileInfo fi(path);
    info.path = QDir::cleanPath(fi.path() + QLatin1Char('/') + "/lib" + fi.baseName() + ".so");

    return true;
}

PreviewPluginInfo *PreviewPluginManager::getPreviewPlugin(const QString &name)
{
    if (name.isEmpty())
        return nullptr;

    int nPluginCount = m_plugins.count();
    for (int i = 0; i < nPluginCount; i++) {
      if(m_plugins[i].name == name)
          return &m_plugins[i];
    }

    return nullptr;
}

void PreviewPluginManager::setPluginPath(const QStringList &dirPaths)
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

bool PreviewPluginManager::downwardCompatibility(const QString &version)
{
    Q_UNUSED(version);

    return true;
}

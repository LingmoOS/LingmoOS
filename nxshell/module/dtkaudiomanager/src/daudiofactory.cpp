// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiofactory_p.h"
#include "daudioplugin_p.h"
#include "daudiomanager_p.h"

#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QObject>
#include <QPluginLoader>
#include <QJsonArray>
#include <QLoggingCategory>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DExpected;
Q_DECLARE_LOGGING_CATEGORY(amLog)

class DAudioPluginLoader
{
public:

    QStringList keys() const
    {
        return m_metas.keys();
    }

    DAudioPlugin *plugin(const QString &type)
    {
        if (m_plugin)
            return m_plugin;

        if (m_metas.isEmpty())
            loadMetas();

        for (auto key : keys()) {
            if (!type.isEmpty() && type != key)
                continue;

            auto fileName = m_metas.value(key);

            QPluginLoader loader(fileName);
            if (!loader.instance()) {
                qWarning() << "load the plugin error." << loader.errorString();
                continue;
            }
            m_plugin = dynamic_cast<DAudioPlugin *>(loader.instance());
            if (!m_plugin) {
                qWarning() << "load the the Plugin error." << loader.instance() << key << fileName;
                continue;
            }
            if (m_plugin) {
                qInfo() << "the Audio plugin is [" << key << "]." << fileName;
                break;
            }
        }
        if (!m_plugin) {
            qWarning() << "load plugin error, Don't exist Audio plugin from " << pluginDirs();
        }

        return m_plugin;
    }

private:
    void loadMetas()
    {
        for (QString fileName : pluginPaths()) {
            const auto &info = parsePluginInfo(fileName);
            if (!info.isEmpty()) {
                m_metas.insert(info, fileName);
                qDebug(amLog()) << "load the plugin [" << info << "] successful." << fileName;
            }
        }
    }

    QStringList pluginDirs() const
    {
        // The same pluginid will be overwritten by later, `DTK_AUDIO_PLUGIN_DIRS` > `./plugins` > `/usr`
        QStringList dirs;
        QString outputDir(DAUDIO_MANAGER_PLUGIN_DIR);
        dirs << outputDir;
        dirs << QCoreApplication::applicationDirPath() + "/plugins";
        const QString &envPaths = qEnvironmentVariable("DTK_AUDIO_PLUGIN_DIRS");
        if (!envPaths.isEmpty()) {
            QStringList list = envPaths.split(':');
            std::reverse(list.begin(), list.end());
            dirs << list;
        }
        return dirs;
    }
    QStringList pluginPaths() const
    {
        // The same pluginid will be overwritten by later, `DTK_AUDIO_PLUGIN_DIRS` > `./plugins` > `/usr`
        QStringList dirs = pluginDirs();

        qDebug(amLog()) << "load plugins from those dir:" << dirs;
        QStringList pluginPaths;
        for (auto dir : qAsConst(dirs)) {
            auto pluginsDir = QDir(dir);
            if (!pluginsDir.exists())
                continue;

            const auto entryList = pluginsDir.entryList(QDir::Files | QDir::NoSymLinks);
            for (const QString &fileName : qAsConst(entryList)) {
                const auto path = pluginsDir.absoluteFilePath(fileName);
                if (!QLibrary::isLibrary(path))
                    continue;

                if (!isPlugin(path))
                    continue;

                pluginPaths << path;
            }
        }
        return pluginPaths;
    }

    bool isPlugin(const QString &fileName) const
    {
        const auto &info = parsePluginInfo(fileName);
        return !info.isEmpty();
    }

    QString parsePluginInfo(const QString &fileName) const
    {
        QPluginLoader loader(fileName);
        const auto &meta = loader.metaData();

        do {
            const auto iid = meta["IID"].toString();
            if (iid.isEmpty())
                break;

            if (iid != QString(qobject_interface_iid<DAudioPlugin *>()))
                break;

            const auto metaData = meta["MetaData"];
            const auto key = metaData["Key"].toString();
            if (key.isEmpty())
                break;

            return key;
        } while (false);

        return QString();
    }

private:
    QMap<QString, QString> m_metas;
    DAudioPlugin *m_plugin = nullptr;
};


Q_GLOBAL_STATIC(DAudioPluginLoader, loader)

class DNullAudioManager : public DAudioManagerPrivate
{
public:
    explicit DNullAudioManager()
    {
        qWarning() << "Don't exist Audio implement.";
    }

    virtual void reset() override
    {
    }
    virtual void setReConnectionEnabled(const bool enable) override
    {
        Q_UNUSED(enable);
    }
    virtual bool increaseVolume() const override
    {
        return false;
    }
    virtual bool reduceNoise() const override
    {
        return false;
    }
    virtual double maxVolume() const override
    {
        return 0.0;
    }

public slots:
    virtual DExpected<void> setIncreaseVolume(bool increaseVolume) override
    {
        Q_UNUSED(increaseVolume);
    }
    virtual DExpected<void> setReduceNoise(bool reduceNoise) override
    {
        Q_UNUSED(reduceNoise);
    }
};

DAudioManagerPrivate *DAudioFactory::createAudioManager()
{
    return createAudioManager(QString());
}

DAudioManagerPrivate *DAudioFactory::createAudioManager(const QString &type)
{
    auto plugin = loader->plugin(type);
    return plugin ? plugin->createAudioManager() : new DNullAudioManager();
}

DAUDIOMANAGER_END_NAMESPACE

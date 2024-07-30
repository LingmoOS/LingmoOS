/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KFILEMETADATA_EXTRACTOR_P_H
#define KFILEMETADATA_EXTRACTOR_P_H

#include "extractorplugin.h"
#include "kfilemetadata_debug.h"
#include <QPluginLoader>

namespace KFileMetaData {

class ExtractorPlugin;

class ExtractorPrivate
{
public:
    ~ExtractorPrivate() {
        if (m_autoDeletePlugin == Extractor::AutoDeletePlugin) {
            delete m_plugin;
        }
    }
    bool initPlugin();

    ExtractorPlugin *m_plugin = nullptr;

    Extractor::ExtractorPluginOwnership m_autoDeletePlugin = Extractor::AutoDeletePlugin;

    QVariantMap m_metaData;
    QString m_pluginPath;
};

inline bool ExtractorPrivate::initPlugin()
{
    if (m_plugin) {
        return true;
    }

    QPluginLoader loader(m_pluginPath);
    if (!loader.load()) {
        qCWarning(KFILEMETADATA_LOG) << "Could not create Extractor:" << m_pluginPath;
        qCWarning(KFILEMETADATA_LOG) << loader.errorString();
        return false;
    }

    QObject* obj = loader.instance();
    if (!obj) {
        qCWarning(KFILEMETADATA_LOG) << "Could not create instance:" << m_pluginPath;
        return false;
    }

    m_plugin = qobject_cast<ExtractorPlugin*>(obj);
    if (!m_plugin) {
        qCWarning(KFILEMETADATA_LOG) << "Could not convert to ExtractorPlugin:" << m_pluginPath;
        return false;
    }

    m_autoDeletePlugin = Extractor::DoNotDeletePlugin;
    return true;
}

}

#endif

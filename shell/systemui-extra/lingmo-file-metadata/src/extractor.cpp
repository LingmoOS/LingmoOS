/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractor.h"
#include "extractor-plugin.h"
#include <QDebug>

using namespace LingmoUIFileMetadata;

class LingmoUIFileMetadata::ExtractorPrivate
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
    QObject* obj = loader.instance();
    if (!obj) {
        qWarning() << loader.errorString();
        return false;
    }

    m_plugin = qobject_cast<ExtractorPlugin*>(obj);
    if (!m_plugin) {
        return false;
    }

    m_autoDeletePlugin = Extractor::DoNotDeletePlugin;
    return true;
}

Extractor::Extractor()
    : d(new ExtractorPrivate)
{
}

Extractor::~Extractor() = default;

Extractor::Extractor(Extractor&& other) noexcept
{
    d = std::move(other.d);
}

void Extractor::extract(ExtractionResult* result)
{
    d->m_plugin->extract(result);
}

QStringList Extractor::mimetypes() const
{
    return d->m_plugin->mimetypes();
}

QVariantMap Extractor::extractorProperties() const
{
    return d->m_metaData;
}

void Extractor::setExtractorPlugin(ExtractorPlugin *extractorPlugin)
{
    d->m_plugin = extractorPlugin;
}

void Extractor::setAutoDeletePlugin(ExtractorPluginOwnership autoDelete)
{
    d->m_autoDeletePlugin = autoDelete;
}

void Extractor::setMetaData(const QVariantMap &metaData)
{
    d->m_metaData = metaData;
}

void Extractor::setPluginPath(const QString &path)
{
    d->m_pluginPath = path;
}

bool Extractor::initPlugin()
{
    return d->initPlugin();
}

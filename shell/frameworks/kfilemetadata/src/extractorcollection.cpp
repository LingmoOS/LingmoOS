/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "extractorcollection.h"
#include "extractor_p.h"
#include "extractorplugin.h"
#include "externalextractor.h"
#include "kfilemetadata_debug.h"
#include "config-kfilemetadata.h"

#include <KPluginMetaData>
#include <QCoreApplication>
#include <QDir>
#include <QMimeDatabase>
#include <vector>

using namespace KFileMetaData;

class KFileMetaData::ExtractorCollectionPrivate
{
public:
    QMultiHash<QString, Extractor*> m_mimeExtractors;

    std::vector<Extractor> m_allExtractors;

    void findExtractors();
    QList<Extractor*> getExtractors(const QString& mimetype);
};

ExtractorCollection::ExtractorCollection()
    : d(new ExtractorCollectionPrivate)
{
    d->findExtractors();
}

ExtractorCollection::~ExtractorCollection() = default;


QList<Extractor*> ExtractorCollection::allExtractors()
{
    QList<Extractor*> plugins;
    for (auto& ex : d->m_allExtractors) {
        if (ex.d->initPlugin()) {
            plugins.push_back(&ex);
        }
    }
    return plugins;
}

void ExtractorCollectionPrivate::findExtractors()
{
    const QList<KPluginMetaData> kfilemetadataPlugins =
        KPluginMetaData::findPlugins(QStringLiteral("kf6/kfilemetadata"), {}, KPluginMetaData::AllowEmptyMetaData);
    for (const KPluginMetaData &plugin : kfilemetadataPlugins) {
        Extractor extractor;
        extractor.d->m_pluginPath = plugin.fileName();
        extractor.setAutoDeletePlugin(Extractor::DoNotDeletePlugin);

        if (!plugin.rawData().isEmpty()) {
            qCDebug(KFILEMETADATA_LOG) << "Found plugin with metadata:" << extractor.d->m_pluginPath;
            extractor.setMetaData(plugin.rawData().toVariantMap());
            m_allExtractors.push_back(std::move(extractor));
        } else {
            qCDebug(KFILEMETADATA_LOG) << "Found plugin without metadata:" << extractor.d->m_pluginPath;
            if (extractor.d->initPlugin() && !extractor.mimetypes().isEmpty()) {
                m_allExtractors.push_back(std::move(extractor));
            }
        }
    }

    QStringList externalPlugins;
    const QDir externalPluginDir(QStringLiteral(LIBEXEC_INSTALL_DIR "/kfilemetadata/externalextractors"));
    qCDebug(KFILEMETADATA_LOG) << "Searching for external extractors:" << externalPluginDir.path();
    // For external plugins, we look into the directories
    const QStringList externalPluginEntryList = externalPluginDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &externalPlugin : externalPluginEntryList) {
        if (externalPlugins.contains(externalPlugin)) {
            qCDebug(KFILEMETADATA_LOG) << "Skipping duplicate - " << externalPluginDir.path() << ":" << externalPlugin;
            continue;
        }

        qCDebug(KFILEMETADATA_LOG) << "Adding plugin - " << externalPluginDir.path() << ":" << externalPlugin;
        externalPlugins << externalPlugin;

        Extractor extractor;
        auto pluginPath = externalPluginDir.absoluteFilePath(externalPlugin);
        ExternalExtractor *plugin = new ExternalExtractor(pluginPath);
        if (plugin && !plugin->mimetypes().isEmpty()) {
              extractor.setExtractorPlugin(plugin);
              extractor.setAutoDeletePlugin(Extractor::AutoDeletePlugin);
              m_allExtractors.push_back(std::move(extractor));
        }
    }
    externalPlugins.clear();

    for (Extractor& extractor : m_allExtractors) {
        auto pluginProperties = extractor.extractorProperties();
        if (!pluginProperties.isEmpty()) {
            auto mimetypeProperties = pluginProperties[QLatin1String("MimeTypes")];
            const auto mimetypes = mimetypeProperties.toMap().keys();
            for (const QString &mimetype : mimetypes) {
                m_mimeExtractors.insert(mimetype, &extractor);
            }
        } else if (extractor.d->m_plugin) {
            const auto mimetypes = extractor.mimetypes();
            for (const QString &mimetype : mimetypes) {
                m_mimeExtractors.insert(mimetype, &extractor);
            }
        }
    }
}

QList<Extractor*> ExtractorCollectionPrivate::getExtractors(const QString& mimetype)
{
    QList<Extractor*> extractors = m_mimeExtractors.values(mimetype);

    if (extractors.isEmpty()) {
        qCDebug(KFILEMETADATA_LOG) << "No extractor for" << mimetype;
        return extractors;
    }

    qCDebug(KFILEMETADATA_LOG) << "Fetching extractors for" << mimetype;
    Extractor* failed = nullptr;
    for (auto ex : extractors) {
        if (!ex->d->initPlugin()) {
            failed = ex;
            break;
        }
    }

    if (!failed) {
        return extractors;
    }

    auto it = m_mimeExtractors.begin();
    while (it != m_mimeExtractors.end()) {
        if (it.value() == failed) {
            it = m_mimeExtractors.erase(it);
        } else {
            ++it;
        }
    }
    return getExtractors(mimetype);
}

QList<Extractor*> ExtractorCollection::fetchExtractors(const QString& mimetype) const
{
    QList<Extractor*> plugins = d->getExtractors(mimetype);
    if (!plugins.isEmpty()) {
        return plugins;
    }

    // try to find the best matching more generic extractor by mimetype inheritance
    QMimeDatabase db;
    auto type = db.mimeTypeForName(mimetype);
    const QStringList ancestors = type.allAncestors();

    for (const auto &ancestor : ancestors) {
        if (ancestor == QLatin1String("application/octet-stream")) {
            continue;
        }
        QList<Extractor*> plugins = d->getExtractors(ancestor);
        if (!plugins.isEmpty()) {
            qCDebug(KFILEMETADATA_LOG) << "Using inherited mimetype" << ancestor <<  "for" << mimetype;
            return plugins;
        }
    }

    return plugins;
}

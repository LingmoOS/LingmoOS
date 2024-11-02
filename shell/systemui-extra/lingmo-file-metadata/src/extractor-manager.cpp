/*
 *  SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "extractor-manager.h"
#include <QDir>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QMimeDatabase>
#include "extractor.h"
#include "lingmo-file-metadata-config.h"

using namespace LingmoUIFileMetadata;

class LingmoUIFileMetadata::ExtractorManagerPrivate
{
public:
    QMultiHash<QString, Extractor*> m_mimeExtractors;

    std::vector<Extractor> m_allExtractors;

    void findExtractors();
    QList<Extractor*> getExtractors(const QString& mimetype);
};

void ExtractorManagerPrivate::findExtractors()
{
    QDir pluginsDir(PLUGIN_INSTALL_DIR);
    pluginsDir.setFilter(QDir::Files);

    for(const QString &fileName: pluginsDir.entryList(QDir::Files)) {

        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QJsonObject metaData = pluginLoader.metaData().value("MetaData").toObject();
        QString type =metaData.value("Type").toString();
        QString version = metaData.value("Version").toString();

        if(type == "LINGMO_FILE_METADATA_EXTRACTOR") {
            if(version != EXTRACTOR_PLUGIN_IFACE_VERSION) {
                qWarning() << "LINGMO_FILE_METADATA_EXTRACTOR version check failed:" << fileName << "version:" << version << "iface version : " << EXTRACTOR_PLUGIN_IFACE_VERSION;
                continue;
            }
        }
        if(metaData.value("MimeTypes").isNull()) {
            qWarning() << "LINGMO_FILE_METADATA_EXTRACTOR MimeTypes check failed:" << fileName << ", MimeTypes is null";
            continue;
        }

        Extractor extractor;
        extractor.setPluginPath(pluginsDir.absoluteFilePath(fileName));
        extractor.setMetaData(metaData.toVariantMap());
        extractor.setAutoDeletePlugin(Extractor::DoNotDeletePlugin);

        m_allExtractors.push_back(std::move(extractor));
    }

    for (Extractor& extractor : m_allExtractors) {
        auto pluginProperties = extractor.extractorProperties();

        auto mimetypeProperties = pluginProperties.value("MimeTypes");
        const auto mimetypes = mimetypeProperties.toMap().keys();
        for (const QString &mimetype : mimetypes) {
            m_mimeExtractors.insert(mimetype, &extractor);
        }
    }
}

QList<Extractor *> ExtractorManagerPrivate::getExtractors(const QString &mimetype)
{
    QList<Extractor*> extractors = m_mimeExtractors.values(mimetype);

    if (extractors.isEmpty()) {
        qWarning() << "No extractor for" << mimetype;
        return extractors;
    }
    Extractor* failed = nullptr;
    for (auto ex : extractors) {
        if (!ex->initPlugin()) {
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

ExtractorManager::ExtractorManager() : d(new ExtractorManagerPrivate)
{
    d->findExtractors();
}

ExtractorManager::~ExtractorManager() = default;

QList<Extractor*> ExtractorManager::fetchExtractors(const QString &mimetype) const
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
        plugins = d->getExtractors(ancestor);
        if (!plugins.isEmpty()) {
            qDebug() << "Using inherited mimetype" << ancestor <<  "for" << mimetype;
            return plugins;
        }
    }

    return plugins;
}

QList<Extractor *> ExtractorManager::allExtractors()
{
    QList<Extractor*> plugins;
    for (Extractor& ex : d->m_allExtractors) {
        if (ex.initPlugin()) {
            plugins.push_back(&ex);
        }
    }
    return plugins;
}

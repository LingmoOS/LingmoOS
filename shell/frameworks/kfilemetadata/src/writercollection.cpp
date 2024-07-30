/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2016 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "writercollection.h"
#include "writer_p.h"
#include "writerplugin.h"
#include "externalwriter.h"
#include "kfilemetadata_debug.h"
#include "config-kfilemetadata.h"

#include <KPluginMetaData>
#include <QCoreApplication>
#include <QDir>
#include <QMimeDatabase>
#include <QPluginLoader>
#include <vector>

using namespace KFileMetaData;

class KFileMetaData::WriterCollectionPrivate
{
public:
    QMultiHash<QString, Writer*> m_writers;

    std::vector<Writer> m_allWriters;

    void findWriters();
};

WriterCollection::WriterCollection()
    : d(new WriterCollectionPrivate)
{
    d->findWriters();
}

WriterCollection::~WriterCollection() = default;

void WriterCollectionPrivate::findWriters()
{
    const auto internalPlugins = KPluginMetaData::findPlugins(QStringLiteral("kf6/kfilemetadata/writers"), {}, KPluginMetaData::AllowEmptyMetaData);

    for (const KPluginMetaData &metaData : internalPlugins) {
        QPluginLoader loader(metaData.fileName());
        if (QObject *obj = loader.instance()) {
            if (WriterPlugin *plugin = qobject_cast<WriterPlugin *>(obj)) {
                Writer writer;
                writer.d->m_plugin = plugin;
                writer.setAutoDeletePlugin(Writer::DoNotDeletePlugin);

                m_allWriters.push_back(std::move(writer));
            } else {
                qCDebug(KFILEMETADATA_LOG) << "Plugin could not be converted to a WriterPlugin";
                qCDebug(KFILEMETADATA_LOG) << metaData.fileName();
            }
        } else {
            qCDebug(KFILEMETADATA_LOG) << "Plugin could not create instance" << metaData.fileName();
        }
    }

    QStringList externalPlugins;
    QStringList externalPluginPaths;
    QDir externalPluginDir(QStringLiteral(LIBEXEC_INSTALL_DIR) + QStringLiteral("/kfilemetadata/writers/externalwriters"));
    // For external plugins, we look into the directories. Those are executables and not C++ plugins.
    const QStringList externalPluginEntryList = externalPluginDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& externalPlugin : externalPluginEntryList) {
        if (externalPlugins.contains(externalPlugin)) {
            continue;
        }

        externalPlugins << externalPlugin;
        externalPluginPaths << externalPluginDir.absoluteFilePath(externalPlugin);
    }

    for (const QString& externalPluginPath : std::as_const(externalPluginPaths)) {
        ExternalWriter *plugin = new ExternalWriter(externalPluginPath);
        Writer writer;
        writer.d->m_plugin = plugin;
        writer.setAutoDeletePlugin(Writer::AutoDeletePlugin);

        m_allWriters.push_back(std::move(writer));
    }

    for (Writer& writer : m_allWriters) {
        const QStringList lst = writer.mimetypes();
        for (const QString& type : lst) {
            m_writers.insert(type, &writer);
        }
    }
}

QList<Writer*> WriterCollection::fetchWriters(const QString& mimetype) const
{
    QList<Writer*> plugins = d->m_writers.values(mimetype);
    if (!plugins.isEmpty()) {
        return plugins;
    }

    // try to find the best matching more generic writer by mimetype inheritance
    QMimeDatabase db;
    auto type = db.mimeTypeForName(mimetype);
    const QStringList ancestors = type.allAncestors();

    for (const auto &ancestor : ancestors) {
        if (ancestor == QLatin1String("application/octet-stream")) {
            continue;
        }
        QList<Writer*> plugins = d->m_writers.values(ancestor);
        if (!plugins.isEmpty()) {
            qCDebug(KFILEMETADATA_LOG) << "Using inherited mimetype" << ancestor <<  "for" << mimetype;
            return plugins;
        }
    }

    return plugins;
}




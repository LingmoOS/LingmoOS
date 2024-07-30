/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2015 Boudhayan Gupta <bgupta@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "externalextractor.h"
#include "kfilemetadata_debug.h"
#include "properties.h"
#include "propertyinfo.h"
#include "typeinfo.h"

#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace KFileMetaData
{
class ExternalExtractorPrivate
{
public:
    QString path;
    QStringList writeMimetypes;
    QString mainPath;
};
}

using namespace KFileMetaData;

ExternalExtractor::ExternalExtractor(QObject* parent)
    : ExtractorPlugin(parent),
      d_ptr(new ExternalExtractorPrivate)
{
}

ExternalExtractor::ExternalExtractor(const QString& pluginPath)
    : ExtractorPlugin(nullptr),
      d_ptr(new ExternalExtractorPrivate)
{
    Q_D(ExternalExtractor);

    d->path = pluginPath;

    QDir pluginDir(pluginPath);
    QStringList pluginDirContents = pluginDir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    if (!pluginDirContents.contains(QStringLiteral("manifest.json"))) {
        qCDebug(KFILEMETADATA_LOG) << pluginPath << "does not seem to contain a valid plugin";
        return;
    }

    QFile manifest(pluginDir.filePath(QStringLiteral("manifest.json")));
    manifest.open(QIODevice::ReadOnly);
    QJsonDocument manifestDoc = QJsonDocument::fromJson(manifest.readAll());
    if (!manifestDoc.isObject()) {
        qCDebug(KFILEMETADATA_LOG) << "Manifest does not seem to be a valid JSON Object";
        return;
    }

    QJsonObject rootObject = manifestDoc.object();
    const QJsonArray mimetypesArray = rootObject.value(QStringLiteral("mimetypes")).toArray();
    QStringList mimetypes;
    mimetypes.reserve(mimetypesArray.count());

     for (const QJsonValue &mimetype : mimetypesArray) {
        mimetypes << mimetype.toString();
    }

    d->writeMimetypes.append(mimetypes);
    d->mainPath = pluginDir.absoluteFilePath(rootObject[QStringLiteral("main")].toString());
}

ExternalExtractor::~ExternalExtractor() = default;

QStringList ExternalExtractor::mimetypes() const
{
    Q_D(const ExternalExtractor);

    return d->writeMimetypes;
}

void ExternalExtractor::extract(ExtractionResult* result)
{
    Q_D(ExternalExtractor);

    QJsonDocument writeData;
    QJsonObject writeRootObject;
    QByteArray output;
    QByteArray errorOutput;

    writeRootObject[QStringLiteral("path")] = QJsonValue(result->inputUrl());
    writeRootObject[QStringLiteral("mimetype")] = result->inputMimetype();
    writeData.setObject(writeRootObject);

    QProcess extractorProcess;
    extractorProcess.start(d->mainPath, QStringList(), QIODevice::ReadWrite);
    bool started = extractorProcess.waitForStarted();
    if (!started) {
        qCWarning(KFILEMETADATA_LOG) << "External extractor" << d->mainPath
            << "failed to start:" << extractorProcess.errorString();
        return;
    }

    extractorProcess.write(writeData.toJson());
    extractorProcess.closeWriteChannel();
    extractorProcess.waitForFinished();

    output = extractorProcess.readAll();
    errorOutput = extractorProcess.readAllStandardError();

    if (extractorProcess.exitStatus()) {
        qCWarning(KFILEMETADATA_LOG) << "External extractor" << d->mainPath
            << "failed to index" << result->inputUrl() << "-" << errorOutput;
        return;
    }

    // now we read in the output (which is a standard json format) into the
    // ExtractionResult

    QJsonDocument extractorData = QJsonDocument::fromJson(output);
    if (!extractorData.isObject()) {
        return;
    }
    QJsonObject rootObject = extractorData.object();
    QJsonObject propertiesObject = rootObject[QStringLiteral("properties")].toObject();

    const auto propertiesObjectEnd = propertiesObject.constEnd();
    auto i = propertiesObject.constBegin();
    for (; i != propertiesObjectEnd; ++i) {
        if (i.key() == QStringLiteral("typeInfo")) {
            TypeInfo info = TypeInfo::fromName(i.value().toString());
            result->addType(info.type());
            continue;
        }

        // for plaintext extraction
        if (i.key() == QStringLiteral("text")) {
            result->append(i.value().toString());
            continue;
        }

        PropertyInfo info = PropertyInfo::fromName(i.key());
        if (info.name() != i.key()) {
            continue;
        }
        result->add(info.property(), i.value().toVariant());
    }

    if (rootObject[QStringLiteral("status")].toString() != QStringLiteral("OK")) {
        qCDebug(KFILEMETADATA_LOG) << rootObject[QStringLiteral("error")].toString();
    }
}

#include "moc_externalextractor.cpp"

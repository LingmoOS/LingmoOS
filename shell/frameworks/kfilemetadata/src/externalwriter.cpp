/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "externalwriter.h"
#include "properties.h"
#include "propertyinfo.h"
#include "kfilemetadata_debug.h"

#include <QDir>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#define WRITER_TIMEOUT_MS 30000

using namespace KFileMetaData;

class KFileMetaData::ExternalWriterPrivate
{
public:
    QString path;
    QStringList writeMimetypes;
    QString mainPath;
};


ExternalWriter::ExternalWriter(QObject* parent)
    : WriterPlugin(parent),
      d_ptr(new ExternalWriterPrivate)
{
}

ExternalWriter::ExternalWriter(const QString& pluginPath)
    : WriterPlugin(nullptr),
      d_ptr(new ExternalWriterPrivate)
{
    Q_D(ExternalWriter);
    d->path = pluginPath;

    QDir pluginDir(pluginPath);
    QStringList pluginDirContents = pluginDir.entryList();

    if (!pluginDirContents.contains(QStringLiteral("manifest.json"))) {
        qCDebug(KFILEMETADATA_LOG) << "Path does not seem to contain a valid plugin";
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
    for (const QJsonValue &mimetype : mimetypesArray) {
        mimetypes << mimetype.toString();
    }

    d->writeMimetypes.append(mimetypes);
    d->mainPath = pluginDir.absoluteFilePath(rootObject[QStringLiteral("main")].toString());
}

ExternalWriter::~ExternalWriter() = default;

QStringList ExternalWriter::writeMimetypes() const
{
    Q_D(const ExternalWriter);
    return d->writeMimetypes;
}

void ExternalWriter::write(const WriteData& data)
{
    Q_D(ExternalWriter);
    QJsonDocument writeData;
    QJsonObject rootObject;
    QJsonObject propertiesObject;
    QByteArray output;
    QByteArray errorOutput;

    const PropertyMultiMap properties = data.properties();

    for (auto i = properties.constBegin(); i != properties.constEnd(); ++i) {
        PropertyInfo propertyInfo(i.key());
        propertiesObject[propertyInfo.name()] = QJsonValue::fromVariant(i.value());
    }

    rootObject[QStringLiteral("path")] = QJsonValue(data.inputUrl());
    rootObject[QStringLiteral("mimetype")] = data.inputMimetype();
    rootObject[QStringLiteral("properties")] = propertiesObject;
    writeData.setObject(rootObject);

    QProcess writerProcess;
    writerProcess.start(d->mainPath, QStringList(), QIODevice::ReadWrite);
    writerProcess.write(writeData.toJson());
    writerProcess.closeWriteChannel();
    writerProcess.waitForFinished(WRITER_TIMEOUT_MS);

    errorOutput = writerProcess.readAllStandardError();

    if (writerProcess.exitStatus()) {
        qCDebug(KFILEMETADATA_LOG) << "Something went wrong while trying to write data";
        qCDebug(KFILEMETADATA_LOG) << errorOutput;
        return;
    }

    output = writerProcess.readAll();

    QJsonDocument writerExitData = QJsonDocument::fromJson(output);
    if (!writerExitData.isObject()) {
        return;
    }
    QJsonObject outputRootObject = writerExitData.object();

    if (outputRootObject[QStringLiteral("status")].toString() != QStringLiteral("OK")) {
        qCDebug(KFILEMETADATA_LOG) << outputRootObject[QStringLiteral("error")].toString();
        qCDebug(KFILEMETADATA_LOG) << errorOutput;
    }

}

#include "moc_externalwriter.cpp"

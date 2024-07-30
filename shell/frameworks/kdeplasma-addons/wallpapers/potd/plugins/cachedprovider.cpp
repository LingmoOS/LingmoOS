/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "cachedprovider.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThreadPool>

#include "debug.h"

LoadImageDataThread::LoadImageDataThread(const QString &filePath)
    : m_localPath(filePath)
{
}

void LoadImageDataThread::run()
{
    PotdProviderData data;
    data.localPath = m_localPath;

    QFile infoFile(m_localPath + QStringLiteral(".json"));

    if (infoFile.exists()) {
        if (infoFile.open(QIODevice::ReadOnly)) {
            QJsonParseError jsonParseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(infoFile.readAll(), &jsonParseError);
            infoFile.close();

            if (jsonParseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                const QJsonObject jsonObject = jsonDoc.object();
                data.infoUrl = QUrl(jsonObject.value(QLatin1String("InfoUrl")).toString());
                data.remoteUrl = QUrl(jsonObject.value(QLatin1String("RemoteUrl")).toString());
                data.title = jsonObject.value(QLatin1String("Title")).toString();
                data.author = jsonObject.value(QLatin1String("Author")).toString();
            } else {
                qCWarning(WALLPAPERPOTD) << "Failed to read the wallpaper information!";
            }
        } else {
            qCWarning(WALLPAPERPOTD) << "Failed to open the wallpaper information file!";
        }
    }

    Q_EMIT done(data);
}

SaveImageThread::SaveImageThread(const QString &identifier, const QVariantList &args, const PotdProviderData &data)
    : m_identifier(identifier)
    , m_args(args)
    , m_data(data)
{
}

void SaveImageThread::run()
{
    const QString localPath = CachedProvider::identifierToPath(m_identifier, m_args);
    m_data.image.save(localPath, "JPEG");

    QFile infoFile(localPath + QStringLiteral(".json"));
    if (infoFile.open(QIODevice::WriteOnly)) {
        QJsonObject jsonObject;

        jsonObject.insert(QLatin1String("InfoUrl"), m_data.infoUrl.url());
        jsonObject.insert(QLatin1String("RemoteUrl"), m_data.remoteUrl.url());
        jsonObject.insert(QLatin1String("Title"), m_data.title);
        jsonObject.insert(QLatin1String("Author"), m_data.author);

        infoFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
        infoFile.close();
    } else {
        qCWarning(WALLPAPERPOTD) << "Failed to save the wallpaper information!";
    }

    Q_EMIT done(localPath);
}

QString CachedProvider::identifierToPath(const QString &identifier, const QVariantList &args)
{
    const QString argString = std::accumulate(args.cbegin(), args.cend(), QString(), [](const QString &s, const QVariant &arg) {
        if (arg.canConvert(QMetaType(QMetaType::QString))) {
            return s + QStringLiteral(":%1").arg(arg.toString());
        }

        return s;
    });

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/plasma_engine_potd/");
    QDir d;
    d.mkpath(dataDir);
    return QStringLiteral("%1%2%3").arg(dataDir, identifier, argString);
}

CachedProvider::CachedProvider(const QString &identifier, const QVariantList &args, QObject *parent)
    : PotdProvider(parent, KPluginMetaData(), QVariantList())
    , mIdentifier(identifier)
    , m_args(args)
{
    LoadImageDataThread *thread = new LoadImageDataThread(CachedProvider::identifierToPath(mIdentifier, m_args));
    connect(thread, &LoadImageDataThread::done, this, &CachedProvider::slotFinished);
    QThreadPool::globalInstance()->start(thread);
}

QString CachedProvider::identifier() const
{
    return mIdentifier;
}

QString CachedProvider::localPath() const
{
    return m_localPath;
}

void CachedProvider::slotFinished(const PotdProviderData &data)
{
    m_localPath = data.localPath;
    m_infoUrl = data.infoUrl;
    m_remoteUrl = data.remoteUrl;
    m_title = data.title;
    m_author = data.author;

    Q_EMIT finished(this, QImage());
}

bool CachedProvider::isCached(const QString &identifier, const QVariantList &args, bool ignoreAge)
{
    const QString path = CachedProvider::identifierToPath(identifier, args);
    if (!QFileInfo::exists(path)) {
        return false;
    }

    QRegularExpression re(QLatin1String(":\\d{4}-\\d{2}-\\d{2}"));

    if (!ignoreAge && !re.match(identifier).hasMatch()) {
        // no date in the identifier, so it's a daily; check to see if the modification time is today
        QFileInfo info(path);
        if (info.lastModified().daysTo(QDateTime::currentDateTime()) >= 1) {
            return false;
        }
    }

    return true;
}

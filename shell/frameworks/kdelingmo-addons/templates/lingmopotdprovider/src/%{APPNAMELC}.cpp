/*
 *   SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "%{APPNAMELC}.h"

// KF
#include <KPluginFactory>
#include <KIO/Job>


%{APPNAME}::%{APPNAME}(QObject *parent, const QVariantList &args)
    : PotdProvider(parent, args)
{
    // TODO: replace with url to data about what the current picture of the day is
    const QUrl potdFeed(QStringLiteral("https://kde.org"));

    KIO::StoredTransferJob* job = KIO::storedGet(potdFeed, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished,
            this, &%{APPNAME}::handleFinishedFeedRequest);
}

%{APPNAME}::~%{APPNAME}()
{
}

void %{APPNAME}::handleFinishedFeedRequest(KJob *job)
{
    KIO::StoredTransferJob *requestJob = static_cast<KIO::StoredTransferJob*>(job);
    if (requestJob->error()) {
        Q_EMIT error(this);
        return;
    }

    // TODO: read url to image from requestJob->data()
    const QUrl picureUrl(QStringLiteral("https://techbase.kde.org/favicon.png"));

    KIO::StoredTransferJob *imageJob = KIO::storedGet(picureUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished,
            this, &%{APPNAME}::handleFinishedImageRequest);
}

void %{APPNAME}::handleFinishedImageRequest(KJob *job)
{
    KIO::StoredTransferJob *requestJob = static_cast<KIO::StoredTransferJob*>(job);
    if (requestJob->error()) {
        Q_EMIT error(this);
        return;
    }

    potdProviderData()->wallpaperImage = QImage::fromData(requestJob->data());

    if (potdProviderData()->wallpaperImage.isNull()) {
        Q_EMIT error(this);
        return;
    }

    Q_EMIT finished(this);
}


K_PLUGIN_CLASS_WITH_JSON(%{APPNAME}, "%{APPNAMELC}.json")

#include "%{APPNAMELC}.moc"

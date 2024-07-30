/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "epodprovider.h"

#include <QRegularExpression>
#include <QTextDocumentFragment>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

EpodProvider::EpodProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    const QUrl url(QStringLiteral("https://epod.usra.edu/blog/"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &EpodProvider::pageRequestFinished);
}

void EpodProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QString data = QString::fromUtf8(job->data()).simplified();

    const QString pattern = QStringLiteral("://epod.usra.edu/.a/*-pi");
    const auto exp = QRegularExpression(QRegularExpression::wildcardToRegularExpression(pattern));
    const auto expMatch = exp.match(data);

    if (expMatch.hasMatch()) {
        /**
         * Match link and title
         * Example:
         * <h3 class="entry-header">
         * <a href="https://epod.usra.edu/blog/2022/01/archive-panamint-delta-.html">Archive - Panamint Delta </a>
         * </h3><!-- .entry-header -->
         */
        const QRegularExpression titleRegEx(QStringLiteral("<h3 class=\"entry-header\">.*?<a href=\"(.+?)\">(.+?)</a>.*?</h3>"));
        const QRegularExpressionMatch titleMatch = titleRegEx.match(data);
        if (titleMatch.hasMatch()) {
            m_infoUrl = QUrl(titleMatch.captured(1).trimmed());
            m_title = QTextDocumentFragment::fromHtml(titleMatch.captured(2).trimmed()).toPlainText();
        }

        /**
         * Match author
         * Example:
         * <strong>Photographer</strong>: <a href="...">Wendy Van Norden</a>
         */
        const QRegularExpression authorRegEx(QStringLiteral("<strong>Photographer.*?</strong>.*?<a.+?>(.+?)</a>"));
        const QRegularExpressionMatch authorMatch = authorRegEx.match(data);
        if (authorMatch.hasMatch()) {
            m_author = QTextDocumentFragment::fromHtml(authorMatch.captured(1).trimmed()).toPlainText();
        }
    } else {
        Q_EMIT error(this);
        return;
    }

    int pos = expMatch.capturedStart() + pattern.length();
    const QString sub = data.mid(pos - 4, pattern.length() + 10);
    m_remoteUrl = QUrl(QStringLiteral("https://epod.usra.edu/.a/%1-pi").arg(sub));

    KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &EpodProvider::imageRequestFinished);
}

void EpodProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    // FIXME: this really should be done in a thread as this can block
    Q_EMIT finished(this, QImage::fromData(job->data()));
}

K_PLUGIN_CLASS_WITH_JSON(EpodProvider, "epodprovider.json")

#include "epodprovider.moc"

/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comicprovider.h"
#include "comic_debug.h"

#include <QTimer>
#include <QUrl>

#include <KIO/MimetypeJob>
#include <KIO/StoredTransferJob>

class ComicProvider::Private
{
public:
    Private(ComicProvider *parent, const KPluginMetaData &data, IdentifierType suffixType)
        : mParent(parent)
        , mIsCurrent(false)
        , mFirstStripNumber(1)
        , mComicDescription(data)
        , mSuffixType(suffixType)
    {
        mTimer = new QTimer(parent);
        mTimer->setSingleShot(true);
        mTimer->setInterval(60000); // timeout after 1 minute
        connect(mTimer, &QTimer::timeout, mParent, [this]() {
            // operation took too long, abort it
            Q_EMIT mParent->error(mParent);
        });
    }

    void jobDone(KJob *job)
    {
        if (job->error()) {
            mParent->pageError(job->property("uid").toInt(), job->errorText());
        } else {
            KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob *>(job);
            mParent->pageRetrieved(job->property("uid").toInt(), storedJob->data());
        }
    }

    void slotRedirection(KIO::Job *job, const QUrl &oldUrl, const QUrl &newUrl)
    {
        Q_UNUSED(oldUrl)

        mParent->redirected(job->property("uid").toInt(), newUrl);
        mRedirections.remove(job);
    }

    void slotRedirectionDone(KJob *job)
    {
        if (job->error()) {
            qCDebug(PLASMA_COMIC) << "Redirection job with id" << job->property("uid").toInt() << "finished with an error.";
        }

        if (mRedirections.contains(job)) {
            // no redirection took place, return the original url
            mParent->redirected(job->property("uid").toInt(), mRedirections[job]);
            mRedirections.remove(job);
        }
    }

    ComicProvider *mParent;
    QString mRequestedId;
    QString mRequestedComicName;
    QString mComicAuthor;
    QUrl mImageUrl;
    bool mIsCurrent;
    bool mIsLeftToRight;
    bool mIsTopToBottom;
    QDate mRequestedDate;
    QDate mFirstStripDate;
    int mRequestedNumber;
    int mFirstStripNumber;
    const KPluginMetaData mComicDescription;
    QTimer *mTimer;
    QHash<KJob *, QUrl> mRedirections;
    const IdentifierType mSuffixType;
};

ComicProvider::ComicProvider(QObject *parent, const KPluginMetaData &data, IdentifierType type, const QVariant &identifier)
    : QObject(parent)
    , d(new Private(this, data, type))
{
    if (type == IdentifierType::DateIdentifier) {
        d->mRequestedDate = identifier.toDate();
    } else if (type == IdentifierType::NumberIdentifier) {
        d->mRequestedNumber = identifier.toInt();
    } else if (type == IdentifierType::StringIdentifier) {
        d->mRequestedId = identifier.toString();

        int index = d->mRequestedId.indexOf(QLatin1Char(':'));
        d->mRequestedComicName = d->mRequestedId.mid(0, index);
    } else {
        qFatal("Invalid type passed to comic provider");
    }

    d->mTimer->start();
    connect(this, &ComicProvider::finished, this, [this]() {
        // everything finished, stop the timeout timer
        d->mTimer->stop();
    });
}

ComicProvider::~ComicProvider()
{
    delete d;
}

QString ComicProvider::nextIdentifier() const
{
    if (identifierType() == IdentifierType::DateIdentifier && d->mRequestedDate != QDate::currentDate()) {
        return d->mRequestedDate.addDays(1).toString(Qt::ISODate);
    }

    return QString();
}

QString ComicProvider::previousIdentifier() const
{
    if ((identifierType() == IdentifierType::DateIdentifier) && (!firstStripDate().isValid() || d->mRequestedDate > firstStripDate())) {
        return d->mRequestedDate.addDays(-1).toString(Qt::ISODate);
    }

    return QString();
}

QString ComicProvider::stripTitle() const
{
    return QString();
}

QString ComicProvider::additionalText() const
{
    return QString();
}

void ComicProvider::setIsCurrent(bool value)
{
    d->mIsCurrent = value;
}

bool ComicProvider::isCurrent() const
{
    return d->mIsCurrent;
}

QDate ComicProvider::requestedDate() const
{
    return d->mRequestedDate;
}

QDate ComicProvider::firstStripDate() const
{
    return d->mFirstStripDate;
}

QString ComicProvider::comicAuthor() const
{
    return d->mComicAuthor;
}

void ComicProvider::setComicAuthor(const QString &author)
{
    d->mComicAuthor = author;
}

void ComicProvider::setFirstStripDate(const QDate &date)
{
    d->mFirstStripDate = date;
}

int ComicProvider::firstStripNumber() const
{
    return d->mFirstStripNumber;
}

void ComicProvider::setFirstStripNumber(int number)
{
    d->mFirstStripNumber = number;
}

QString ComicProvider::firstStripIdentifier() const
{
    if ((identifierType() == IdentifierType::DateIdentifier) && d->mFirstStripDate.isValid()) {
        return d->mFirstStripDate.toString(Qt::ISODate);
    } else if (identifierType() == IdentifierType::NumberIdentifier) {
        return QString::number(d->mFirstStripNumber);
    }

    return QString();
}

int ComicProvider::requestedNumber() const
{
    return d->mRequestedNumber;
}

QString ComicProvider::requestedString() const
{
    return d->mRequestedId;
}

QString ComicProvider::requestedComicName() const
{
    return d->mRequestedComicName;
}

void ComicProvider::requestPage(const QUrl &url, int id, const MetaInfos &infos)
{
    qCDebug(PLASMA_COMIC) << "Requested page" << url << "with id" << id << "and additional metadata" << infos;
    // each request restarts the timer
    d->mTimer->start();

    if (id == Image) {
        d->mImageUrl = url;
    }

    KIO::StoredTransferJob *job;
    if (id == Image) {
        // use cached information for the image if available
        job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    } else {
        // for webpages we always reload, making sure, that changes are recognised
        job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    }
    job->setProperty("uid", id);
    connect(job, &KJob::result, this, [this](KJob *job) {
        d->jobDone(job);
    });

    if (!infos.isEmpty()) {
        QMapIterator<QString, QString> it(infos);
        while (it.hasNext()) {
            it.next();
            job->addMetaData(it.key(), it.value());
        }
    }
}

void ComicProvider::requestRedirectedUrl(const QUrl &url, int id, const MetaInfos &infos)
{
    // each request restarts the timer
    d->mTimer->start();

    KIO::MimetypeJob *job = KIO::mimetype(url, KIO::HideProgressInfo);
    job->setProperty("uid", id);
    d->mRedirections[job] = url;
    connect(job, &KIO::MimetypeJob::redirection, this, [this](KIO::Job *job, const QUrl &newUrl) {
        d->slotRedirection(job, QUrl(), newUrl);
    });
    connect(job, &KIO::MimetypeJob::permanentRedirection, this, [this](KIO::Job *job, const QUrl &oldUrl, const QUrl &newUrl) {
        d->slotRedirection(job, oldUrl, newUrl);
    });
    connect(job, &KIO::MimetypeJob::result, this, [this](KJob *job) {
        d->slotRedirectionDone(job);
    });

    if (!infos.isEmpty()) {
        QMapIterator<QString, QString> it(infos);
        while (it.hasNext()) {
            it.next();
            job->addMetaData(it.key(), it.value());
        }
    }
}

void ComicProvider::pageRetrieved(int, const QByteArray &)
{
}

void ComicProvider::pageError(int, const QString &)
{
}

void ComicProvider::redirected(int, const QUrl &)
{
}

QString ComicProvider::pluginName() const
{
    return d->mComicDescription.pluginId();
}

QString ComicProvider::name() const
{
    return d->mComicDescription.name();
}

KPluginMetaData ComicProvider::description() const
{
    return d->mComicDescription;
}

QUrl ComicProvider::shopUrl() const
{
    return QUrl();
}

QUrl ComicProvider::imageUrl() const
{
    return d->mImageUrl;
}

bool ComicProvider::isLeftToRight() const
{
    return true;
}

bool ComicProvider::isTopToBottom() const
{
    return true;
}

#include "moc_comicprovider.cpp"

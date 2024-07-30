// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
// SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include "entry.h"

namespace KNSCore
{

class DownloadLinkInformationV2Private
{
public:
    QString name;
    QString priceAmount;
    QString distributionType;
    QString descriptionLink;
    int id = -1;
    bool isDownloadtypeLink = false;
    quint64 size = 0;
    QStringList tags;
    QString version;
};

class EntryPrivate : public QSharedData
{
public:
    EntryPrivate()
    {
        qRegisterMetaType<KNSCore::Entry::List>();
    }

    bool operator==(const EntryPrivate &other) const
    {
        return mUniqueId == other.mUniqueId && mProviderId == other.mProviderId;
    }

    QString mUniqueId;
    QString mRequestedUniqueId; // We need to map the entry to the request in the ResultsStream, but invalid entries would have an empty ID
    QString mName;
    QUrl mHomepage;
    QString mCategory;
    QString mLicense;
    QString mVersion;
    QDate mReleaseDate = QDate::currentDate();

    // Version and date if a newer version is found (updateable)
    QString mUpdateVersion;
    QDate mUpdateReleaseDate;

    Author mAuthor;
    int mRating = 0;
    int mNumberOfComments = 0;
    int mDownloadCount = 0;
    int mNumberFans = 0;
    int mNumberKnowledgebaseEntries = 0;
    QString mKnowledgebaseLink;
    QString mSummary;
    QString mShortSummary;
    QString mChangelog;
    QString mPayload;
    QStringList mInstalledFiles;
    QString mProviderId;
    QStringList mUnInstalledFiles;
    QString mDonationLink;
    QStringList mTags;

    QString mChecksum;
    QString mSignature;
    KNSCore::Entry::Status mStatus = Entry::Invalid;
    Entry::Source mSource = Entry::Online;
    Entry::EntryType mEntryType = Entry::CatalogEntry;

    QString mPreviewUrl[6];
    QImage mPreviewImage[6];

    QList<DownloadLinkInformationV2Private> mDownloadLinkInformationList;
};

} // namespace KNSCore

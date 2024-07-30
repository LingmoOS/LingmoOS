/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicdata.h"

#include <KLocalizedString>
#include <Plasma/Theme>
// Qt
#include <QPainter>

ComicData::ComicData()
{
}

void ComicData::init(const QString &id, const KConfigGroup &config)
{
    mId = id;
    mCfg = config;
    load();
}

void ComicData::load()
{
    mScaleComic = mCfg.readEntry(QLatin1String("scaleToContent_") + mId, false);
    mMaxStripNum = mCfg.readEntry(QLatin1String("maxStripNum_") + mId, 0);
    mStored = mCfg.readEntry(QLatin1String("storedPosition_") + mId, QString());
}

void ComicData::save()
{
    mCfg.writeEntry(QLatin1String("scaleToContent_") + mId, mScaleComic);
    mCfg.writeEntry(QLatin1String("maxStripNum_") + mId, mMaxStripNum);
    mCfg.writeEntry(QLatin1String("storedPosition_") + id(), mStored);

    // no next, thus the most recent strip
    if (!hasNext()) {
        mCfg.writeEntry(QLatin1String("lastStripVisited_") + mId, true);
        mCfg.writeEntry(QLatin1String("lastStrip_") + mId, mLast);
    }
}

void ComicData::setScaleComic(bool scale)
{
    mScaleComic = scale;
    save();
}

void ComicData::storePosition(bool store)
{
    mStored = (store ? mCurrent : QString());
    save();
}

void ComicData::setData(const ComicMetaData &data)
{
    if (!data.error) {
        mImage = data.image;
        mPrev = data.previousIdentifier;
        mNext = data.nextIdentifier;
        mAdditionalText = data.additionalText;
        mReady = true;
    }

    mWebsiteUrl = data.websiteUrl;
    mImageUrl = data.imageUrl;
    mShopUrl = data.shopUrl;
    mFirst = data.firstStripIdentifier;
    mStripTitle = data.stripTitle;
    mAuthor = data.comicAuthor;
    mTitle = data.providerName;
    mType = data.identifierType;

    QString temp = data.identifier;
    mCurrent = temp.remove(mId + QLatin1Char(':'));

    // found a new last identifier
    if (!hasNext()) {
        mLast = mCurrent;
    }

    mCurrentReadable.clear();
    if (mType == IdentifierType::NumberIdentifier) {
        mCurrentReadable = i18nc("an abbreviation for Number", "# %1", mCurrent);
        int tempNum = mCurrent.toInt();
        if (mMaxStripNum < tempNum) {
            mMaxStripNum = tempNum;
        }

        temp = mFirst.remove(mId + QLatin1Char(':'));
        mFirstStripNum = temp.toInt();
    } else if (mType == IdentifierType::DateIdentifier && QDate::fromString(temp, QStringLiteral("yyyy-MM-dd")).isValid()) {
        mCurrentReadable = mCurrent;
    } else if (mType == IdentifierType::StringIdentifier) {
        mCurrentReadable = mCurrent;
    }

    mIsLeftToRight = data.isLeftToRight;
    mIsTopToBottom = data.isTopToBottom;

    save();
}

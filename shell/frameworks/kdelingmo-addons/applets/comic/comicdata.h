/*
 *   SPDX-FileCopyrightText: 2012 Matthias Fuchs <mat69@gmx.net>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMIC_DATA_H
#define COMIC_DATA_H

#include "comicinfo.h"
#include "engine/types.h"

// Qt
#include <KConfigGroup>
#include <QImage>
#include <QString>
#include <QUrl>

class ComicData
{
public:
    ComicData();

    void init(const QString &id, const KConfigGroup &config);

    void setData(const ComicMetaData &data);

    IdentifierType type() const
    {
        return mType;
    }

    /**
     * The identifier of the comic, e.g. "garfield"
     */
    QString id() const
    {
        return mId;
    }

    /**
     * The stored comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString stored() const
    {
        return mStored;
    }

    void storePosition(bool store);

    /**
     * The previous comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString prev() const
    {
        return mPrev;
    }

    /**
     * The current comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString current() const
    {
        return mCurrent;
    }

    /**
     * The next comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString next() const
    {
        return mNext;
    }

    QString currentReadable() const
    {
        return mCurrentReadable;
    }

    /**
     * The first comic e.g. "2007-12-21" for a comic of the Date type
     */
    QString first() const
    {
        return mFirst;
    }

    bool hasNext() const
    {
        return !mNext.isEmpty();
    }

    bool hasPrev() const
    {
        return !mPrev.isEmpty();
    }

    bool hasFirst() const
    {
        return !mFirst.isEmpty();
    }

    bool hasStored() const
    {
        return !mStored.isEmpty();
    }

    bool hasImage() const
    {
        return !mImage.isNull();
    }

    QString additionalText() const
    {
        return mAdditionalText;
    }

    QString title() const
    {
        return mTitle;
    }
    void setTitle(const QString &title)
    {
        mTitle = title;
    }

    QString stripTitle() const
    {
        return mStripTitle;
    }

    QUrl websiteUrl() const
    {
        return mWebsiteUrl;
    }

    QUrl imageUrl() const
    {
        return mImageUrl;
    }

    QUrl shopUrl() const
    {
        return mShopUrl;
    }

    QString author() const
    {
        return mAuthor;
    }

    QImage image() const
    {
        return mImage;
    }

    bool scaleComic() const
    {
        return mScaleComic;
    }

    bool isLeftToRight() const
    {
        return mIsLeftToRight;
    }

    bool isTopToBottom() const
    {
        return mIsTopToBottom;
    }

    bool storePosition() const
    {
        return !mStored.isEmpty();
    }

    void setScaleComic(bool scale);

    QString errorStrip() const
    {
        return mErrorStrip;
    }

    int firstStripNum() const
    {
        return mFirstStripNum;
    }

    int maxStripNum() const
    {
        return mMaxStripNum;
    }

    bool ready() const
    {
        return mReady;
    }

    void save();

private:
    void load();

private:
    IdentifierType mType;
    QString mId;
    QString mFirst;
    QString mLast;
    QString mCurrent;
    QString mNext;
    QString mPrev;
    QString mStored;
    QString mCurrentReadable;

    QString mErrorStrip;

    QString mAuthor;
    QString mTitle;
    QString mStripTitle;
    QString mAdditionalText;
    QUrl mWebsiteUrl;
    QUrl mImageUrl;
    QUrl mShopUrl;

    QImage mImage;

    // only applicable if the comic is of type Number
    int mFirstStripNum = 0;
    int mMaxStripNum = 0;

    bool mScaleComic = false;
    bool mIsLeftToRight = false;
    bool mIsTopToBottom = false;
    bool mReady = false;

    KConfigGroup mCfg;
};

#endif

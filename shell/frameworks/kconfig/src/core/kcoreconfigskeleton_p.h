/*
    This file is part of KOrganizer.
    SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCORECONFIGSKELETON_P_H
#define KCORECONFIGSKELETON_P_H

#include "kcoreconfigskeleton.h"

class KCoreConfigSkeletonPrivate
{
public:
    KCoreConfigSkeletonPrivate()
        : mCurrentGroup(QStringLiteral("No Group"))
        , mUseDefaults(false)
    {
    }
    ~KCoreConfigSkeletonPrivate()
    {
        qDeleteAll(mItems);
    }
    QString mCurrentGroup;

    KSharedConfig::Ptr mConfig; // pointer to KConfig object

    KConfigSkeletonItem::List mItems;
    KConfigSkeletonItem::Dict mItemDict;

    bool mUseDefaults;
};

class KConfigSkeletonItemPrivate
{
public:
    KConfigSkeletonItemPrivate()
        : mIsImmutable(true)
        , mWriteFlags(KConfigBase::Normal)
    {
    }
    virtual ~KConfigSkeletonItemPrivate();
    bool mIsImmutable; ///< Indicates this item is immutable
    KConfigBase::WriteConfigFlags mWriteFlags; ///< The flags to pass to calls of writeEntry() and revertToDefault()

    QString mLabel; ///< The label for this item
    QString mToolTip; ///< The ToolTip text for this item
    QString mWhatsThis; ///< The What's This text for this item
    KConfigGroup mConfigGroup; ///< KConfigGroup, allow to read/write item in nested groups

    // HACK: Necessary to avoid introducing new virtuals in KConfigSkeletonItem
    std::function<bool()> mIsDefaultImpl;
    std::function<bool()> mIsSaveNeededImpl;
    std::function<QVariant()> mGetDefaultImpl;
};

class KPropertySkeletonItemPrivate : public KConfigSkeletonItemPrivate
{
public:
    KPropertySkeletonItemPrivate(QObject *object, const QByteArray &propertyName, const QVariant &defaultValue)
        : KConfigSkeletonItemPrivate()
        , mObject(object)
        , mPropertyName(propertyName)
        , mDefaultValue(defaultValue)
        , mConstDefaultValue(defaultValue)
    {
        mIsImmutable = false;
    }

    QObject *mObject;
    const QByteArray mPropertyName;
    QVariant mDefaultValue;
    const QVariant mConstDefaultValue;
    QVariant mReference;
    QVariant mLoadedValue;
    std::function<void()> mNotifyFunction;
};

#endif

/*
    This file is part of KOrganizer.
    SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigskeleton.h"

#include <kcoreconfigskeleton_p.h>

KConfigSkeleton::KConfigSkeleton(const QString &configname, QObject *parent)
    : KCoreConfigSkeleton(configname, parent)
{
}

KConfigSkeleton::KConfigSkeleton(KSharedConfig::Ptr pConfig, QObject *parent)
    : KCoreConfigSkeleton(std::move(pConfig), parent)
{
}

KConfigSkeleton::ItemColor::ItemColor(const QString &_group, const QString &_key, QColor &reference, const QColor &defaultValue)
    : KConfigSkeletonGenericItem<QColor>(_group, _key, reference, defaultValue)
{
}

void KConfigSkeleton::ItemColor::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);

    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KConfigSkeleton::ItemColor::setProperty(const QVariant &p)
{
    mReference = qvariant_cast<QColor>(p);
}

bool KConfigSkeleton::ItemColor::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QColor>(v);
}

QVariant KConfigSkeleton::ItemColor::property() const
{
    return QVariant(mReference);
}

KConfigSkeleton::ItemFont::ItemFont(const QString &_group, const QString &_key, QFont &reference, const QFont &defaultValue)
    : KConfigSkeletonGenericItem<QFont>(_group, _key, reference, defaultValue)
{
}

void KConfigSkeleton::ItemFont::readConfig(KConfig *config)
{
    KConfigGroup cg = configGroup(config);

    mReference = cg.readEntry(mKey, mDefault);
    mLoadedValue = mReference;

    readImmutability(cg);
}

void KConfigSkeleton::ItemFont::setProperty(const QVariant &p)
{
    mReference = qvariant_cast<QFont>(p);
}

bool KConfigSkeleton::ItemFont::isEqual(const QVariant &v) const
{
    return mReference == qvariant_cast<QFont>(v);
}

QVariant KConfigSkeleton::ItemFont::property() const
{
    return QVariant(mReference);
}

KConfigSkeleton::ItemColor *KConfigSkeleton::addItemColor(const QString &name, QColor &reference, const QColor &defaultValue, const QString &key)
{
    KConfigSkeleton::ItemColor *item;
    item = new KConfigSkeleton::ItemColor(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

KConfigSkeleton::ItemFont *KConfigSkeleton::addItemFont(const QString &name, QFont &reference, const QFont &defaultValue, const QString &key)
{
    KConfigSkeleton::ItemFont *item;
    item = new KConfigSkeleton::ItemFont(d->mCurrentGroup, key.isNull() ? name : key, reference, defaultValue);
    addItem(item, name);
    return item;
}

#include "moc_kconfigskeleton.cpp"

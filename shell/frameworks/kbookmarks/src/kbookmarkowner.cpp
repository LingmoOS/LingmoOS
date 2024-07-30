// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>
    SPDX-FileCopyrightText: 2008 Norbert Frese <nf2@scheinwelt.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkowner.h"

class Q_DECL_HIDDEN KBookmarkOwner::FutureBookmarkPrivate : public QSharedData
{
public:
    QString title;
    QUrl url;
    QString icon;
};

KBookmarkOwner::FutureBookmark::FutureBookmark(const QString &title, const QUrl &url, const QString &icon)
    : d(new FutureBookmarkPrivate)
{
    d->title = title;
    d->url = url;
    d->icon = icon;
}

KBookmarkOwner::FutureBookmark::FutureBookmark(const FutureBookmark &other)
    : d(other.d)
{
}

KBookmarkOwner::FutureBookmark &KBookmarkOwner::FutureBookmark::operator=(const FutureBookmark &other)
{
    d = other.d;
    return *this;
}

KBookmarkOwner::FutureBookmark::~FutureBookmark()
{
}

QString KBookmarkOwner::FutureBookmark::title() const
{
    return d->title;
}

QUrl KBookmarkOwner::FutureBookmark::url() const
{
    return d->url;
}

QString KBookmarkOwner::FutureBookmark::icon() const
{
    return d->icon;
}

KBookmarkOwner::KBookmarkOwner()
    : d(nullptr)
{
}

KBookmarkOwner::~KBookmarkOwner() = default;

bool KBookmarkOwner::enableOption(BookmarkOption action) const
{
    if (action == ShowAddBookmark) {
        return true;
    }
    if (action == ShowEditBookmark) {
        return true;
    }
    return false;
}

void KBookmarkOwner::openFolderinTabs(const KBookmarkGroup &)
{
}

void KBookmarkOwner::openInNewTab(const KBookmark &bm)
{
    // fallback
    openBookmark(bm, Qt::LeftButton, Qt::NoModifier);
}

void KBookmarkOwner::openInNewWindow(const KBookmark &bm)
{
    // fallback
    openBookmark(bm, Qt::LeftButton, Qt::NoModifier);
}

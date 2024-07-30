/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000, 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KBOOKMARK_OWNER_H
#define KBOOKMARK_OWNER_H

#include "kbookmark.h"

#include <QSharedDataPointer>
#include <QString>

class QWidget;

class KBookmarkGroup;

/**
 * @class KBookmarkOwner kbookmarkowner.h KBookmarkOwner
 *
 * The KBookmarkMenu and KBookmarkBar classes gives the user
 * the ability to either edit bookmarks or add their own.  In the
 * first case, the app may want to open the bookmark in a special way.
 * In the second case, the app @em must supply the name and the
 * URL for the bookmark.
 *
 * This class gives the app this callback-like ability.
 *
 * If your app does not give the user the ability to add bookmarks and
 * you don't mind using the default bookmark editor to edit your
 * bookmarks, then you don't need to overload this class at all.
 * Rather, just use something like:
 *
 * @code
 * bookmarks = new KBookmarkMenu(manager, nullptr, menu);
 * @endcode
 *
 * If you wish to use your own editor or allow the user to add
 * bookmarks, you must overload this class.
 */
class KBOOKMARKS_EXPORT KBookmarkOwner
{
public:
    KBookmarkOwner();
    virtual ~KBookmarkOwner();

    /**
     * This function is called whenever the user wants to add the
     * current page to the bookmarks list.  The title will become the
     * "name" of the bookmark.  You must overload this function if you
     * wish to give your users the ability to add bookmarks.
     * The default returns an empty string.
     *
     * @return the title of the current page.
     */
    virtual QString currentTitle() const
    {
        return QString();
    }

    /**
     * This function is called whenever the user wants to add the
     * current page to the bookmarks list.  The URL will become the URL
     * of the bookmark.  You must overload this function if you wish to
     * give your users the ability to add bookmarks.
     * The default returns an empty string.
     *
     * @return the URL of the current page.
     * Since 5.0 this method returns a QUrl. While porting it, remember to implement currentIcon too.
     */
    virtual QUrl currentUrl() const
    {
        return QUrl();
    }

    /**
     * This function is called whenever the user wants to add the
     * current page to the bookmarks list.  The icon will become the icon
     * of the bookmark.  You must overload this function if you wish to
     * give your users the ability to add bookmarks.
     * The default returns an empty string.
     *
     * A very common implementation for this method is
     * return KIO::iconNameForUrl(currentUrl());
     *
     * @return the icon name of the current page.
     * @since 5.0
     */
    virtual QString currentIcon() const
    {
        return QString();
    }

    /**
     * This function returns whether the owner supports tabs.
     * The default returns @c false.
     */
    virtual bool supportsTabs() const
    {
        return false;
    }

    class FutureBookmarkPrivate;
    /**
     * Represents the data for a bookmark that will be added.
     * @since 5.0
     */
    class KBOOKMARKS_EXPORT FutureBookmark
    {
    public:
        FutureBookmark(const QString &title, const QUrl &url, const QString &icon);
        ~FutureBookmark();
        FutureBookmark(const FutureBookmark &other);
        FutureBookmark &operator=(const FutureBookmark &other);

        QString title() const;
        QUrl url() const;
        QString icon() const;

    private:
        QSharedDataPointer<FutureBookmarkPrivate> d;
    };

    /**
     * Returns a list of bookmark data for the open tabs.
     * The default returns an empty list.
     */
    virtual QList<FutureBookmark> currentBookmarkList() const
    {
        return QList<FutureBookmark>();
    }

    enum BookmarkOption { ShowAddBookmark, ShowEditBookmark };

    /** Returns true if \p action should be shown in the menu
     *  The default is to show both a add and editBookmark Entry
     *  //TODO ContextMenuAction? to disable the contextMenu?
     *         Delete and Properties to disable those in the
     *         context menu?
     */
    virtual bool enableOption(BookmarkOption option) const;

    /**
     * Called if a bookmark is selected. You need to override this.
     */
    virtual void openBookmark(const KBookmark &bm, Qt::MouseButtons mb, Qt::KeyboardModifiers km) = 0;

    /**
     * Called if the user wants to open every bookmark in this folder in a new tab.
     * The default implementation does nothing.
     * This is only called if supportsTabs() returns true
     */
    virtual void openFolderinTabs(const KBookmarkGroup &bm);

    /**
     * Called when a bookmark should be opened in a new tab.
     * The default implementation calls openBookmark.
     * @since 5.0
     */
    virtual void openInNewTab(const KBookmark &bm);

    /**
     * Called when a bookmark should be opened in a new window.
     * The default implementation calls openBookmark.
     * @since 5.0
     */
    virtual void openInNewWindow(const KBookmark &bm);

private:
    class KBookmarkOwnerPrivate;
    KBookmarkOwnerPrivate *d;
};

#endif

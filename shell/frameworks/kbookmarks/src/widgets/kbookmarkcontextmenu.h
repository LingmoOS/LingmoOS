/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBOOKMARKCONTEXTMENU_H
#define KBOOKMARKCONTEXTMENU_H

#include <QMenu>

#include "kbookmark.h"
#include <kbookmarkswidgets_export.h>

class KBookmarkManager;
class KBookmarkOwner;

/**
 * @class KBookmarkContextMenu kbookmarkcontextmenu.h KBookmarkContextMenu
 *
 * A context menu for a bookmark.
 */
class KBOOKMARKSWIDGETS_EXPORT KBookmarkContextMenu : public QMenu
{
    Q_OBJECT

public:
    KBookmarkContextMenu(const KBookmark &bm, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent = nullptr);
    ~KBookmarkContextMenu() override;
    virtual void addActions();

    /**
     * Set this to true to make any "Edit Bookmarks" dialog
     * show UI elements that are specific to browsers.
     *
     * @since 6.0
     */
    void setBrowserMode(bool browserMode);

    /**
     * Whether any "Edit Bookmarks" dialog shows UI elements
     * that are specific to browsers.
     *
     * @since 6.0
     */
    bool browserMode() const;

public Q_SLOTS:
    void slotEditAt();
    void slotProperties();
    void slotInsert();
    void slotRemove();
    void slotCopyLocation();
    void slotOpenFolderInTabs();

protected:
    void addBookmark();
    void addFolderActions();
    void addProperties();
    void addBookmarkActions();
    void addOpenFolderInTabs();

    KBookmarkManager *manager() const;
    KBookmarkOwner *owner() const;
    KBookmark bookmark() const;

private Q_SLOTS:
    KBOOKMARKS_NO_EXPORT void slotAboutToShow();

private:
    // TODO KF6 dptr it
    const KBookmark bm;
    KBookmarkManager *const m_pManager;
    KBookmarkOwner *const m_pOwner;
    bool m_browserMode = false;
};

#endif

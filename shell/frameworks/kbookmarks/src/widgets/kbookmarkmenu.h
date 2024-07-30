/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __kbookmarkmenu_h__
#define __kbookmarkmenu_h__

#include <kbookmarkswidgets_export.h>

#include <QObject>
#include <memory>

class QAction;
class QMenu;
class KBookmark;
class KBookmarkManager;
class KBookmarkOwner;
class KBookmarkMenu;

class KBookmarkMenuPrivate;

/**
 * @class KBookmarkMenu kbookmarkmenu.h KBookmarkMenu
 *
 * This class provides a bookmark menu.  It is typically used in
 * cooperation with KActionMenu but doesn't have to be.
 *
 * If you use this class by itself, then it will use KDE defaults for
 * everything -- the bookmark path, bookmark editor, bookmark launcher..
 * everything.  These defaults reside in the classes
 * KBookmarkOwner (editing bookmarks) and KBookmarkManager
 * (almost everything else).  If you wish to change the defaults in
 * any way, you must reimplement either this class or KBookmarkOwner.
 *
 * Using this class is very simple:
 *
 * 1) Create a popup menu (either KActionMenu or QMenu will do)
 * 2) Instantiate a new KBookmarkMenu object using the above popup
 *    menu as a parameter
 * 3) Insert your (now full) popup menu wherever you wish
 *
 * The functionality of this class can be disabled with the "action/bookmarks"
 * Kiosk action (see the KAuthorized namespace).
 */
class KBOOKMARKSWIDGETS_EXPORT KBookmarkMenu : public QObject
{
    Q_OBJECT
public:
    /**
     * Fills a bookmark menu
     * (one instance of KBookmarkMenu is created for the toplevel menu,
     *  but also one per submenu).
     *
     * @param manager the bookmark manager to use (i.e. for reading and writing)
     * @param owner implementation of the KBookmarkOwner callback interface.
     * @note If you pass a null KBookmarkOwner to the constructor, the
     * openBookmark signal is not emitted, instead QDesktopServices::openUrl is used to open the bookmark.
     * @param parentMenu menu to be filled
     * @since 5.69
     */
    KBookmarkMenu(KBookmarkManager *manager, KBookmarkOwner *owner, QMenu *parentMenu);

    /**
     * Creates a bookmark submenu
     *
     * @todo KF6: give ownership of the bookmarkmenu to another qobject, e.g. parentMenu.
     * Currently this is a QObject without a parent, use setParent to benefit from automatic deletion.
     */
    KBookmarkMenu(KBookmarkManager *mgr, KBookmarkOwner *owner, QMenu *parentMenu, const QString &parentAddress);

    ~KBookmarkMenu() override;

    /**
     * Call ensureUpToDate() if you need KBookmarkMenu to adjust to its
     * final size before it is executed.
     **/
    void ensureUpToDate();

    /**
     * Returns the action for adding a bookmark. If you are using KXmlGui, you can add it to your
     * action collection.
     * @code
     * KBookmarkMenu *menu = new KBookmarkMenu(manager, owner, parentMenu);
     * QAction *addAction = menu->addBookmarkAction();
     * actionCollection()->addAction(addAction->objectName(), addAction);
     * @endcode
     * @return the action for adding a bookmark.
     * @since 5.69
     */
    QAction *addBookmarkAction() const;

    /**
     * Returns the action for adding all current tabs as bookmarks. If you are using KXmlGui, you can
     * add it to your action collection.
     * @code
     * KBookmarkMenu *menu = new KBookmarkMenu(manager, owner, parentMenu);
     * QAction *bookmarkTabsAction = menu->bookmarkTabsAsFolderAction();
     * actionCollection()->addAction(bookmarkTabsAction->objectName(), bookmarkTabsAction);
     * @endcode
     * @return the action for adding all current tabs as bookmarks.
     * @since 5.69
     */
    QAction *bookmarkTabsAsFolderAction() const;

    /**
     * Returns the action for adding a new bookmarks folder. If you are using KXmlGui, you can add it
     * to your action collection.
     * @code
     * KBookmarkMenu *menu = new KBookmarkMenu(manager, owner, parentMenu);
     * QAction *newBookmarkFolderAction = menu->bookmarkTabsAsFolderAction();
     * actionCollection()->addAction(newBookmarkFolderAction->objectName(), newBookmarkFolderAction);
     * @endcode
     * @return the action for adding a new bookmarks folder
     * @since 5.70
     */
    QAction *newBookmarkFolderAction() const;

    /**
     * Returns the action for editing bookmarks.  If you are using KXmlGui, you can add it to your
     * action collection.
     * @code
     * KBookmarkMenu *menu = new KBookmarkMenu(manager, owner, parentMenu);
     * QAction *editAction = menu->editBookmarksAction();
     * actionCollection()->addAction(editAction->objectName(), editAction);
     * @endcode
     * @return the action for editing bookmarks.
     * @since 5.69
     */
    QAction *editBookmarksAction() const;

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
    // public for KonqBookmarkBar
    void slotBookmarksChanged(const QString &);

protected Q_SLOTS:
    void slotAboutToShow();
    void slotAddBookmarksList();
    void slotAddBookmark();
    void slotNewFolder();
    void slotOpenFolderInTabs();

protected:
    virtual void clear();
    virtual void refill();
    virtual QAction *actionForBookmark(const KBookmark &bm);
    virtual QMenu *contextMenu(QAction *action);

    void addActions();
    void fillBookmarks();
    void addAddBookmark();
    void addAddBookmarksList();
    void addEditBookmarks();
    void addNewFolder();
    void addOpenInTabs();

    bool isRoot() const;
    bool isDirty() const;

    /**
     * Parent bookmark for this menu.
     */
    QString parentAddress() const;

    KBookmarkManager *manager() const;
    KBookmarkOwner *owner() const;
    /**
     * The menu in which we insert our actions
     * Supplied in the constructor.
     */
    QMenu *parentMenu() const;

    /**
     * List of our sub menus
     */
    QList<KBookmarkMenu *> m_lstSubMenus;

    /**
     * List of our actions.
     */
    QList<QAction *> m_actions;

private Q_SLOTS:
    KBOOKMARKSWIDGETS_NO_EXPORT void slotCustomContextMenu(const QPoint &);

private:
    void slotEditBookmarks();
    KBOOKMARKSWIDGETS_NO_EXPORT void init();

private:
    std::unique_ptr<KBookmarkMenuPrivate> const d;
};

#endif

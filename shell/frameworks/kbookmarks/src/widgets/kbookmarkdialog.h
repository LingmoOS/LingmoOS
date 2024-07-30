/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef __kbookmarkdialog_h
#define __kbookmarkdialog_h

#include "kbookmark.h"
#include "kbookmarkowner.h"
#include <kbookmarkswidgets_export.h>

#include <QDialog>
#include <memory>

class KBookmarkManager;
class KBookmarkDialogPrivate;

/**
 * @class KBookmarkDialog kbookmarkdialog.h KBookmarkDialog
 *
 * This class provides a Dialog for editing properties, adding Bookmarks and creating new folders.
 * It can be used to show dialogs for common tasks with bookmarks.
 *
 * It is used by KBookmarkMenu to show a dialog for "Properties", "Add Bookmark" and "Create New Folder".
 * If you want to customize those dialogs, derive from KBookmarkOwner and reimplement bookmarkDialog(),
 * return a KBookmarkDialog subclass and reimplement initLayout(), aboutToShow() and save().
 */
class KBOOKMARKSWIDGETS_EXPORT KBookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Creates a KBookmarkDialog instance
     */
    KBookmarkDialog(KBookmarkManager *manager, QWidget *parent = nullptr);
    /**
     * Shows a properties dialog
     * Note: this updates the bookmark and calls KBookmarkManager::emitChanged
     */
    KBookmark editBookmark(const KBookmark &bm);
    /**
     * Shows a "Add Bookmark" dialog
     * Note: this updates the bookmark and calls KBookmarkManager::emitChanged
     */
    KBookmark addBookmark(const QString &title, const QUrl &url, const QString &icon, KBookmark parent = KBookmark());
    /**
     * Creates a folder from a list of bookmarks
     * Note: this updates the bookmark and calls KBookmarkManager::emitChanged
     */
    KBookmarkGroup addBookmarks(const QList<KBookmarkOwner::FutureBookmark> &list, const QString &name = QString(), KBookmarkGroup parent = KBookmarkGroup());
    /**
     * Shows a dialog to create a new folder.
     */
    KBookmarkGroup createNewFolder(const QString &name, KBookmark parent = KBookmark());
    /**
     * Shows a dialog to select a folder.
     */
    KBookmarkGroup selectFolder(KBookmark start = KBookmark());

    ~KBookmarkDialog() override;

protected:
    void accept() override;

protected Q_SLOTS:
    void newFolderButton();

private:
    std::unique_ptr<KBookmarkDialogPrivate> const d;
    friend class KBookmarkDialogPrivate;
};

#endif

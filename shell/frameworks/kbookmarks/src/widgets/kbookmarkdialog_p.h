//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Jignesh Kakadiya <jigneshhk1992@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef __kbookmarkdialog_p_h__
#define __kbookmarkdialog_p_h__

#include "kbookmark.h"
#include <QDialog>

class KBookmarkDialog;
class KBookmarkManager;
class QDialogButtonBox;
class QLabel;
class QTreeWidget;
class QLineEdit;
class QTreeWidgetItem;

class KBookmarkDialogPrivate
{
public:
    explicit KBookmarkDialogPrivate(KBookmarkDialog *qq);
    ~KBookmarkDialogPrivate();

    typedef enum { NewFolder, NewBookmark, EditBookmark, NewMultipleBookmarks, SelectFolder } BookmarkDialogMode;

    void initLayout();
    void initLayoutPrivate();
    // selects the specified bookmark in the folder tree
    void setParentBookmark(const KBookmark &bm);
    KBookmarkGroup parentBookmark();
    void fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group, const KBookmarkGroup &selectGroup = KBookmarkGroup());

    KBookmarkDialog *const q;
    BookmarkDialogMode mode;
    QDialogButtonBox *buttonBox = nullptr;
    QLineEdit *url = nullptr;
    QLineEdit *title = nullptr;
    QLineEdit *comment = nullptr;
    QLabel *titleLabel = nullptr;
    QLabel *urlLabel = nullptr;
    QLabel *commentLabel;
    QString icon;
    QTreeWidget *folderTree = nullptr;
    KBookmarkManager *mgr = nullptr;
    KBookmark bm;
    QList<KBookmarkOwner::FutureBookmark> list;
    bool layout;
};

#endif

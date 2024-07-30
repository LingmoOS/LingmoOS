//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __kbookmarkmenu_p_h__
#define __kbookmarkmenu_p_h__

#include <QTreeWidget>

#include "kbookmark.h"

class QString;
class KBookmark;
class KBookmarkGroup;

class KBookmarkTreeItem : public QTreeWidgetItem
{
public:
    explicit KBookmarkTreeItem(QTreeWidget *tree);
    KBookmarkTreeItem(QTreeWidgetItem *parent, QTreeWidget *tree, const KBookmarkGroup &bk);
    ~KBookmarkTreeItem() override;
    QString address();

private:
    QString m_address;
};

#endif

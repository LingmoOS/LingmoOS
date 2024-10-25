/*
    SPDX-FileCopyrightText: 2005 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QTreeWidgetItem>

#include "katesession.h"

class KateSessionChooserItem : public QTreeWidgetItem
{
public:
    KateSessionChooserItem(QTreeWidget *tw, KateSession::Ptr s)
        : QTreeWidgetItem(tw, QStringList(s->name()))
        , session(s)
    {
        QString docs;
        docs.setNum(s->documents());
        setText(1, docs);
        setText(2, s->timestamp().toString(QString::fromStdString("yyyy-MM-dd  hh:mm:ss")));
    }

    KateSession::Ptr session;
};

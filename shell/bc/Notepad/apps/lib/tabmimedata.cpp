/*
 *  SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "tabmimedata.h"

#include <QDataStream>

TabMimeData::TabMimeData(KateViewSpace *vs, DocOrWidget d)
    : QMimeData()
    , sourceVS(vs)
    , doc(d)
{
    if (doc.doc() && doc.doc()->url().isValid()) {
        setUrls({doc.doc()->url()});
    }
}

bool TabMimeData::hasValidData(const QMimeData *md)
{
    bool valid = md && md->hasFormat(QStringLiteral("application/kate.tab.mimedata"));
    if (valid) {
        QByteArray data = md->data(QStringLiteral("application/kate.tab.mimedata"));
        QDataStream ds(data);
        int line = 0;
        int column = 0;
        QUrl url;
        ds >> line;
        ds >> column;
        ds >> url;
        return url.isValid();
    }
    return false;
}

std::optional<TabMimeData::DroppedData> TabMimeData::data(const QMimeData *md)
{
    if (!md || !md->hasFormat(QStringLiteral("application/kate.tab.mimedata"))) {
        return {};
    }

    QByteArray data = md->data(QStringLiteral("application/kate.tab.mimedata"));
    QDataStream ds(data);
    TabMimeData::DroppedData d;
    ds >> d.line;
    ds >> d.col;
    ds >> d.url;
    if (!d.url.isValid()) {
        return {};
    }
    return d;
}

/*
    SPDX-FileCopyrightText: 2013 Albert Vaca <albertvaka@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "klineediturldropeventfilter.h"

#include <QDropEvent>
#include <QLineEdit>
#include <QMimeData>

static const char s_kdeUriListMime[] = "application/x-kde4-urilist"; // keep this name "kde4" for compat.

KLineEditUrlDropEventFilter::KLineEditUrlDropEventFilter(QObject *parent)
    : QObject(parent)
{
}

KLineEditUrlDropEventFilter::~KLineEditUrlDropEventFilter() = default;

bool KLineEditUrlDropEventFilter::eventFilter(QObject *object, QEvent *event)
{
    // Handle only drop events
    if (event->type() != QEvent::Drop) {
        return false;
    }
    auto *dropEvent = static_cast<QDropEvent *>(event);

    // Handle only url drops, we check the MIME type for the standard or kde's urllist
    // It would be interesting to handle urls that don't have any MIME type set (like a drag and drop from kate)
    const QMimeData *data = dropEvent->mimeData();
    if (!data->hasUrls() && !data->hasFormat(QLatin1String(s_kdeUriListMime))) {
        return false;
    }

    // Our object should be a QLineEdit
    auto *line = qobject_cast<QLineEdit *>(object);
    if (!line) {
        return false;
    }

    QString content = data->text();
    line->setText(content);
    line->setCursorPosition(content.length());

    event->accept();
    return true;
}

#include "moc_klineediturldropeventfilter.cpp"

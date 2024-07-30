/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "qtclipboard_p.h"

#include <QClipboard>
#include <QGuiApplication>

QtClipboard::QtClipboard(QObject *parent)
    : KSystemClipboard(parent)
{
    connect(qGuiApp->clipboard(), &QClipboard::changed, this, &QtClipboard::changed);
}

void QtClipboard::setMimeData(QMimeData *mime, QClipboard::Mode mode)
{
    qGuiApp->clipboard()->setMimeData(mime, mode);
}

void QtClipboard::clear(QClipboard::Mode mode)
{
    qGuiApp->clipboard()->clear(mode);
}

const QMimeData *QtClipboard::mimeData(QClipboard::Mode mode) const
{
    return qGuiApp->clipboard()->mimeData(mode);
}

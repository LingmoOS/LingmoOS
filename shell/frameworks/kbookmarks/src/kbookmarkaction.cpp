/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbookmarkaction.h"
#include "kbookmarkowner.h"

#include <QDesktopServices>
#include <QGuiApplication>

KBookmarkAction::KBookmarkAction(const KBookmark &bk, KBookmarkOwner *owner, QObject *parent)
    : QAction(bk.text().replace(QLatin1Char('&'), QLatin1String("&&")), parent)
    , KBookmarkActionInterface(bk)
    , m_pOwner(owner)
{
    setIcon(QIcon::fromTheme(bookmark().icon()));
    setIconText(text());
    setToolTip(bookmark().url().toDisplayString(QUrl::PreferLocalFile));
    setStatusTip(toolTip());
    setWhatsThis(toolTip());
    const QString description = bk.description();
    if (!description.isEmpty()) {
        setToolTip(description);
    }
    connect(this, &QAction::triggered, this, &KBookmarkAction::slotTriggered);
}

KBookmarkAction::~KBookmarkAction()
{
}

void KBookmarkAction::slotTriggered()
{
    slotSelected(QGuiApplication::mouseButtons(), QGuiApplication::keyboardModifiers());
}

void KBookmarkAction::slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
    if (!m_pOwner) {
        QDesktopServices::openUrl(bookmark().url());
    } else {
        m_pOwner->openBookmark(bookmark(), mb, km);
    }
}

#include "moc_kbookmarkaction.cpp"

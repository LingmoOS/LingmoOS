/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbookmarkcontextmenu.h"
#include "kbookmarkdialog.h"
#include "kbookmarkmanager.h"
#include "kbookmarkowner.h"
#include "keditbookmarks_p.h"

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QMimeData>

KBookmarkContextMenu::KBookmarkContextMenu(const KBookmark &bk, KBookmarkManager *manager, KBookmarkOwner *owner, QWidget *parent)
    : QMenu(parent)
    , bm(bk)
    , m_pManager(manager)
    , m_pOwner(owner)
{
    connect(this, &QMenu::aboutToShow, this, &KBookmarkContextMenu::slotAboutToShow);
}

void KBookmarkContextMenu::slotAboutToShow()
{
    addActions();
}

void KBookmarkContextMenu::addActions()
{
    if (bm.isGroup()) {
        addOpenFolderInTabs();
        addBookmark();
        addFolderActions();
    } else {
        addBookmark();
        addBookmarkActions();
    }
}

KBookmarkContextMenu::~KBookmarkContextMenu()
{
}

void KBookmarkContextMenu::addBookmark()
{
    if (m_pOwner && m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark)) {
        addAction(QIcon::fromTheme(QStringLiteral("bookmark-new")), tr("Add Bookmark Here", "@action:inmenu"), this, &KBookmarkContextMenu::slotInsert);
    }
}

void KBookmarkContextMenu::addFolderActions()
{
    addAction(tr("Open Folder in Bookmark Editor", "@action:inmenu"), this, &KBookmarkContextMenu::slotEditAt);
    addProperties();
    addSeparator();
    addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete Folder", "@action:inmenu"), this, &KBookmarkContextMenu::slotRemove);
}

void KBookmarkContextMenu::addProperties()
{
    addAction(tr("Properties", "@action:inmenu"), this, &KBookmarkContextMenu::slotProperties);
}

void KBookmarkContextMenu::addBookmarkActions()
{
    addAction(tr("Copy Link Address", "@action:inmenu"), this, &KBookmarkContextMenu::slotCopyLocation);
    addProperties();
    addSeparator();
    addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete Bookmark", "@action:inmenu"), this, &KBookmarkContextMenu::slotRemove);
}

void KBookmarkContextMenu::addOpenFolderInTabs()
{
    if (m_pOwner->supportsTabs()) {
        addAction(QIcon::fromTheme(QStringLiteral("tab-new")), tr("Open Folder in Tabs", "@action:inmenu"), this, &KBookmarkContextMenu::slotOpenFolderInTabs);
    }
}

void KBookmarkContextMenu::slotEditAt()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotEditAt" << m_highlightedAddress;
    KEditBookmarks editBookmarks;
    editBookmarks.setBrowserMode(m_browserMode);
    auto result = editBookmarks.openForFileAtAddress(m_pManager->path(), bm.address());

    if (!result.sucess()) {
        QMessageBox::critical(QApplication::activeWindow(), QApplication::applicationDisplayName(), result.errorMessage());
    }
}

void KBookmarkContextMenu::slotProperties()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotProperties" << m_highlightedAddress;

    KBookmarkDialog *dlg = new KBookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->editBookmark(bm);
    delete dlg;
}

void KBookmarkContextMenu::slotInsert()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotInsert" << m_highlightedAddress;

    QUrl url = m_pOwner->currentUrl();
    if (url.isEmpty()) {
        QMessageBox::critical(QApplication::activeWindow(), QApplication::applicationName(), tr("Cannot add bookmark with empty URL.", "@info"));
        return;
    }
    QString title = m_pOwner->currentTitle();
    if (title.isEmpty()) {
        title = url.toDisplayString();
    }

    if (bm.isGroup()) {
        KBookmarkGroup parentBookmark = bm.toGroup();
        Q_ASSERT(!parentBookmark.isNull());
        parentBookmark.addBookmark(title, url, m_pOwner->currentIcon());
        m_pManager->emitChanged(parentBookmark);
    } else {
        KBookmarkGroup parentBookmark = bm.parentGroup();
        Q_ASSERT(!parentBookmark.isNull());
        KBookmark newBookmark = parentBookmark.addBookmark(title, m_pOwner->currentUrl(), m_pOwner->currentIcon());
        parentBookmark.moveBookmark(newBookmark, parentBookmark.previous(bm));
        m_pManager->emitChanged(parentBookmark);
    }
}

void KBookmarkContextMenu::slotRemove()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotRemove" << m_highlightedAddress;

    bool folder = bm.isGroup();

    if (QMessageBox::warning(QApplication::activeWindow(),
                             folder ? tr("Bookmark Folder Deletion", "@title:window") : tr("Bookmark Deletion", "@title:window"),
                             folder ? tr("Are you sure you wish to remove the bookmark folder\n\"%1\"?").arg(bm.text())
                                    : tr("Are you sure you wish to remove the bookmark\n\"%1\"?").arg(bm.text()),
                             QMessageBox::Yes | QMessageBox::Cancel)
        != QMessageBox::Yes) {
        return;
    }

    KBookmarkGroup parentBookmark = bm.parentGroup();
    parentBookmark.deleteBookmark(bm);
    m_pManager->emitChanged(parentBookmark);
}

void KBookmarkContextMenu::slotCopyLocation()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotCopyLocation" << m_highlightedAddress;

    if (!bm.isGroup()) {
        QMimeData *mimeData = new QMimeData;
        bm.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Selection);
        mimeData = new QMimeData;
        bm.populateMimeData(mimeData);
        QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
    }
}

void KBookmarkContextMenu::slotOpenFolderInTabs()
{
    owner()->openFolderinTabs(bookmark().toGroup());
}

KBookmarkManager *KBookmarkContextMenu::manager() const
{
    return m_pManager;
}

KBookmarkOwner *KBookmarkContextMenu::owner() const
{
    return m_pOwner;
}

KBookmark KBookmarkContextMenu::bookmark() const
{
    return bm;
}

void KBookmarkContextMenu::setBrowserMode(bool browserMode)
{
    m_browserMode = browserMode;
}

bool KBookmarkContextMenu::browserMode() const
{
    return m_browserMode;
}

#include "moc_kbookmarkcontextmenu.cpp"

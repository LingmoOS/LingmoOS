/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"

#include "../kbookmarksettings_p.h"
#include "kbookmarkaction.h"
#include "kbookmarkactionmenu.h"
#include "kbookmarkcontextmenu.h"
#include "kbookmarkdialog.h"
#include "kbookmarkmanager.h"
#include "kbookmarkowner.h"
#include "kbookmarkswidgets_debug.h"
#include "keditbookmarks_p.h"

#include <KAuthorized>
#include <KStandardAction>

#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>

/********************************************************************/
/********************************************************************/
/********************************************************************/
class KBookmarkMenuPrivate
{
public:
    QAction *newBookmarkFolderAction = nullptr;
    QAction *addBookmarkAction = nullptr;
    QAction *bookmarksToFolderAction = nullptr;
    QAction *editBookmarksAction = nullptr;
    bool browserMode = false;
    bool isRoot;
    bool dirty;
    KBookmarkManager *manager;
    KBookmarkOwner *owner;
    QMenu *parentMenu;
    QString parentAddress;
};

KBookmarkMenu::KBookmarkMenu(KBookmarkManager *manager, KBookmarkOwner *_owner, QMenu *_parentMenu)
    : QObject()
    , d(new KBookmarkMenuPrivate())
{
    d->isRoot = true;
    d->manager = manager;
    d->owner = _owner;
    d->parentMenu = _parentMenu;
    d->parentAddress = QString(); // TODO KBookmarkAdress::root
    // TODO KDE5 find a QMenu equvalnet for this one
    // m_parentMenu->setKeyboardShortcutsEnabled( true );

    init();
}

void KBookmarkMenu::init()
{
    connect(d->parentMenu, &QMenu::aboutToShow, this, &KBookmarkMenu::slotAboutToShow);

    if (KBookmarkSettings::self()->m_contextmenu) {
        d->parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(d->parentMenu, &QWidget::customContextMenuRequested, this, &KBookmarkMenu::slotCustomContextMenu);
    }

    connect(d->manager, &KBookmarkManager::changed, this, &KBookmarkMenu::slotBookmarksChanged);

    d->dirty = true;
    addActions();
}

void KBookmarkMenu::addActions()
{
    if (d->isRoot) {
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
        addEditBookmarks();
    } else {
        if (!d->parentMenu->actions().isEmpty()) {
            d->parentMenu->addSeparator();
        }

        addOpenInTabs();
        addAddBookmark();
        addAddBookmarksList();
        addNewFolder();
    }
}

KBookmarkMenu::KBookmarkMenu(KBookmarkManager *mgr, KBookmarkOwner *_owner, QMenu *_parentMenu, const QString &parentAddress)
    : QObject()
    , d(new KBookmarkMenuPrivate())
{
    d->isRoot = false;
    d->manager = mgr;
    d->owner = _owner;
    d->parentMenu = _parentMenu;
    d->parentAddress = parentAddress;

    connect(_parentMenu, &QMenu::aboutToShow, this, &KBookmarkMenu::slotAboutToShow);
    if (KBookmarkSettings::self()->m_contextmenu) {
        d->parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(d->parentMenu, &QWidget::customContextMenuRequested, this, &KBookmarkMenu::slotCustomContextMenu);
    }
    d->dirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
    qDeleteAll(m_lstSubMenus);
    qDeleteAll(m_actions);
}

void KBookmarkMenu::ensureUpToDate()
{
    slotAboutToShow();
}

void KBookmarkMenu::slotAboutToShow()
{
    // Did the bookmarks change since the last time we showed them ?
    if (d->dirty) {
        d->dirty = false;
        clear();
        refill();
        d->parentMenu->adjustSize();
    }
}

void KBookmarkMenu::slotCustomContextMenu(const QPoint &pos)
{
    QAction *action = d->parentMenu->actionAt(pos);
    QMenu *menu = contextMenu(action);
    if (!menu) {
        return;
    }
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(d->parentMenu->mapToGlobal(pos));
}

QMenu *KBookmarkMenu::contextMenu(QAction *action)
{
    KBookmarkActionInterface *act = dynamic_cast<KBookmarkActionInterface *>(action);
    if (!act) {
        return nullptr;
    }
    return new KBookmarkContextMenu(act->bookmark(), d->manager, d->owner);
}

bool KBookmarkMenu::isRoot() const
{
    return d->isRoot;
}

bool KBookmarkMenu::isDirty() const
{
    return d->dirty;
}

QString KBookmarkMenu::parentAddress() const
{
    return d->parentAddress;
}

KBookmarkManager *KBookmarkMenu::manager() const
{
    return d->manager;
}

KBookmarkOwner *KBookmarkMenu::owner() const
{
    return d->owner;
}

QMenu *KBookmarkMenu::parentMenu() const
{
    return d->parentMenu;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::slotBookmarksChanged(const QString &groupAddress)
{
    qCDebug(KBOOKMARKSWIDGETS_LOG) << "KBookmarkMenu::slotBookmarksChanged groupAddress: " << groupAddress;
    if (groupAddress == d->parentAddress) {
        // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress;
        d->dirty = true;
    } else {
        // Iterate recursively into child menus
        for (QList<KBookmarkMenu *>::iterator it = m_lstSubMenus.begin(), end = m_lstSubMenus.end(); it != end; ++it) {
            (*it)->slotBookmarksChanged(groupAddress);
        }
    }
}

void KBookmarkMenu::clear()
{
    qDeleteAll(m_lstSubMenus);
    m_lstSubMenus.clear();

    for (QList<QAction *>::iterator it = m_actions.begin(), end = m_actions.end(); it != end; ++it) {
        d->parentMenu->removeAction(*it);
        delete *it;
    }

    d->parentMenu->clear();
    m_actions.clear();
}

void KBookmarkMenu::refill()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkMenu::refill()";
    if (d->isRoot) {
        addActions();
    }
    fillBookmarks();
    if (!d->isRoot) {
        addActions();
    }
}

void KBookmarkMenu::addOpenInTabs()
{
    if (!d->owner || !d->owner->supportsTabs() || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    const QString title = tr("Open Folder in Tabs", "@action:inmenu");

    QAction *paOpenFolderInTabs = new QAction(title, this);
    paOpenFolderInTabs->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    paOpenFolderInTabs->setToolTip(tr("Open all bookmarks in this folder as a new tab", "@info:tooltip"));
    paOpenFolderInTabs->setStatusTip(paOpenFolderInTabs->toolTip());
    connect(paOpenFolderInTabs, &QAction::triggered, this, &KBookmarkMenu::slotOpenFolderInTabs);

    d->parentMenu->addAction(paOpenFolderInTabs);
    m_actions.append(paOpenFolderInTabs);
}

void KBookmarkMenu::addAddBookmarksList()
{
    if (!d->owner || !d->owner->enableOption(KBookmarkOwner::ShowAddBookmark) || !d->owner->supportsTabs()
        || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    if (!d->bookmarksToFolderAction) {
        const QString title = tr("Bookmark Tabs as Folder…", "@action:inmenu");
        d->bookmarksToFolderAction = new QAction(title, this);

        if (d->isRoot) {
            d->bookmarksToFolderAction->setObjectName(QStringLiteral("add_bookmarks_list"));
        }

        d->bookmarksToFolderAction->setIcon(QIcon::fromTheme(QStringLiteral("bookmark-new-list")));
        d->bookmarksToFolderAction->setToolTip(tr("Add a folder of bookmarks for all open tabs", "@info:tooltip"));
        d->bookmarksToFolderAction->setStatusTip(d->bookmarksToFolderAction->toolTip());
        connect(d->bookmarksToFolderAction, &QAction::triggered, this, &KBookmarkMenu::slotAddBookmarksList);
    }

    d->parentMenu->addAction(d->bookmarksToFolderAction);
}

void KBookmarkMenu::addAddBookmark()
{
    if (!d->owner || !d->owner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    if (!d->addBookmarkAction) {
        d->addBookmarkAction = KStandardAction::addBookmark(this, &KBookmarkMenu::slotAddBookmark, this);
        if (d->isRoot) {
            d->addBookmarkAction->setObjectName(QStringLiteral("add_bookmark"));
        }

        if (!d->isRoot) {
            d->addBookmarkAction->setShortcut(QKeySequence());
        }
    }

    d->parentMenu->addAction(d->addBookmarkAction);
}

void KBookmarkMenu::addEditBookmarks()
{
    if ((d->owner && !d->owner->enableOption(KBookmarkOwner::ShowEditBookmark))
        || QStandardPaths::findExecutable(QStringLiteral(KEDITBOOKMARKS_BINARY)).isEmpty() || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    d->editBookmarksAction = KStandardAction::editBookmarks(this, &KBookmarkMenu::slotEditBookmarks, this);
    d->editBookmarksAction->setObjectName(QStringLiteral("edit_bookmarks"));

    d->parentMenu->addAction(d->editBookmarksAction);
    d->editBookmarksAction->setToolTip(tr("Edit your bookmark collection in a separate window", "@info:tooltip"));
    d->editBookmarksAction->setStatusTip(d->editBookmarksAction->toolTip());
}

void KBookmarkMenu::slotEditBookmarks()
{
    KEditBookmarks editBookmarks;
    editBookmarks.setBrowserMode(d->browserMode);
    auto result = editBookmarks.openForFile(d->manager->path());

    if (!result.sucess()) {
        QMessageBox::critical(QApplication::activeWindow(), QApplication::applicationDisplayName(), result.errorMessage());
    }
}

void KBookmarkMenu::addNewFolder()
{
    if (!d->owner || !d->owner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeAction(QStringLiteral("bookmarks"))) {
        return;
    }

    if (!d->newBookmarkFolderAction) {
        d->newBookmarkFolderAction = new QAction(tr("New Bookmark Folder…", "@action:inmenu"), this);
        d->newBookmarkFolderAction->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));
        d->newBookmarkFolderAction->setToolTip(tr("Create a new bookmark folder in this menu", "@info:tooltip"));
        d->newBookmarkFolderAction->setStatusTip(d->newBookmarkFolderAction->toolTip());

        if (d->isRoot) {
            d->newBookmarkFolderAction->setObjectName(QStringLiteral("new_bookmark_folder"));
        }

        connect(d->newBookmarkFolderAction, &QAction::triggered, this, &KBookmarkMenu::slotNewFolder);
    }

    d->parentMenu->addAction(d->newBookmarkFolderAction);
}

void KBookmarkMenu::fillBookmarks()
{
    KBookmarkGroup parentBookmark = d->manager->findByAddress(d->parentAddress).toGroup();
    Q_ASSERT(!parentBookmark.isNull());

    if (d->isRoot && !parentBookmark.first().isNull()) { // at least one bookmark
        d->parentMenu->addSeparator();
    }

    for (KBookmark bm = parentBookmark.first(); !bm.isNull(); bm = parentBookmark.next(bm)) {
        d->parentMenu->addAction(actionForBookmark(bm));
    }
}

QAction *KBookmarkMenu::actionForBookmark(const KBookmark &bm)
{
    if (bm.isGroup()) {
        // qCDebug(KBOOKMARKS_LOG) << "Creating bookmark submenu named " << bm.text();
        KActionMenu *actionMenu = new KBookmarkActionMenu(bm, this);
        m_actions.append(actionMenu);
        KBookmarkMenu *subMenu = new KBookmarkMenu(d->manager, d->owner, actionMenu->menu(), bm.address());
        m_lstSubMenus.append(subMenu);
        return actionMenu;
    } else if (bm.isSeparator()) {
        QAction *sa = new QAction(this);
        sa->setSeparator(true);
        m_actions.append(sa);
        return sa;
    } else {
        // qCDebug(KBOOKMARKS_LOG) << "Creating bookmark menu item for " << bm.text();
        QAction *action = new KBookmarkAction(bm, d->owner, this);
        m_actions.append(action);
        return action;
    }
}

void KBookmarkMenu::slotAddBookmarksList()
{
    if (!d->owner || !d->owner->supportsTabs()) {
        return;
    }

    KBookmarkGroup parentBookmark = d->manager->findByAddress(d->parentAddress).toGroup();

    KBookmarkDialog *dlg = new KBookmarkDialog(d->manager, QApplication::activeWindow());
    dlg->addBookmarks(d->owner->currentBookmarkList(), QLatin1String(""), parentBookmark);
    delete dlg;
}

void KBookmarkMenu::slotAddBookmark()
{
    if (!d->owner) {
        return;
    }
    if (d->owner->currentTitle().isEmpty() && d->owner->currentUrl().isEmpty()) {
        return;
    }
    KBookmarkGroup parentBookmark = d->manager->findByAddress(d->parentAddress).toGroup();

    if (KBookmarkSettings::self()->m_advancedaddbookmark) {
        KBookmarkDialog *dlg = new KBookmarkDialog(d->manager, QApplication::activeWindow());
        dlg->addBookmark(d->owner->currentTitle(), d->owner->currentUrl(), d->owner->currentIcon(), parentBookmark);
        delete dlg;
    } else {
        parentBookmark.addBookmark(d->owner->currentTitle(), d->owner->currentUrl(), d->owner->currentIcon());
        d->manager->emitChanged(parentBookmark);
    }
}

void KBookmarkMenu::slotOpenFolderInTabs()
{
    d->owner->openFolderinTabs(d->manager->findByAddress(d->parentAddress).toGroup());
}

void KBookmarkMenu::slotNewFolder()
{
    if (!d->owner) {
        return; // this view doesn't handle bookmarks...
    }
    KBookmarkGroup parentBookmark = d->manager->findByAddress(d->parentAddress).toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmarkDialog *dlg = new KBookmarkDialog(d->manager, QApplication::activeWindow());
    dlg->createNewFolder(QLatin1String(""), parentBookmark);
    delete dlg;
}

QAction *KBookmarkMenu::addBookmarkAction() const
{
    return d->addBookmarkAction;
}

QAction *KBookmarkMenu::bookmarkTabsAsFolderAction() const
{
    return d->bookmarksToFolderAction;
}

QAction *KBookmarkMenu::newBookmarkFolderAction() const
{
    return d->newBookmarkFolderAction;
}

QAction *KBookmarkMenu::editBookmarksAction() const
{
    return d->editBookmarksAction;
}

void KBookmarkMenu::setBrowserMode(bool browserMode)
{
    d->browserMode = browserMode;
}

bool KBookmarkMenu::browserMode() const
{
    return d->browserMode;
}

#include "moc_kbookmarkmenu.cpp"

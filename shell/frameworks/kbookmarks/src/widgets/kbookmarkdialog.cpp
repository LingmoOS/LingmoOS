//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Daniel Teske <teske@squorn.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkdialog.h"
#include "kbookmarkdialog_p.h"
#include "kbookmarkmanager.h"
#include "kbookmarkmenu_p.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>

#include <KGuiItem>

KBookmarkDialogPrivate::KBookmarkDialogPrivate(KBookmarkDialog *qq)
    : q(qq)
    , folderTree(nullptr)
    , layout(false)
{
}

KBookmarkDialogPrivate::~KBookmarkDialogPrivate()
{
}

void KBookmarkDialogPrivate::initLayout()
{
    QBoxLayout *vbox = new QVBoxLayout(q);

    QFormLayout *form = new QFormLayout();
    vbox->addLayout(form);

    form->addRow(titleLabel, title);
    form->addRow(urlLabel, url);
    form->addRow(commentLabel, comment);

    vbox->addWidget(folderTree);
    vbox->addWidget(buttonBox);
}

void KBookmarkDialogPrivate::initLayoutPrivate()
{
    title = new QLineEdit(q);
    title->setMinimumWidth(300);
    titleLabel = new QLabel(KBookmarkDialog::tr("Name:", "@label:textbox"), q);
    titleLabel->setBuddy(title);

    url = new QLineEdit(q);
    url->setMinimumWidth(300);
    urlLabel = new QLabel(KBookmarkDialog::tr("Location:", "@label:textbox"), q);
    urlLabel->setBuddy(url);

    comment = new QLineEdit(q);
    comment->setMinimumWidth(300);
    commentLabel = new QLabel(KBookmarkDialog::tr("Comment:", "@label:textbox"), q);
    commentLabel->setBuddy(comment);

    folderTree = new QTreeWidget(q);
    folderTree->setColumnCount(1);
    folderTree->header()->hide();
    folderTree->setSortingEnabled(false);
    folderTree->setSelectionMode(QTreeWidget::SingleSelection);
    folderTree->setSelectionBehavior(QTreeWidget::SelectRows);
    folderTree->setMinimumSize(60, 100);
    QTreeWidgetItem *root = new KBookmarkTreeItem(folderTree);
    fillGroup(root, mgr->root());

    buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    q->connect(buttonBox, &QDialogButtonBox::accepted, q, &KBookmarkDialog::accept);
    q->connect(buttonBox, &QDialogButtonBox::rejected, q, &QDialog::reject);

    initLayout();
    layout = true;
}

void KBookmarkDialogPrivate::fillGroup(QTreeWidgetItem *parentItem, const KBookmarkGroup &group, const KBookmarkGroup &selectGroup)
{
    for (KBookmark bk = group.first(); !bk.isNull(); bk = group.next(bk)) {
        if (bk.isGroup()) {
            const KBookmarkGroup bkGroup = bk.toGroup();
            QTreeWidgetItem *item = new KBookmarkTreeItem(parentItem, folderTree, bkGroup);
            if (selectGroup == bkGroup) {
                folderTree->setCurrentItem(item);
            }
            fillGroup(item, bkGroup, selectGroup);
        }
    }
}

void KBookmarkDialogPrivate::setParentBookmark(const KBookmark &bm)
{
    QString address = bm.address();
    KBookmarkTreeItem *item = static_cast<KBookmarkTreeItem *>(folderTree->topLevelItem(0));
    while (true) {
        if (item->address() == bm.address()) {
            folderTree->setCurrentItem(item);
            return;
        }
        for (int i = 0; i < item->childCount(); ++i) {
            KBookmarkTreeItem *child = static_cast<KBookmarkTreeItem *>(item->child(i));
            if (KBookmark::commonParent(child->address(), address) == child->address()) {
                item = child;
                break;
            }
        }
    }
}

KBookmarkGroup KBookmarkDialogPrivate::parentBookmark()
{
    KBookmarkTreeItem *item = dynamic_cast<KBookmarkTreeItem *>(folderTree->currentItem());
    if (!item) {
        return mgr->root();
    }
    const QString &address = item->address();
    return mgr->findByAddress(address).toGroup();
}

void KBookmarkDialog::accept()
{
    if (d->mode == KBookmarkDialogPrivate::NewFolder) {
        KBookmarkGroup parent = d->parentBookmark();
        if (d->title->text().isEmpty()) {
            d->title->setText(QStringLiteral("New Folder"));
        }
        d->bm = parent.createNewFolder(d->title->text());
        d->bm.setDescription(d->comment->text());
        d->mgr->emitChanged(parent);
    } else if (d->mode == KBookmarkDialogPrivate::NewBookmark) {
        KBookmarkGroup parent = d->parentBookmark();
        if (d->title->text().isEmpty()) {
            d->title->setText(QStringLiteral("New Bookmark"));
        }
        d->bm = parent.addBookmark(d->title->text(), QUrl(d->url->text()), d->icon);
        d->bm.setDescription(d->comment->text());
        d->mgr->emitChanged(parent);
    } else if (d->mode == KBookmarkDialogPrivate::NewMultipleBookmarks) {
        KBookmarkGroup parent = d->parentBookmark();
        if (d->title->text().isEmpty()) {
            d->title->setText(QStringLiteral("New Folder"));
        }
        d->bm = parent.createNewFolder(d->title->text());
        d->bm.setDescription(d->comment->text());
        for (const KBookmarkOwner::FutureBookmark &fb : std::as_const(d->list)) {
            d->bm.toGroup().addBookmark(fb.title(), fb.url(), fb.icon());
        }
        d->mgr->emitChanged(parent);
    } else if (d->mode == KBookmarkDialogPrivate::EditBookmark) {
        d->bm.setFullText(d->title->text());
        d->bm.setUrl(QUrl(d->url->text()));
        d->bm.setDescription(d->comment->text());
        d->mgr->emitChanged(d->bm.parentGroup());
    } else if (d->mode == d->SelectFolder) {
        d->bm = d->parentBookmark();
    }
    QDialog::accept();
}

KBookmark KBookmarkDialog::editBookmark(const KBookmark &bm)
{
    if (!d->layout) {
        d->initLayoutPrivate();
    }

    KGuiItem::assign(d->buttonBox->button(QDialogButtonBox::Ok), KGuiItem(tr("Update", "@action:button")));
    setWindowTitle(tr("Bookmark Properties", "@title:window"));
    d->url->setVisible(!bm.isGroup());
    d->urlLabel->setVisible(!bm.isGroup());
    d->bm = bm;
    d->title->setText(bm.fullText());
    d->url->setText(bm.url().toString());
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->comment->setText(bm.description());
    d->folderTree->setVisible(false);

    d->mode = KBookmarkDialogPrivate::EditBookmark;

    if (exec() == QDialog::Accepted) {
        return d->bm;
    } else {
        return KBookmark();
    }
}

KBookmark KBookmarkDialog::addBookmark(const QString &title, const QUrl &url, const QString &icon, KBookmark parent)
{
    if (!d->layout) {
        d->initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->mgr->root();
    }

    QPushButton *newButton = new QPushButton;
    KGuiItem::assign(newButton, KGuiItem(tr("&New Folder…", "@action:button"), QStringLiteral("folder-new")));
    d->buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    connect(newButton, &QAbstractButton::clicked, this, &KBookmarkDialog::newFolderButton);

    KGuiItem::assign(d->buttonBox->button(QDialogButtonBox::Ok), KGuiItem(tr("Add", "@action:button"), QStringLiteral("bookmark-new")));
    setWindowTitle(tr("Add Bookmark", "@title:window"));
    d->url->setVisible(true);
    d->urlLabel->setVisible(true);
    d->title->setText(title);
    d->url->setText(url.toString());
    d->comment->setText(QString());
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);
    d->icon = icon;

    d->mode = KBookmarkDialogPrivate::NewBookmark;

    if (exec() == QDialog::Accepted) {
        return d->bm;
    } else {
        return KBookmark();
    }
}

KBookmarkGroup KBookmarkDialog::addBookmarks(const QList<KBookmarkOwner::FutureBookmark> &list, const QString &name, KBookmarkGroup parent)
{
    if (!d->layout) {
        d->initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->mgr->root();
    }

    d->list = list;

    QPushButton *newButton = new QPushButton;
    KGuiItem::assign(newButton, KGuiItem(tr("&New Folder…", "@action:button"), QStringLiteral("folder-new")));
    d->buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    connect(newButton, &QAbstractButton::clicked, this, &KBookmarkDialog::newFolderButton);

    KGuiItem::assign(d->buttonBox->button(QDialogButtonBox::Ok), KGuiItem(tr("Add", "@action:button"), QStringLiteral("bookmark-new")));
    setWindowTitle(tr("Add Bookmarks", "@title:window"));
    d->url->setVisible(false);
    d->urlLabel->setVisible(false);
    d->title->setText(name);
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->comment->setText(QString());
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = KBookmarkDialogPrivate::NewMultipleBookmarks;

    if (exec() == QDialog::Accepted) {
        return d->bm.toGroup();
    } else {
        return KBookmarkGroup();
    }
}

KBookmarkGroup KBookmarkDialog::selectFolder(KBookmark parent)
{
    if (!d->layout) {
        d->initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->mgr->root();
    }

    QPushButton *newButton = new QPushButton;
    KGuiItem::assign(newButton, KGuiItem(tr("&New Folder…", "@action:button"), QStringLiteral("folder-new")));
    d->buttonBox->addButton(newButton, QDialogButtonBox::ActionRole);
    connect(newButton, &QAbstractButton::clicked, this, &KBookmarkDialog::newFolderButton);

    setWindowTitle(tr("Select Folder", "@title:window"));
    d->url->setVisible(false);
    d->urlLabel->setVisible(false);
    d->title->setVisible(false);
    d->titleLabel->setVisible(false);
    d->comment->setVisible(false);
    d->commentLabel->setVisible(false);
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = d->SelectFolder;

    if (exec() == QDialog::Accepted) {
        return d->bm.toGroup();
    } else {
        return KBookmarkGroup();
    }
}

KBookmarkGroup KBookmarkDialog::createNewFolder(const QString &name, KBookmark parent)
{
    if (!d->layout) {
        d->initLayoutPrivate();
    }
    if (parent.isNull()) {
        parent = d->mgr->root();
    }

    setWindowTitle(tr("New Folder", "@title:window"));
    d->url->setVisible(false);
    d->urlLabel->setVisible(false);
    d->comment->setVisible(true);
    d->commentLabel->setVisible(true);
    d->comment->setText(QString());
    d->title->setText(name);
    d->setParentBookmark(parent);
    d->folderTree->setVisible(true);

    d->mode = KBookmarkDialogPrivate::NewFolder;

    if (exec() == QDialog::Accepted) {
        return d->bm.toGroup();
    } else {
        return KBookmarkGroup();
    }
}

KBookmarkDialog::KBookmarkDialog(KBookmarkManager *mgr, QWidget *parent)
    : QDialog(parent)
    , d(new KBookmarkDialogPrivate(this))
{
    d->mgr = mgr;
}

KBookmarkDialog::~KBookmarkDialog() = default;

void KBookmarkDialog::newFolderButton()
{
    QString caption = d->parentBookmark().fullText().isEmpty() ? tr("Create New Bookmark Folder", "@title:window")
                                                               : tr("Create New Bookmark Folder in %1", "@title:window").arg(d->parentBookmark().text());
    bool ok;
    QString text = QInputDialog::getText(this, caption, tr("New folder:", "@label:textbox"), QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }

    KBookmarkGroup group = d->parentBookmark().createNewFolder(text);
    if (!group.isNull()) {
        KBookmarkGroup parentGroup = group.parentGroup();
        d->mgr->emitChanged(parentGroup);
        d->folderTree->clear();
        QTreeWidgetItem *root = new KBookmarkTreeItem(d->folderTree);
        d->fillGroup(root, d->mgr->root(), group);
    }
}

/********************************************************************/

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidget *tree)
    : QTreeWidgetItem(tree)
    , m_address(QLatin1String(""))
{
    setText(0, KBookmarkDialog::tr("Bookmarks", "name of the container of all browser bookmarks"));
    setIcon(0, QIcon::fromTheme(QStringLiteral("bookmarks")));
    tree->expandItem(this);
    tree->setCurrentItem(this);
    setSelected(true);
}

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidgetItem *parent, QTreeWidget *tree, const KBookmarkGroup &bk)
    : QTreeWidgetItem(parent)
{
    setIcon(0, QIcon::fromTheme(bk.icon()));
    setText(0, bk.fullText());
    tree->expandItem(this);
    m_address = bk.address();
}

KBookmarkTreeItem::~KBookmarkTreeItem()
{
}

QString KBookmarkTreeItem::address()
{
    return m_address;
}

#include "moc_kbookmarkdialog.cpp"

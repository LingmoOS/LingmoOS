/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kfiletreeview_p.h"

#include <QContextMenuEvent>
#include <QDir>
#include <QMenu>
#include <QUrl>

#include <kdirlister.h>
#include <kdirmodel.h>
#include <kdirsortfilterproxymodel.h>
#include <kfileitemdelegate.h>
#include <klocalizedstring.h>
#include <ktoggleaction.h>

class KFileTreeView::Private
{
public:
    Private(KFileTreeView *parent)
        : q(parent)
    {
    }

    QUrl urlForProxyIndex(const QModelIndex &index) const;

    void activated(const QModelIndex &);
    void currentChanged(const QModelIndex &, const QModelIndex &);
    void expanded(const QModelIndex &);

    KFileTreeView *const q;
    KDirModel *mSourceModel = nullptr;
    KDirSortFilterProxyModel *mProxyModel = nullptr;
};

QUrl KFileTreeView::Private::urlForProxyIndex(const QModelIndex &index) const
{
    const KFileItem item = mSourceModel->itemForIndex(mProxyModel->mapToSource(index));

    return !item.isNull() ? item.url() : QUrl();
}

void KFileTreeView::Private::activated(const QModelIndex &index)
{
    const QUrl url = urlForProxyIndex(index);
    if (url.isValid()) {
        Q_EMIT q->activated(url);
    }
}

void KFileTreeView::Private::currentChanged(const QModelIndex &currentIndex, const QModelIndex &)
{
    const QUrl url = urlForProxyIndex(currentIndex);
    if (url.isValid()) {
        Q_EMIT q->currentUrlChanged(url);
    }
}

void KFileTreeView::Private::expanded(const QModelIndex &baseIndex)
{
    QModelIndex index = mProxyModel->mapFromSource(baseIndex);

    q->setExpanded(index, true);
    q->selectionModel()->clearSelection();
    q->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    q->scrollTo(index);
}

KFileTreeView::KFileTreeView(QWidget *parent)
    : QTreeView(parent)
    , d(new Private(this))
{
    d->mSourceModel = new KDirModel(this);
    d->mProxyModel = new KDirSortFilterProxyModel(this);
    d->mProxyModel->setSourceModel(d->mSourceModel);

    setModel(d->mProxyModel);
    setItemDelegate(new KFileItemDelegate(this));
    setLayoutDirection(Qt::LeftToRight);

    d->mSourceModel->dirLister()->openUrl(QUrl::fromLocalFile(QDir::root().absolutePath()), KDirLister::Keep);

    connect(this, &QAbstractItemView::activated, this, [this](const QModelIndex &index) {
        d->activated(index);
    });

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current, const QModelIndex &previous) {
        d->currentChanged(current, previous);
    });

    connect(d->mSourceModel, &KDirModel::expand, this, [this](const QModelIndex &index) {
        d->expanded(index);
    });
}

KFileTreeView::~KFileTreeView()
{
    delete d;
}

QUrl KFileTreeView::currentUrl() const
{
    return d->urlForProxyIndex(currentIndex());
}

QUrl KFileTreeView::selectedUrl() const
{
    if (!selectionModel()->hasSelection()) {
        return QUrl();
    }

    const QItemSelection selection = selectionModel()->selection();
    const QModelIndex firstIndex = selection.indexes().first();

    return d->urlForProxyIndex(firstIndex);
}

QList<QUrl> KFileTreeView::selectedUrls() const
{
    QList<QUrl> urls;

    if (!selectionModel()->hasSelection()) {
        return urls;
    }

    const QModelIndexList indexes = selectionModel()->selection().indexes();
    foreach (const QModelIndex &index, indexes) {
        const QUrl url = d->urlForProxyIndex(index);
        if (url.isValid()) {
            urls.append(url);
        }
    }

    return urls;
}

QUrl KFileTreeView::rootUrl() const
{
    return d->mSourceModel->dirLister()->url();
}

void KFileTreeView::setDirOnlyMode(bool enabled)
{
    d->mSourceModel->dirLister()->setDirOnlyMode(enabled);
    d->mSourceModel->dirLister()->openUrl(d->mSourceModel->dirLister()->url());
}

void KFileTreeView::setShowHiddenFiles(bool enabled)
{
    QUrl url = currentUrl();
    d->mSourceModel->dirLister()->setShowHiddenFiles(enabled);
    d->mSourceModel->dirLister()->openUrl(d->mSourceModel->dirLister()->url());
    setCurrentUrl(url);
}

void KFileTreeView::setCurrentUrl(const QUrl &url)
{
    const QModelIndex baseIndex = d->mSourceModel->indexForUrl(url);

    if (!baseIndex.isValid()) {
        d->mSourceModel->expandToUrl(url);
        return;
    }

    const QModelIndex proxyIndex = d->mProxyModel->mapFromSource(baseIndex);
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
    scrollTo(proxyIndex);
}

void KFileTreeView::setRootUrl(const QUrl &url)
{
    d->mSourceModel->dirLister()->openUrl(url);
}

void KFileTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    KToggleAction *showHiddenAction = new KToggleAction(i18n("Show Hidden Folders"), &menu);
    showHiddenAction->setChecked(d->mSourceModel->dirLister()->showHiddenFiles());
    connect(showHiddenAction, &QAction::toggled, this, &KFileTreeView::setShowHiddenFiles);

    menu.addAction(showHiddenAction);
    menu.exec(event->globalPos());
}

bool KFileTreeView::showHiddenFiles() const
{
    return d->mSourceModel->dirLister()->showHiddenFiles();
}

QSize KFileTreeView::sizeHint() const
{
    // This size makes KDirSelectDialog pop up just under 800x600 by default :-)
    return QSize(680, 500);
}

#include "moc_kfiletreeview_p.cpp"

/*
   SPDX-FileCopyrightText: 2003 Scott Wheeler <wheeler@kde.org>
   SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
   SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
   SPDX-FileCopyrightText: 2007 Pino Toscano <pino@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktreeviewsearchline.h"

#include <QActionGroup>
#include <QApplication>
#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>

#include <QDebug>
#include <klocalizedstring.h>

class KTreeViewSearchLinePrivate
{
public:
    KTreeViewSearchLinePrivate(KTreeViewSearchLine *_parent)
        : parent(_parent)
        , caseSensitive(Qt::CaseInsensitive)
        , activeSearch(false)
        , keepParentsVisible(true)
        , canChooseColumns(true)
        , queuedSearches(0)
    {
    }

    KTreeViewSearchLine *parent;
    QList<QTreeView *> treeViews;
    Qt::CaseSensitivity caseSensitive;
    bool activeSearch;
    bool keepParentsVisible;
    bool canChooseColumns;
    QString search;
    int queuedSearches;
    QList<int> searchColumns;

    void rowsInserted(QAbstractItemModel *model, const QModelIndex &parent, int start, int end) const;
    void treeViewDeleted(QObject *treeView);
    void slotColumnActivated(QAction *action);
    void slotAllVisibleColumns();

    void checkColumns();
    void checkItemParentsNotVisible(QTreeView *treeView);
    bool checkItemParentsVisible(QTreeView *treeView, const QModelIndex &index);
};

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////
void KTreeViewSearchLine::rowsInserted(const QModelIndex &parentIndex, int start, int end) const
{
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(sender());
    d->rowsInserted(model, parentIndex, start, end);
}

void KTreeViewSearchLinePrivate::rowsInserted(QAbstractItemModel *model, const QModelIndex &parentIndex, int start, int end) const
{
    // QAbstractItemModel* model = qobject_cast<QAbstractItemModel*>( parent->sender() );
    if (!model) {
        return;
    }

    const auto it = std::find_if(treeViews.cbegin(), treeViews.cend(), [=](QTreeView *tree) {
        return tree->model() == model;
    });
    if (it == treeViews.cend()) {
        return;
    }

    QTreeView *widget = *it;
    for (int i = start; i <= end; ++i) {
        widget->setRowHidden(i, parentIndex, !parent->itemMatches(parentIndex, i, parent->text()));
    }
}

void KTreeViewSearchLinePrivate::treeViewDeleted(QObject *object)
{
    treeViews.removeAll(static_cast<QTreeView *>(object));
    parent->setEnabled(treeViews.isEmpty());
}

void KTreeViewSearchLinePrivate::slotColumnActivated(QAction *action)
{
    if (!action) {
        return;
    }

    bool ok;
    int column = action->data().toInt(&ok);

    if (!ok) {
        return;
    }

    if (action->isChecked()) {
        if (!searchColumns.isEmpty()) {
            if (!searchColumns.contains(column)) {
                searchColumns.append(column);
            }

            if (searchColumns.count() == treeViews.first()->header()->count() - treeViews.first()->header()->hiddenSectionCount()) {
                searchColumns.clear();
            }

        } else {
            searchColumns.append(column);
        }
    } else {
        if (searchColumns.isEmpty()) {
            QHeaderView *const header = treeViews.first()->header();

            for (int i = 0; i < header->count(); i++) {
                if (i != column && !header->isSectionHidden(i)) {
                    searchColumns.append(i);
                }
            }

        } else if (searchColumns.contains(column)) {
            searchColumns.removeAll(column);
        }
    }

    parent->updateSearch();
}

void KTreeViewSearchLinePrivate::slotAllVisibleColumns()
{
    if (searchColumns.isEmpty()) {
        searchColumns.append(0);
    } else {
        searchColumns.clear();
    }

    parent->updateSearch();
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void KTreeViewSearchLinePrivate::checkColumns()
{
    canChooseColumns = parent->canChooseColumnsCheck();
}

void KTreeViewSearchLinePrivate::checkItemParentsNotVisible(QTreeView *treeView)
{
    Q_UNUSED(treeView)

// TODO: PORT ME
#if 0
  QTreeWidgetItemIterator it( treeWidget );

  for ( ; *it; ++it ) {
    QTreeWidgetItem *item = *it;
    item->treeWidget()->setItemHidden( item, !parent->itemMatches( item, search ) );
  }
#endif
}

/** Check whether \p item, its siblings and their descendents should be shown. Show or hide the items as necessary.
 *
 *  \p item  The list view item to start showing / hiding items at. Typically, this is the first child of another item, or the
 *              the first child of the list view.
 *  \return \c true if an item which should be visible is found, \c false if all items found should be hidden. If this function
 *             returns true and \p highestHiddenParent was not 0, highestHiddenParent will have been shown.
 */
bool KTreeViewSearchLinePrivate::checkItemParentsVisible(QTreeView *treeView, const QModelIndex &index)
{
    bool childMatch = false;
    const int rowcount = treeView->model()->rowCount(index);
    for (int i = 0; i < rowcount; ++i) {
        childMatch |= checkItemParentsVisible(treeView, treeView->model()->index(i, 0, index));
    }

    // Should this item be shown? It should if any children should be, or if it matches.
    const QModelIndex parentindex = index.parent();
    if (childMatch || parent->itemMatches(parentindex, index.row(), search)) {
        treeView->setRowHidden(index.row(), parentindex, false);
        return true;
    }

    treeView->setRowHidden(index.row(), parentindex, true);

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

KTreeViewSearchLine::KTreeViewSearchLine(QWidget *parent, QTreeView *treeView)
    : KLineEdit(parent)
    , d(new KTreeViewSearchLinePrivate(this))
{
    connect(this, &QLineEdit::textChanged, this, &KTreeViewSearchLine::queueSearch);

    setClearButtonEnabled(true);
    setTreeView(treeView);

    if (!treeView) {
        setEnabled(false);
    }
}

KTreeViewSearchLine::KTreeViewSearchLine(QWidget *parent, const QList<QTreeView *> &treeViews)
    : KLineEdit(parent)
    , d(new KTreeViewSearchLinePrivate(this))
{
    connect(this, &QLineEdit::textChanged, this, &KTreeViewSearchLine::queueSearch);

    setClearButtonEnabled(true);
    setTreeViews(treeViews);
}

KTreeViewSearchLine::~KTreeViewSearchLine()
{
    delete d;
}

Qt::CaseSensitivity KTreeViewSearchLine::caseSensitivity() const
{
    return d->caseSensitive;
}

QList<int> KTreeViewSearchLine::searchColumns() const
{
    if (d->canChooseColumns) {
        return d->searchColumns;
    } else {
        return QList<int>();
    }
}

bool KTreeViewSearchLine::keepParentsVisible() const
{
    return d->keepParentsVisible;
}

QTreeView *KTreeViewSearchLine::treeView() const
{
    if (d->treeViews.count() == 1) {
        return d->treeViews.first();
    } else {
        return nullptr;
    }
}

QList<QTreeView *> KTreeViewSearchLine::treeViews() const
{
    return d->treeViews;
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void KTreeViewSearchLine::addTreeView(QTreeView *treeView)
{
    if (treeView) {
        connectTreeView(treeView);

        d->treeViews.append(treeView);
        setEnabled(!d->treeViews.isEmpty());

        d->checkColumns();
    }
}

void KTreeViewSearchLine::removeTreeView(QTreeView *treeView)
{
    if (treeView) {
        int index = d->treeViews.indexOf(treeView);

        if (index != -1) {
            d->treeViews.removeAt(index);
            d->checkColumns();

            disconnectTreeView(treeView);

            setEnabled(!d->treeViews.isEmpty());
        }
    }
}

void KTreeViewSearchLine::updateSearch(const QString &pattern)
{
    d->search = pattern.isNull() ? text() : pattern;

    for (QTreeView *treeView : std::as_const(d->treeViews)) {
        updateSearch(treeView);
    }
}

void KTreeViewSearchLine::updateSearch(QTreeView *treeView)
{
    if (!treeView || !treeView->model()->rowCount()) {
        return;
    }

    // If there's a selected item that is visible, make sure that it's visible
    // when the search changes too (assuming that it still matches).

    QModelIndex currentIndex = treeView->currentIndex();

    bool wasUpdateEnabled = treeView->updatesEnabled();
    treeView->setUpdatesEnabled(false);
    if (d->keepParentsVisible) {
        for (int i = 0; i < treeView->model()->rowCount(); ++i) {
            d->checkItemParentsVisible(treeView, treeView->rootIndex());
        }
    } else {
        d->checkItemParentsNotVisible(treeView);
    }
    treeView->setUpdatesEnabled(wasUpdateEnabled);

    if (currentIndex.isValid()) {
        treeView->scrollTo(currentIndex);
    }
}

void KTreeViewSearchLine::setCaseSensitivity(Qt::CaseSensitivity caseSensitive)
{
    if (d->caseSensitive != caseSensitive) {
        d->caseSensitive = caseSensitive;
        updateSearch();
    }
}

void KTreeViewSearchLine::setKeepParentsVisible(bool visible)
{
    if (d->keepParentsVisible != visible) {
        d->keepParentsVisible = visible;
        updateSearch();
    }
}

void KTreeViewSearchLine::setSearchColumns(const QList<int> &columns)
{
    if (d->canChooseColumns) {
        d->searchColumns = columns;
    }
}

void KTreeViewSearchLine::setTreeView(QTreeView *treeView)
{
    setTreeViews(QList<QTreeView *>());
    addTreeView(treeView);
}

void KTreeViewSearchLine::setTreeViews(const QList<QTreeView *> &treeViews)
{
    for (QTreeView *treeView : std::as_const(d->treeViews)) {
        disconnectTreeView(treeView);
    }

    d->treeViews = treeViews;

    for (QTreeView *treeView : std::as_const(d->treeViews)) {
        connectTreeView(treeView);
    }

    d->checkColumns();

    setEnabled(!d->treeViews.isEmpty());
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

bool KTreeViewSearchLine::itemMatches(const QModelIndex &index, int row, const QString &pattern) const
{
    if (pattern.isEmpty()) {
        return true;
    }

    if (!index.isValid()) {
        return false;
    }

    // If the search column list is populated, search just the columns
    // specifified.  If it is empty default to searching all of the columns.

    const int columncount = index.model()->columnCount(index);
    if (!d->searchColumns.isEmpty()) {
        QList<int>::ConstIterator it = d->searchColumns.constBegin();
        for (; it != d->searchColumns.constEnd(); ++it) {
            if (*it < columncount && index.model()->index(row, *it, index).data(Qt::DisplayRole).toString().indexOf(pattern, 0, d->caseSensitive) >= 0) {
                return true;
            }
        }
    } else {
        for (int i = 0; i < columncount; ++i) {
            if (index.model()->index(row, i, index).data(Qt::DisplayRole).toString().indexOf(pattern, 0, d->caseSensitive) >= 0) {
                return true;
            }
        }
    }

    return false;
}

void KTreeViewSearchLine::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = KLineEdit::createStandardContextMenu();

    if (d->canChooseColumns) {
        popup->addSeparator();
        QMenu *subMenu = popup->addMenu(i18n("Search Columns"));

        QAction *allVisibleColumnsAction = subMenu->addAction(i18n("All Visible Columns"), this, SLOT(slotAllVisibleColumns()));
        allVisibleColumnsAction->setCheckable(true);
        allVisibleColumnsAction->setChecked(!d->searchColumns.count());
        subMenu->addSeparator();

        bool allColumnsAreSearchColumns = true;

        QActionGroup *group = new QActionGroup(popup);
        group->setExclusive(false);
        connect(group, SIGNAL(triggered(QAction *)), SLOT(slotColumnActivated(QAction *)));

        QHeaderView *const header = d->treeViews.first()->header();
        for (int j = 0; j < header->count(); j++) {
            int i = header->logicalIndex(j);

            if (header->isSectionHidden(i)) {
                continue;
            }

            QString columnText = header->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
            QAction *columnAction = subMenu->addAction(qvariant_cast<QIcon>(header->model()->headerData(i, Qt::Horizontal, Qt::DecorationRole)), columnText);
            columnAction->setCheckable(true);
            columnAction->setChecked(d->searchColumns.isEmpty() || d->searchColumns.contains(i));
            columnAction->setData(i);
            columnAction->setActionGroup(group);

            if (d->searchColumns.isEmpty() || d->searchColumns.indexOf(i) != -1) {
                columnAction->setChecked(true);
            } else {
                allColumnsAreSearchColumns = false;
            }
        }

        allVisibleColumnsAction->setChecked(allColumnsAreSearchColumns);

        // searchColumnsMenuActivated() relies on one possible "all" representation
        if (allColumnsAreSearchColumns && !d->searchColumns.isEmpty()) {
            d->searchColumns.clear();
        }
    }

    popup->exec(event->globalPos());
    delete popup;
}

void KTreeViewSearchLine::connectTreeView(QTreeView *treeView)
{
    connect(treeView, SIGNAL(destroyed(QObject *)), this, SLOT(treeViewDeleted(QObject *)));

    connect(treeView->model(), &QAbstractItemModel::rowsInserted, this, &KTreeViewSearchLine::rowsInserted);
}

void KTreeViewSearchLine::disconnectTreeView(QTreeView *treeView)
{
    disconnect(treeView, SIGNAL(destroyed(QObject *)), this, SLOT(treeViewDeleted(QObject *)));

    disconnect(treeView->model(), &QAbstractItemModel::rowsInserted, this, &KTreeViewSearchLine::rowsInserted);
}

bool KTreeViewSearchLine::canChooseColumnsCheck()
{
    // This is true if either of the following is true:

    // there are no listviews connected
    if (d->treeViews.isEmpty()) {
        return false;
    }

    const QTreeView *first = d->treeViews.first();

    const int numcols = first->model()->columnCount();
    // the listviews have only one column,
    if (numcols < 2) {
        return false;
    }

    QStringList headers;
    for (int i = 0; i < numcols; ++i) {
        headers.append(first->header()->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
    }

    QList<QTreeView *>::ConstIterator it = d->treeViews.constBegin();
    for (++it /* skip the first one */; it != d->treeViews.constEnd(); ++it) {
        // the listviews have different numbers of columns,
        if ((*it)->model()->columnCount() != numcols) {
            return false;
        }

        // the listviews differ in column labels.
        QStringList::ConstIterator jt;
        int i;
        for (i = 0, jt = headers.constBegin(); i < numcols; ++i, ++jt) {
            Q_ASSERT(jt != headers.constEnd());

            if ((*it)->header()->model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() != *jt) {
                return false;
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// protected slots
////////////////////////////////////////////////////////////////////////////////

void KTreeViewSearchLine::queueSearch(const QString &search)
{
    d->queuedSearches++;
    d->search = search;

    QTimer::singleShot(200, this, &KTreeViewSearchLine::activateSearch);
}

void KTreeViewSearchLine::activateSearch()
{
    --(d->queuedSearches);

    if (d->queuedSearches == 0) {
        updateSearch(d->search);
    }
}

////////////////////////////////////////////////////////////////////////////////
// KTreeViewSearchLineWidget
////////////////////////////////////////////////////////////////////////////////

class KTreeViewSearchLineWidgetPrivate
{
public:
    KTreeViewSearchLineWidgetPrivate()
        : treeView(nullptr)
        , searchLine(nullptr)
    {
    }

    QTreeView *treeView;
    KTreeViewSearchLine *searchLine;
};

KTreeViewSearchLineWidget::KTreeViewSearchLineWidget(QWidget *parent, QTreeView *treeView)
    : QWidget(parent)
    , d(new KTreeViewSearchLineWidgetPrivate)
{
    d->treeView = treeView;

    QTimer::singleShot(0, this, &KTreeViewSearchLineWidget::createWidgets);
}

KTreeViewSearchLineWidget::~KTreeViewSearchLineWidget()
{
    delete d;
}

KTreeViewSearchLine *KTreeViewSearchLineWidget::createSearchLine(QTreeView *treeView) const
{
    return new KTreeViewSearchLine(const_cast<KTreeViewSearchLineWidget *>(this), treeView);
}

void KTreeViewSearchLineWidget::createWidgets()
{
    QLabel *label = new QLabel(i18n("S&earch:"), this);
    label->setObjectName(QLatin1String("kde toolbar widget"));

    searchLine()->show();

    label->setBuddy(d->searchLine);
    label->show();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(label);
    layout->addWidget(d->searchLine);
}

KTreeViewSearchLine *KTreeViewSearchLineWidget::searchLine() const
{
    if (!d->searchLine) {
        d->searchLine = createSearchLine(d->treeView);
    }

    return d->searchLine;
}

#include "moc_ktreeviewsearchline.cpp"

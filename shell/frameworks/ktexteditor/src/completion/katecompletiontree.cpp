/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecompletiontree.h"

#include <QApplication>
#include <QHeaderView>
#include <QList>
#include <QScreen>
#include <QScrollBar>
#include <QTimer>

#include "kateconfig.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "kateview.h"

#include "documentation_tip.h"
#include "katecompletiondelegate.h"
#include "katecompletionmodel.h"
#include "katecompletionwidget.h"

KateCompletionTree::KateCompletionTree(KateCompletionWidget *parent)
    : QTreeView(parent)
{
    m_scrollingEnabled = true;
    header()->hide();
    setRootIsDecorated(false);
    setIndentation(0);
    setFrameStyle(QFrame::NoFrame);
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    header()->setMinimumSectionSize(0);

    // We need ScrollPerItem, because ScrollPerPixel is too slow with a very large completion-list(see KDevelop).
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

    m_resizeTimer = new QTimer(this);
    m_resizeTimer->setSingleShot(true);

    connect(m_resizeTimer, &QTimer::timeout, this, &KateCompletionTree::resizeColumnsSlot);

    // Provide custom highlighting to completion entries
    setItemDelegate(new KateCompletionDelegate(this));
    // make sure we adapt to size changes when the model got reset
    // this is important for delayed creation of groups, without this
    // the first column would never get resized to the correct size
    connect(widget()->model(), &QAbstractItemModel::modelReset, this, &KateCompletionTree::scheduleUpdate, Qt::QueuedConnection);

    // Prevent user from expanding / collapsing with the mouse
    setItemsExpandable(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void KateCompletionTree::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    // If config is enabled OR the tip is already visible => show / update it
    if (widget()->view()->config()->showDocWithCompletion() || widget()->docTip()->isVisible()) {
        widget()->showDocTip(current);
    }
    widget()->model()->rowSelected(current);
    QTreeView::currentChanged(current, previous);
}

void KateCompletionTree::setScrollingEnabled(bool enabled)
{
    m_scrollingEnabled = enabled;
}

void KateCompletionTree::scrollContentsBy(int dx, int dy)
{
    if (m_scrollingEnabled) {
        QTreeView::scrollContentsBy(dx, dy);
    }

    if (isVisible()) {
        scheduleUpdate();
    }
}

KateCompletionWidget *KateCompletionTree::widget() const
{
    return static_cast<KateCompletionWidget *>(const_cast<QObject *>(parent()));
}

void KateCompletionTree::resizeColumnsSlot()
{
    if (model()) {
        resizeColumns();

        if (!widget()->docTip()->isHidden()) {
            widget()->docTip()->updatePosition(widget());
        }
    }
}

/**
 * Measure the width of visible columns.
 *
 * This iterates from the start index @p current down until a dead end is hit.
 * In a tree model, it will recurse into child indices. Iteration is stopped if
 * no more items are available, or the visited rows exceed the available @p maxHeight.
 *
 * If the model is a tree model, and @p current points to a leaf, and the max height
 * is not exceeded, then iteration will continue from the next parent sibling.
 */
static bool measureColumnSizes(const KateCompletionTree *tree,
                               QModelIndex current,
                               QVarLengthArray<int, 8> &columnSize,
                               int &currentYPos,
                               const int maxHeight,
                               bool recursed = false)
{
    while (current.isValid() && currentYPos < maxHeight) {
        currentYPos += tree->sizeHintForIndex(current).height();
        const int row = current.row();
        for (int a = 0; a < columnSize.size(); a++) {
            QSize s = tree->sizeHintForIndex(current.sibling(row, a));
            if (s.width() > 2000) {
                qCDebug(LOG_KTE) << "got invalid size-hint of width " << s.width();
            } else if (s.width() > columnSize[a]) {
                columnSize[a] = s.width();
            }
        }

        const QAbstractItemModel *model = current.model();
        const int children = model->rowCount(current);
        if (children > 0) {
            for (int i = 0; i < children; ++i) {
                if (measureColumnSizes(tree, model->index(i, 0, current), columnSize, currentYPos, maxHeight, true)) {
                    break;
                }
            }
        }

        QModelIndex oldCurrent = current;
        current = current.sibling(current.row() + 1, 0);

        // Are we at the end of a group? If yes, move up into the next group
        // only do this when we did not recurse already
        while (!recursed && !current.isValid() && oldCurrent.parent().isValid()) {
            oldCurrent = oldCurrent.parent();
            current = oldCurrent.sibling(oldCurrent.row() + 1, 0);
        }
    }

    return currentYPos >= maxHeight;
}

void KateCompletionTree::resizeColumns(bool firstShow, bool forceResize)
{
    static bool preventRecursion = false;
    if (preventRecursion) {
        return;
    }
    m_resizeTimer->stop();

    if (firstShow) {
        forceResize = true;
    }

    preventRecursion = true;

    widget()->setUpdatesEnabled(false);

    int modelIndexOfName = kateModel()->translateColumn(KTextEditor::CodeCompletionModel::Name);
    int oldIndentWidth = columnViewportPosition(modelIndexOfName);

    /// Step 1: Compute the needed column-sizes for the visible content
    const int numColumns = model()->columnCount();
    QVarLengthArray<int, 8> columnSize(numColumns);
    for (int i = 0; i < numColumns; ++i) {
        columnSize[i] = 0;
    }
    QModelIndex current = indexAt(QPoint(1, 1));
    //    const bool changed = current.isValid();
    int currentYPos = 0;
    measureColumnSizes(this, current, columnSize, currentYPos, height());

    auto totalColumnsWidth = 0;
    auto originalViewportWidth = viewport()->width();

    const int maxWidth = (widget()->parentWidget()->geometry().width()) / 2;

    /// Step 2: Update column-sizes
    // This contains several hacks to reduce the amount of resizing that happens. Generally,
    // resizes only happen if a) More than a specific amount of space is saved by the resize, or
    // b) the resizing is required so the list can show all of its contents.
    int minimumResize = 0;
    int maximumResize = 0;

    for (int n = 0; n < numColumns; n++) {
        totalColumnsWidth += columnSize[n];

        int diff = columnSize[n] - columnWidth(n);
        if (diff < minimumResize) {
            minimumResize = diff;
        }
        if (diff > maximumResize) {
            maximumResize = diff;
        }
    }

    int noReduceTotalWidth = 0; // The total width of the widget of no columns are reduced
    for (int n = 0; n < numColumns; n++) {
        if (columnSize[n] < columnWidth(n)) {
            noReduceTotalWidth += columnWidth(n);
        } else {
            noReduceTotalWidth += columnSize[n];
        }
    }

    // Check whether we can afford to reduce none of the columns
    // Only reduce size if we widget would else be too wide.
    bool noReduce = noReduceTotalWidth < maxWidth && !forceResize;

    if (noReduce) {
        totalColumnsWidth = 0;
        for (int n = 0; n < numColumns; n++) {
            if (columnSize[n] < columnWidth(n)) {
                columnSize[n] = columnWidth(n);
            }

            totalColumnsWidth += columnSize[n];
        }
    }

    if (minimumResize > -40 && maximumResize == 0 && !forceResize) {
        // No column needs to be exanded, and no column needs to be reduced by more than 40 pixels.
        // To prevent flashing, do not resize at all.
        totalColumnsWidth = 0;
        for (int n = 0; n < numColumns; n++) {
            columnSize[n] = columnWidth(n);
            totalColumnsWidth += columnSize[n];
        }
    } else {
        //       viewport()->resize( 5000, viewport()->height() );
        for (int n = 0; n < numColumns; n++) {
            setColumnWidth(n, columnSize[n]);
        }
        // For the first column (which is arrow-down / arrow-right) we keep its width to 20
        // to prevent glitches and weird resizes when we have no expanding items in the view
        //       qCDebug(LOG_KTE) << "resizing viewport to" << totalColumnsWidth;
        viewport()->resize(totalColumnsWidth, viewport()->height());
    }

    /// Step 3: Update widget-size and -position

    int scrollBarWidth = verticalScrollBar()->width();

    int newIndentWidth = columnViewportPosition(modelIndexOfName);

    int newWidth = qMin(maxWidth, qMax(75, totalColumnsWidth));

    if (newWidth == maxWidth) {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if (maximumResize > 0 || forceResize || oldIndentWidth != newIndentWidth) {
        //   qCDebug(LOG_KTE) << geometry() << "newWidth" << newWidth << "current width" << width() << "target width" << newWidth + scrollBarWidth;

        if ((newWidth + scrollBarWidth) != width() && originalViewportWidth != totalColumnsWidth) {
            auto width = newWidth + scrollBarWidth + 2;
            widget()->resize(width, widget()->height());
            resize(width, widget()->height() - (2 * widget()->frameWidth()));
        }

        //   qCDebug(LOG_KTE) << "created geometry:" << widget()->geometry() << geometry() << "newWidth" << newWidth << "viewport" << viewport()->width();

        if (viewport()->width() > totalColumnsWidth) { // Set the size of the last column to fill the whole rest of the widget
            setColumnWidth(numColumns - 1, viewport()->width() - columnViewportPosition(numColumns - 1));
        }

        /*  for(int a = 0; a < numColumns; ++a)
            qCDebug(LOG_KTE) << "column" << a << columnWidth(a) << "target:" << columnSize[a];*/

        if (oldIndentWidth != newIndentWidth) {
            if (!forceResize) {
                preventRecursion = false;
                resizeColumns(true, true);
            }
        }
    }

    widget()->setUpdatesEnabled(true);

    preventRecursion = false;
}

void KateCompletionTree::initViewItemOption(QStyleOptionViewItem *option) const
{
    QTreeView::initViewItemOption(option);
    option->font = widget()->view()->renderer()->currentFont();
}

KateCompletionModel *KateCompletionTree::kateModel() const
{
    return static_cast<KateCompletionModel *>(model());
}

bool KateCompletionTree::nextCompletion()
{
    QModelIndex current;
    QModelIndex firstCurrent = currentIndex();

    do {
        QModelIndex oldCurrent = currentIndex();

        current = moveCursor(MoveDown, Qt::NoModifier);

        if (current != oldCurrent && current.isValid()) {
            setCurrentIndex(current);
        } else {
            if (firstCurrent.isValid()) {
                setCurrentIndex(firstCurrent);
            }
            return false;
        }

    } while (!kateModel()->indexIsItem(current));

    return true;
}

bool KateCompletionTree::previousCompletion()
{
    QModelIndex current;
    QModelIndex firstCurrent = currentIndex();

    do {
        QModelIndex oldCurrent = currentIndex();

        current = moveCursor(MoveUp, Qt::NoModifier);

        if (current != oldCurrent && current.isValid()) {
            setCurrentIndex(current);

        } else {
            if (firstCurrent.isValid()) {
                setCurrentIndex(firstCurrent);
            }
            return false;
        }

    } while (!kateModel()->indexIsItem(current));

    return true;
}

bool KateCompletionTree::pageDown()
{
    QModelIndex old = currentIndex();

    QModelIndex current = moveCursor(MovePageDown, Qt::NoModifier);

    if (current.isValid()) {
        setCurrentIndex(current);
        if (!kateModel()->indexIsItem(current)) {
            if (!nextCompletion()) {
                previousCompletion();
            }
        }
    }

    return current != old;
}

bool KateCompletionTree::pageUp()
{
    QModelIndex old = currentIndex();
    QModelIndex current = moveCursor(MovePageUp, Qt::NoModifier);

    if (current.isValid()) {
        setCurrentIndex(current);
        if (!kateModel()->indexIsItem(current)) {
            if (!previousCompletion()) {
                nextCompletion();
            }
        }
    }
    return current != old;
}

void KateCompletionTree::top()
{
    QModelIndex current = moveCursor(MoveHome, Qt::NoModifier);
    setCurrentIndex(current);

    if (current.isValid()) {
        setCurrentIndex(current);
        if (!kateModel()->indexIsItem(current)) {
            nextCompletion();
        }
    }
}

void KateCompletionTree::scheduleUpdate()
{
    m_resizeTimer->start(0);
}

void KateCompletionTree::bottom()
{
    QModelIndex current = moveCursor(MoveEnd, Qt::NoModifier);
    setCurrentIndex(current);

    if (current.isValid()) {
        setCurrentIndex(current);
        if (!kateModel()->indexIsItem(current)) {
            previousCompletion();
        }
    }
}

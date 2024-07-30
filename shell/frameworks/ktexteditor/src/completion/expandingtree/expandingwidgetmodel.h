/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EXPANDING_WIDGET_MODEL_H
#define EXPANDING_WIDGET_MODEL_H

#include <QAbstractTableModel>
#include <QIcon>
#include <QPointer>

class QTreeView;

/**
 * Cares about expanding/un-expanding items in a tree-view together with ExpandingDelegate
 */
// TODO: Merge this into KateCompletionModel
class ExpandingWidgetModel : public QAbstractItemModel
{
public:
    explicit ExpandingWidgetModel(QWidget *parent);
    ~ExpandingWidgetModel() override;

    enum ExpandingType { NotExpandable = 0, Expandable, Expanded };

    virtual QTreeView *treeView() const = 0;

    /// Should return true if the given row should be painted like a contained item(as opposed to label-rows etc.)
    virtual bool indexIsItem(const QModelIndex &index) const = 0;

    /// Does not request data from index, this only returns local data like highlighting for expanded rows and similar
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /// Returns the match-color for the given index, or zero if match-quality could not be computed.
    uint matchColor(const QModelIndex &index) const;

protected:
    /**
     * @return the context-match quality from 0 to 10 if it could be determined, else -1
     * */
    virtual int contextMatchQuality(const QModelIndex &index) const = 0;
};

/**
 * Helper-function to merge custom-highlighting variant-lists.
 *
 * @param strings A list of strings that should be merged
 * @param highlights One variant-list for highlighting, as described in the kde header ktextedtor/codecompletionmodel.h
 * @param gapBetweenStrings How many signs are inserted between 2 strings?
 * */
QList<QVariant> mergeCustomHighlighting(QStringList strings, QList<QVariantList> highlights, int gapBetweenStrings = 0);
#endif

/*
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecompletionmodel.h"

#include "document.h"
#include "view.h"

using namespace KTextEditor;

class KTextEditor::CodeCompletionModelPrivate
{
public:
    CodeCompletionModelPrivate()
    {
    }

    int rowCount = 0;
    bool hasGroups = false;
};

CodeCompletionModel::CodeCompletionModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new CodeCompletionModelPrivate)
{
}

CodeCompletionModel::~CodeCompletionModel()
{
    delete d;
}

int CodeCompletionModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

QModelIndex CodeCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= d->rowCount || column < 0 || column >= ColumnCount || parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column, (void *)nullptr);
}

QMap<int, QVariant> CodeCompletionModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> ret = QAbstractItemModel::itemData(index);

    for (int i = CompletionRole; i <= AccessibilityAccept; ++i) {
        QVariant v = data(index, i);
        if (v.isValid()) {
            ret.insert(i, v);
        }
    }

    return ret;
}

QModelIndex CodeCompletionModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

void CodeCompletionModel::setRowCount(int rowCount)
{
    d->rowCount = rowCount;
}

int CodeCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->rowCount;
}

void CodeCompletionModel::completionInvoked(KTextEditor::View *view, const Range &range, InvocationType invocationType)
{
    Q_UNUSED(view)
    Q_UNUSED(range)
    Q_UNUSED(invocationType)
}

void CodeCompletionModel::executeCompletionItem(KTextEditor::View *view, const Range &word, const QModelIndex &index) const
{
    view->document()->replaceText(word, data(index.sibling(index.row(), Name)).toString());
}

bool CodeCompletionModel::hasGroups() const
{
    return d->hasGroups;
}

void CodeCompletionModel::setHasGroups(bool hasGroups)
{
    if (d->hasGroups != hasGroups) {
        d->hasGroups = hasGroups;
        Q_EMIT hasGroupsChanged(this, hasGroups);
    }
}

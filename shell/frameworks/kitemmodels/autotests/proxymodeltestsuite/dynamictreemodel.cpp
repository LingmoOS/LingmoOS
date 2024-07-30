/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dynamictreemodel.h"

#include <QMimeData>

#include <QDebug>

#include <QStringList>

// If DUMPTREE is defined, ModelInsertCommand dumps the tree of what it is inserting.
// #define DUMPTREE
#ifdef DUMPTREE
#include <iostream>
#endif

DynamicTreeModel::DynamicTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , nextId(1)
{
}

QModelIndex DynamicTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    //   if (column != 0)
    //     return QModelIndex();

    if (column < 0 || row < 0) {
        return QModelIndex();
    }

    QList<QList<qint64>> childIdColumns = m_childItems.value(parent.internalId());

    const qint64 grandParent = findParentId(parent.internalId());
    if (grandParent >= 0) {
        QList<QList<qint64>> parentTable = m_childItems.value(grandParent);
        Q_ASSERT(parent.column() < parentTable.size());
        QList<qint64> parentSiblings = parentTable.at(parent.column());
        Q_ASSERT(parent.row() < parentSiblings.size());
    }

    if (childIdColumns.size() == 0) {
        return QModelIndex();
    }

    if (column >= childIdColumns.size()) {
        return QModelIndex();
    }

    QList<qint64> rowIds = childIdColumns.at(column);

    if (row >= rowIds.size()) {
        return QModelIndex();
    }

    qint64 id = rowIds.at(row);

    return createIndex(row, column, reinterpret_cast<void *>(id));
}

qint64 DynamicTreeModel::findParentId(qint64 searchId) const
{
    if (searchId <= 0) {
        return -1;
    }

    QHashIterator<qint64, QList<QList<qint64>>> i(m_childItems);
    while (i.hasNext()) {
        i.next();
        QListIterator<QList<qint64>> j(i.value());
        while (j.hasNext()) {
            QList<qint64> l = j.next();
            if (l.contains(searchId)) {
                return i.key();
            }
        }
    }
    return -1;
}

QModelIndex DynamicTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    qint64 searchId = index.internalId();
    qint64 parentId = findParentId(searchId);
    // Will never happen for valid index, but what the hey...
    if (parentId <= 0) {
        return QModelIndex();
    }

    qint64 grandParentId = findParentId(parentId);
    if (grandParentId < 0) {
        grandParentId = 0;
    }

    int column = 0;
    QList<qint64> childList = m_childItems.value(grandParentId).at(column);

    int row = childList.indexOf(parentId);

    return createIndex(row, column, reinterpret_cast<void *>(parentId));
}

int DynamicTreeModel::rowCount(const QModelIndex &index) const
{
    QList<QList<qint64>> cols = m_childItems.value(index.internalId());

    if (cols.size() == 0) {
        return 0;
    }

    if (index.column() > 0) {
        return 0;
    }

    return cols.at(0).size();
}

int DynamicTreeModel::columnCount(const QModelIndex &index) const
{
    //   Q_UNUSED(index);
    return m_childItems.value(index.internalId()).size();
}

QVariant DynamicTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (DynamicTreeModelId == role) {
        return index.internalId();
    }

    if (Qt::DisplayRole == role || Qt::EditRole == role) {
        return m_items.value(index.internalId());
    }
    return QVariant();
}

bool DynamicTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        m_items[index.internalId()] = value.toString();
        dataChanged(index, index);
        return true;
    }

    return QAbstractItemModel::setData(index, value, role);
}

void DynamicTreeModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_childItems.clear();
    nextId = 1;
    endResetModel();
}

bool DynamicTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int _column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(_column);
    QByteArray encoded = data->data(mimeTypes().at(0));

    QHash<QModelIndex, QList<int>> movedItems;
    bool ok;
    qint64 id;
    int _row;
    static const int column = 0;
    QHash<qint64, QList<QList<qint64>>>::const_iterator it;
    const auto lst = encoded.split('\0');
    for (const QByteArray &ba : lst) {
        id = ba.toInt(&ok);
        if (!ok) {
            qDebug() << ba;
        }
        Q_ASSERT(ok);

        _row = -1;
        for (it = m_childItems.constBegin(); it != m_childItems.constEnd(); ++it) {
            _row = it.value().first().indexOf(id);
            if (_row < 0) {
                continue;
            }
            movedItems[createIndex(_row, column, reinterpret_cast<void *>(id)).parent()].append(_row);
            break;
        }
        Q_ASSERT(_row >= 0);
        if (_row < 0) {
            return false;
        }
    }

    const int destRow = row < 0 ? 0 : row;
    const QList<int> destPath = indexToPath(parent);

    QList<int> srcPath;
    QModelIndex srcParent;
    QHash<QModelIndex, QList<int>>::iterator src_parent_it = movedItems.begin();
    int startRow = 0;
    int endRow = 0;
    int nextMovedRow = 0;

    QList<int> rowsMoved;
    QList<int>::iterator src_row_it;
    QList<int>::iterator rows_moved_end;
    QList<ModelMoveCommand *> moveCommands;

    for (; src_parent_it != movedItems.end(); ++src_parent_it) {
        srcParent = src_parent_it.key();
        srcPath = indexToPath(srcParent);

        rowsMoved = src_parent_it.value();
        std::sort(rowsMoved.begin(), rowsMoved.end());
        src_row_it = rowsMoved.begin();
        rows_moved_end = rowsMoved.end();
        startRow = *src_row_it;
        endRow = startRow;
        ++src_row_it;

        if (src_row_it == rows_moved_end) {
            moveCommands.prepend(getMoveCommand(srcPath, startRow, endRow));
            continue;
        }

        for (; src_row_it != rows_moved_end; ++src_row_it) {
            nextMovedRow = *src_row_it;

            if (nextMovedRow == endRow + 1) {
                ++endRow;
            } else {
                Q_ASSERT(nextMovedRow > endRow + 1);
                moveCommands.prepend(getMoveCommand(srcPath, startRow, endRow));
                startRow = nextMovedRow;
                endRow = nextMovedRow;

                if ((src_row_it + 1) == rows_moved_end) {
                    moveCommands.prepend(getMoveCommand(srcPath, startRow, endRow));
                }
            }
        }
    }

    QPersistentModelIndex destParent = parent;
    QPersistentModelIndex destRowIndex = index(destRow, column, parent);

    ModelMoveCommand *firstCommand = moveCommands.takeFirst();
    firstCommand->setDestAncestors(indexToPath(parent));
    firstCommand->setDestRow(destRow);
    firstCommand->doCommand();

    if (!destRowIndex.isValid()) {
        destRowIndex = index(destRow, column, parent);
    }

    int offset = firstCommand->endRow() - firstCommand->startRow() + 1;
    for (ModelMoveCommand *moveCommand : std::as_const(moveCommands)) {
        moveCommand->setDestAncestors(indexToPath(destParent));
        moveCommand->setDestRow(destRowIndex.row() + offset);
        moveCommand->doCommand();
        offset = moveCommand->endRow() - moveCommand->startRow() + 1;
    }

    return false;
}

ModelMoveCommand *DynamicTreeModel::getMoveCommand(const QList<int> &srcPath, int startRow, int endRow)
{
    ModelMoveCommand *moveCommand = new ModelMoveCommand(this, this);
    moveCommand->setAncestorRowNumbers(srcPath);
    moveCommand->setStartRow(startRow);
    moveCommand->setEndRow(endRow);
    return moveCommand;
}

Qt::ItemFlags DynamicTreeModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return flags | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
    }
    return flags;
}

Qt::DropActions DynamicTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList DynamicTreeModel::mimeTypes() const
{
    QStringList types;
    types << QStringLiteral("application/x-dynamictreemodel-itemlist");
    return types;
}

QMimeData *DynamicTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    QByteArray itemData;
    QModelIndexList::const_iterator it = indexes.begin();
    const QModelIndexList::const_iterator end = indexes.end();
    while (it != end) {
        itemData.append(QByteArray::number(it->internalId()));
        ++it;
        if (it != end) {
            itemData.append('\0');
        }
    }
    data->setData(mimeTypes().at(0), itemData);
    return data;
}

QList<int> DynamicTreeModel::indexToPath(const QModelIndex &_idx) const
{
    QList<int> list;
    QModelIndex idx = _idx;
    while (idx.isValid()) {
        list.prepend(idx.row());
        idx = idx.parent();
    }
    return list;
}

QModelIndexList DynamicTreeModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    if (role != DynamicTreeModelId && role != Qt::DisplayRole) {
        return QAbstractItemModel::match(start, role, value, hits, flags);
    }

    qint64 id = value.toLongLong();
    if (role == Qt::DisplayRole) {
        id = m_items.key(value.toString());
    }

    QHash<qint64, QList<QList<qint64>>>::const_iterator it = m_childItems.constBegin();
    const QHash<qint64, QList<QList<qint64>>>::const_iterator end = m_childItems.constEnd();

    QList<QList<qint64>> items;
    QList<QList<qint64>>::const_iterator itemIt;
    QList<QList<qint64>>::const_iterator itemEnd;
    int foundIndexRow;
    for (; it != end; ++it) {
        items = it.value();
        itemEnd = items.constEnd();
        for (itemIt = items.constBegin(); itemIt != itemEnd; ++itemIt) {
            foundIndexRow = itemIt->indexOf(id);
            if (foundIndexRow != -1) {
                static const int column = 0;
                return QModelIndexList() << createIndex(foundIndexRow, column, reinterpret_cast<void *>(id));
            }
        }
    }
    return QModelIndexList();
}

ModelChangeCommand::ModelChangeCommand(DynamicTreeModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_startRow(-1)
    , m_endRow(-1)
    , m_numCols(1)
{
}

QModelIndex ModelChangeCommand::findIndex(const QList<int> &rows) const
{
    const int col = 0;
    QModelIndex parent = QModelIndex();
    QListIterator<int> i(rows);
    while (i.hasNext()) {
        parent = m_model->index(i.next(), col, parent);
        Q_ASSERT(parent.isValid());
    }
    return parent;
}

ModelInsertCommand::ModelInsertCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
{
}

QList<ModelInsertCommand::Token> ModelInsertCommand::tokenize(const QString &treeString) const
{
    QStringList parts = treeString.split(QStringLiteral("-"));

    QList<Token> tokens;

    const QStringList::const_iterator begin = parts.constBegin();
    const QStringList::const_iterator end = parts.constEnd();

    QStringList::const_iterator it = begin;
    ++it;
    for (; it != end; ++it) {
        Token token;
        if (it->trimmed().isEmpty()) {
            token.type = Token::Branch;
        } else {
            token.type = Token::Leaf;
            token.content = *it;
        }
        tokens.append(token);
    }
    return tokens;
}

void ModelInsertCommand::interpret(const QString &treeString)
{
    m_treeString = treeString;

    const QList<int> depths = getDepths(m_treeString);

    const int size = std::count(depths.begin(), depths.end(), 0);
    Q_ASSERT(size != 0);

    m_endRow = m_startRow + size - 1;
}

QList<int> ModelInsertCommand::getDepths(const QString &treeString) const
{
    int depth = 0;
    QList<int> depths;

#ifdef DUMPTREE
    int id = 1;
#endif

    QList<Token> tokens = tokenize(treeString);
    while (!tokens.isEmpty()) {
        Token token = tokens.takeFirst();

        if (token.type == Token::Branch) {
            ++depth;
            continue;
        }
        Q_ASSERT(token.type == Token::Leaf);

        depths.append(depth);
#ifdef DUMPTREE
        std::cout << "\"";
        for (int i = 0; i <= depth; ++i) {
            std::cout << " -";
        }
        std::cout << " " << id++ << "\"" << std::endl;
#endif
        depth = 0;
    }

    return depths;
}

void ModelInsertCommand::doCommand()
{
    QModelIndex parent = findIndex(m_rowNumbers);

    if (!m_treeString.isEmpty()) {
        const QList<int> depths = getDepths(m_treeString);
        const int size = std::count(depths.begin(), depths.end(), 0);
        Q_ASSERT(size != 0);
        m_endRow = m_startRow + size - 1;
    }
    m_model->beginInsertRows(parent, m_startRow, m_endRow);
    if (!m_treeString.isEmpty()) {
        doInsertTree(parent);
    } else {
        qint64 parentId = parent.internalId();

        for (int row = m_startRow; row <= m_endRow; row++) {
            for (int col = 0; col < m_numCols; col++) {
                if (m_model->m_childItems[parentId].size() <= col) {
                    m_model->m_childItems[parentId].append(QList<qint64>());
                }
                qint64 id = m_model->newId();
                QString name = QString::number(id);

                m_model->m_items.insert(id, name);
                m_model->m_childItems[parentId][col].insert(row, id);
            }
        }
    }
    m_model->endInsertRows();
}

void ModelInsertCommand::doInsertTree(const QModelIndex &fragmentParent)
{
    const QList<int> depths = getDepths(m_treeString);

    qint64 fragmentParentIdentifier = fragmentParent.internalId();

    QList<int>::const_iterator it = depths.constBegin();
    const QList<int>::const_iterator end = depths.constEnd();

    QList<qint64> recentParents;
    recentParents.append(fragmentParentIdentifier);

    qint64 lastId = 0;
    qint64 id;
    QString name;
    int depth = 0;
    int row = m_startRow;
    Q_ASSERT(*it == depth);

    QList<int> rows;
    rows.append(row);

    for (; it != end; ++it) {
        if (*it > depth) {
            Q_ASSERT(*it == depth + 1);
            fragmentParentIdentifier = lastId;
            if (recentParents.size() == *it) {
                recentParents.append(fragmentParentIdentifier);
            } else {
                recentParents[*it] = fragmentParentIdentifier;
            }

            ++depth;

        } else if (*it < depth) {
            fragmentParentIdentifier = recentParents.at(*it);
            depth = (*it);
        }

        if (rows.size() == depth) {
            rows.append(0);
        }

        id = m_model->newId();
        lastId = id;
        for (int column = 0; column < m_numCols; ++column) {
            QList<QList<qint64>> &children = m_model->m_childItems[fragmentParentIdentifier];
            if (children.size() <= column) {
                children.append(QList<qint64>());
            }
            m_model->m_items.insert(id, QString::number(id));
            const int rowForDepth = rows[depth];
            if (rowForDepth >= children[column].size()) {
                children[column].append(id);
            } else {
                children[column].insert(rowForDepth, id);
            }
            if (column != m_numCols - 1) {
                id = m_model->newId();
            }
        }
        rows[depth]++;
    }
}

ModelInsertAndRemoveQueuedCommand::ModelInsertAndRemoveQueuedCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
{
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

void ModelInsertAndRemoveQueuedCommand::queuedBeginInsertRows(const QModelIndex &parent, int start, int end)
{
    m_model->beginInsertRows(parent, start, end);
}

void ModelInsertAndRemoveQueuedCommand::queuedEndInsertRows()
{
    m_model->endInsertRows();
}

void ModelInsertAndRemoveQueuedCommand::queuedBeginRemoveRows(const QModelIndex &parent, int start, int end)
{
    m_model->beginRemoveRows(parent, start, end);
}

void ModelInsertAndRemoveQueuedCommand::queuedEndRemoveRows()
{
    m_model->endRemoveRows();
}

void ModelInsertAndRemoveQueuedCommand::purgeItem(qint64 parent)
{
    QList<QList<qint64>> childItemRows = m_model->m_childItems.value(parent);

    if (!childItemRows.isEmpty()) {
        for (int col = 0; col < m_numCols; col++) {
            const QList<qint64> childItems = childItemRows[col];
            for (qint64 item : childItems) {
                purgeItem(item);
                m_model->m_childItems[parent][col].removeOne(item);
            }
        }
    }
    m_model->m_items.remove(parent);
}

void ModelInsertAndRemoveQueuedCommand::doCommand()
{
    QModelIndex parent = findIndex(m_rowNumbers);

    connect(this, &ModelInsertAndRemoveQueuedCommand::beginInsertRows, this, &ModelInsertAndRemoveQueuedCommand::queuedBeginInsertRows, Qt::QueuedConnection);
    connect(this, &ModelInsertAndRemoveQueuedCommand::endInsertRows, this, &ModelInsertAndRemoveQueuedCommand::queuedEndInsertRows, Qt::QueuedConnection);
    connect(this, &ModelInsertAndRemoveQueuedCommand::beginRemoveRows, this, &ModelInsertAndRemoveQueuedCommand::queuedBeginRemoveRows, Qt::QueuedConnection);
    connect(this, &ModelInsertAndRemoveQueuedCommand::endRemoveRows, this, &ModelInsertAndRemoveQueuedCommand::queuedEndRemoveRows, Qt::QueuedConnection);

    Q_EMIT beginInsertRows(parent, m_startRow, m_endRow);
    //   m_model->beginInsertRows(parent, m_startRow, m_endRow);
    qint64 parentId = parent.internalId();
    for (int row = m_startRow; row <= m_endRow; row++) {
        for (int col = 0; col < m_numCols; col++) {
            if (m_model->m_childItems[parentId].size() <= col) {
                m_model->m_childItems[parentId].append(QList<qint64>());
            }
            qint64 id = m_model->newId();
            QString name = QString::number(id);

            m_model->m_items.insert(id, name);
            m_model->m_childItems[parentId][col].insert(row, id);
        }
    }

    Q_EMIT endInsertRows();
    //   m_model->endInsertRows();

    Q_EMIT beginRemoveRows(parent, m_startRow, m_endRow);
    //   m_model->beginRemoveRows(parent, m_startRow, m_endRow);
    for (int col = 0; col < m_numCols; col++) {
        QList<qint64> childItems = m_model->m_childItems.value(parentId).value(col);
        for (int row = m_startRow; row <= m_endRow; row++) {
            qint64 item = childItems[row];
            purgeItem(item);
            m_model->m_childItems[parentId][col].removeOne(item);
        }
    }
    Q_EMIT endRemoveRows();
    //   m_model->endRemoveRows();
}

ModelRemoveCommand::ModelRemoveCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
{
}

void ModelRemoveCommand::doCommand()
{
    QModelIndex parent = findIndex(m_rowNumbers);
    m_model->beginRemoveRows(parent, m_startRow, m_endRow);
    qint64 parentId = parent.internalId();
    for (int col = 0; col < m_numCols; col++) {
        QList<qint64> childItems = m_model->m_childItems.value(parentId).value(col);
        for (int row = m_startRow; row <= m_endRow; row++) {
            qint64 item = childItems[row];
            purgeItem(item);
            m_model->m_childItems[parentId][col].removeOne(item);
        }
    }
    m_model->endRemoveRows();
}

void ModelRemoveCommand::purgeItem(qint64 parent)
{
    const QList<QList<qint64>> childItemRows = m_model->m_childItems.value(parent);

    if (!childItemRows.isEmpty()) {
        for (int col = 0; col < m_numCols; col++) {
            const QList<qint64> childItems = childItemRows[col];
            for (qint64 item : childItems) {
                purgeItem(item);
                m_model->m_childItems[parent][col].removeOne(item);
            }
        }
    }
    m_model->m_items.remove(parent);
}

ModelDataChangeCommand::ModelDataChangeCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
    , m_startColumn(0)
{
}

void ModelDataChangeCommand::doCommand()
{
    QModelIndex parent = findIndex(m_rowNumbers);
    QModelIndex topLeft = m_model->index(m_startRow, m_startColumn, parent);
    QModelIndex bottomRight = m_model->index(m_endRow, m_numCols - 1, parent);

    QList<QList<qint64>> childItems = m_model->m_childItems[parent.internalId()];

    for (int col = m_startColumn; col < m_startColumn + m_numCols; col++) {
        for (int row = m_startRow; row <= m_endRow; row++) {
            QString name = QString::number(m_model->newId());
            m_model->m_items[childItems[col][row]] = name;
        }
    }
    m_model->dataChanged(topLeft, bottomRight);
}

ModelMoveCommand::ModelMoveCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
{
}

bool ModelMoveCommand::emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
    return m_model->beginMoveRows(srcParent, srcStart, srcEnd, destParent, destRow);
}

void ModelMoveCommand::doCommand()
{
    QModelIndex srcParent = findIndex(m_rowNumbers);
    QModelIndex destParent = findIndex(m_destRowNumbers);

    if (!emitPreSignal(srcParent, m_startRow, m_endRow, destParent, m_destRow)) {
        return;
    }

    for (int column = 0; column < m_numCols; ++column) {
        const QList<qint64> l = m_model->m_childItems.value(srcParent.internalId())[column].mid(m_startRow, m_endRow - m_startRow + 1);

        for (int i = m_startRow; i <= m_endRow; i++) {
            m_model->m_childItems[srcParent.internalId()][column].removeAt(m_startRow);
        }
        int d;
        if (m_destRow < m_startRow) {
            d = m_destRow;
        } else {
            if (srcParent == destParent) {
                d = m_destRow - (m_endRow - m_startRow + 1);
            } else {
                d = m_destRow - (m_endRow - m_startRow);
            }
        }

        for (const qint64 id : l) {
            if (!m_model->m_childItems.contains(destParent.internalId())) {
                m_model->m_childItems[destParent.internalId()].append(QList<qint64>());
            }

            m_model->m_childItems[destParent.internalId()][column].insert(d++, id);
        }
    }

    emitPostSignal();
}

void ModelMoveCommand::emitPostSignal()
{
    m_model->endMoveRows();
}

ModelMoveLayoutChangeCommand::ModelMoveLayoutChangeCommand(DynamicTreeModel *model, QObject *parent)
    : ModelMoveCommand(model, parent)
{
}

ModelMoveLayoutChangeCommand::~ModelMoveLayoutChangeCommand()
{
}

bool ModelMoveLayoutChangeCommand::emitPreSignal(const QModelIndex &srcParent, int srcStart, int srcEnd, const QModelIndex &destParent, int destRow)
{
    m_model->layoutAboutToBeChanged();

    const int column = 0;

    for (int row = srcStart; row <= srcEnd; ++row) {
        m_beforeMoveList << m_model->index(row, column, srcParent);
    }

    if (srcParent != destParent) {
        for (int row = srcEnd + 1; row < m_model->rowCount(srcParent); ++row) {
            m_beforeMoveList << m_model->index(row, column, srcParent);
        }
        for (int row = destRow; row < m_model->rowCount(destParent); ++row) {
            m_beforeMoveList << m_model->index(row, column, destParent);
        }
    } else {
        if (destRow < srcStart) {
            for (int row = destRow; row < srcStart; ++row) {
                m_beforeMoveList << m_model->index(row, column, srcParent);
            }
        } else {
            for (int row = srcStart + (srcEnd - srcStart + 1); row < destRow; ++row) {
                m_beforeMoveList << m_model->index(row, column, srcParent);
            }
        }
    }
    // We assume that the move was legal here.
    return true;
}

void ModelMoveLayoutChangeCommand::emitPostSignal()
{
    int srcStart = m_startRow;
    int srcEnd = m_endRow;
    int destRow = m_destRow;

    // Moving indexes may affect the m_rowNumbers and m_destRowNumbers.
    // Instead of adjusting them programmatically, the test writer must specify them if they change.

    const QList<int> sourceRowNumbers = m_endOfMoveSourceAncestors.isEmpty() ? m_rowNumbers : m_endOfMoveSourceAncestors;
    QModelIndex srcParent = findIndex(sourceRowNumbers);

    const QList<int> destRowNumbers = m_endOfMoveDestAncestors.isEmpty() ? m_destRowNumbers : m_endOfMoveDestAncestors;
    QModelIndex destParent = findIndex(destRowNumbers);

    const int column = 0;

    QModelIndexList afterMoveList;

    if (srcParent != destParent) {
        for (int row = destRow; row <= (destRow + (srcEnd - srcStart)); ++row) {
            afterMoveList << m_model->index(row, column, destParent);
        }
        for (int row = srcStart; row < m_model->rowCount(srcParent); ++row) {
            afterMoveList << m_model->index(row, column, srcParent);
        }
        for (int row = destRow + (srcEnd - srcStart + 1); row < m_model->rowCount(destParent); ++row) {
            afterMoveList << m_model->index(row, column, destParent);
        }
    } else {
        if (destRow < srcStart) {
            for (int row = srcStart; row <= srcEnd; ++row) {
                afterMoveList << m_model->index(destRow + (srcStart - row), column, destParent);
            }
        } else {
            for (int row = srcStart; row <= srcEnd; ++row) {
                afterMoveList << m_model->index(destRow + (srcStart - row - 1), column, destParent);
            }
        }
        if (destRow < srcStart) {
            for (int row = destRow + 1; row <= srcStart; ++row) {
                afterMoveList << m_model->index(row, column, srcParent);
            }
        } else {
            for (int row = srcStart + (srcEnd - srcStart + 1); row < (srcStart + (destRow - srcEnd)); ++row) {
                afterMoveList << m_model->index(row - (srcEnd - srcStart + 1), column, srcParent);
            }
        }
    }

    m_model->changePersistentIndexList(m_beforeMoveList, afterMoveList);
    m_beforeMoveList.clear();
    m_model->layoutChanged();
}

ModelResetCommand::ModelResetCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
{
}

ModelResetCommand::~ModelResetCommand()
{
}

void ModelResetCommand::setInitialTree(const QString &treeString)
{
    m_treeString = treeString;
}

void ModelResetCommand::doCommand()
{
    m_model->beginResetModel();
    bool blocked = m_model->blockSignals(true);
    m_model->clear();
    if (!m_treeString.isEmpty()) {
        ModelInsertCommand ins(m_model);
        ins.setStartRow(0);
        ins.interpret(m_treeString);
        ins.doCommand();
    }
    m_model->blockSignals(blocked);
    m_model->endResetModel();
}

ModelLayoutChangeCommand::ModelLayoutChangeCommand(DynamicTreeModel *model, QObject *parent)
    : ModelChangeCommand(model, parent)
{
}

ModelLayoutChangeCommand::~ModelLayoutChangeCommand()
{
}

void ModelLayoutChangeCommand::setInitialTree(const QString &treeString)
{
    m_treeString = treeString;
}

void ModelLayoutChangeCommand::setPersistentChanges(const QList<ModelLayoutChangeCommand::PersistentChange> &changes)
{
    m_changes = changes;
}

void ModelLayoutChangeCommand::doCommand()
{
    m_model->layoutAboutToBeChanged();
    QModelIndexList oldList;

    for (const PersistentChange &change : std::as_const(m_changes)) {
        const IndexFinder oldFinder(m_model, change.oldPath);
        oldList << oldFinder.getIndex();
    }

    bool blocked = m_model->blockSignals(true);
    m_model->clear();
    if (!m_treeString.isEmpty()) {
        ModelInsertCommand *ins = new ModelInsertCommand(m_model);
        ins->setStartRow(0);
        ins->interpret(m_treeString);
        ins->doCommand();
    }

    QModelIndexList newList;
    for (const PersistentChange &change : std::as_const(m_changes)) {
        const IndexFinder newFinder(m_model, change.newPath);
        newList << newFinder.getIndex();
    }
    m_model->changePersistentIndexList(oldList, newList);

    m_model->blockSignals(blocked);
    m_model->layoutChanged();
}

#include "moc_dynamictreemodel.cpp"

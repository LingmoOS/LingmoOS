/*
    This file is part of the test suite of the Qt Toolkit.

    SPDX-FileCopyrightText: 2013 Digia Plc and/or its subsidiary(-ies) <https://www.qt.io/terms-conditions/>
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#include "modeltest.h"

#include <QTest>
#include <qabstracteventdispatcher.h>

/*!
    Connect to all of the models signals.  Whenever anything happens recheck everything.
*/
ModelTest::ModelTest(QAbstractItemModel *_model, Mode testType, QObject *parent)
    : QObject(parent)
    , model(_model)
    , fetchingMore(false)
    , pedantic(testType == Pedantic)
{
    init();
    if (pedantic) {
        refreshStatus();
        // This is almost certainly not needed.
        //         connect(QAbstractEventDispatcher::instance(), &QAbstractEventDispatcher::awake, this, &ModelTest::ensureSteady);
    }
}

ModelTest::ModelTest(QAbstractItemModel *_model, QObject *parent)
    : QObject(parent)
    , model(_model)
    , fetchingMore(false)
    , pedantic(false)
{
    init();
}

void ModelTest::init()
{
    if (!model) {
        qFatal("%s: model must not be null", Q_FUNC_INFO);
    }

    connect(model, &QAbstractItemModel::columnsAboutToBeInserted, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::columnsAboutToBeRemoved, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::columnsInserted, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::columnsRemoved, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::dataChanged, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::headerDataChanged, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::layoutChanged, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::modelReset, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::modelAboutToBeReset, this, &ModelTest::modelAboutToBeReset);
    connect(model, &QAbstractItemModel::modelReset, this, &ModelTest::modelReset);
    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::rowsInserted, this, &ModelTest::runAllTests);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &ModelTest::runAllTests);

    // Special checks for changes
    connect(model, &QAbstractItemModel::layoutAboutToBeChanged, this, &ModelTest::layoutAboutToBeChanged);
    connect(model, &QAbstractItemModel::layoutChanged, this, &ModelTest::layoutChanged);

    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &ModelTest::rowsAboutToBeInserted);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ModelTest::rowsAboutToBeRemoved);
    connect(model, &QAbstractItemModel::rowsInserted, this, &ModelTest::rowsInserted);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &ModelTest::rowsRemoved);
    connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &ModelTest::rowsAboutToBeMoved);
    connect(model, &QAbstractItemModel::rowsMoved, this, &ModelTest::rowsMoved);
    connect(model, &QAbstractItemModel::dataChanged, this, &ModelTest::dataChanged);
    connect(model, &QAbstractItemModel::headerDataChanged, this, &ModelTest::headerDataChanged);

    runAllTests();
}

void ModelTest::runAllTests()
{
    if (fetchingMore) {
        return;
    }
    nonDestructiveBasicTest();
    rowCount();
    columnCount();
    hasIndex();
    index();
    parent();
    data();
}

/*!
    nonDestructiveBasicTest tries to call a number of the basic functions (not all)
    to make sure the model doesn't outright segfault, testing the functions that makes sense.
*/
void ModelTest::nonDestructiveBasicTest()
{
    QVERIFY(model->buddy(QModelIndex()) == QModelIndex());
    model->canFetchMore(QModelIndex());
    QVERIFY(model->columnCount(QModelIndex()) >= 0);
    QVERIFY(model->data(QModelIndex()) == QVariant());
    fetchingMore = true;
    model->fetchMore(QModelIndex());
    fetchingMore = false;
    Qt::ItemFlags flags = model->flags(QModelIndex());
    QVERIFY(flags == Qt::ItemIsDropEnabled || flags == 0);
    model->hasChildren(QModelIndex());
    model->hasIndex(0, 0);
    model->headerData(0, Qt::Horizontal);
    model->index(0, 0);
    model->itemData(QModelIndex());
    QVariant cache;
    model->match(QModelIndex(), -1, cache);
    model->mimeTypes();
    QVERIFY(model->parent(QModelIndex()) == QModelIndex());
    QVERIFY(model->rowCount() >= 0);
    QVariant variant;
    model->setData(QModelIndex(), variant, -1);
    model->setHeaderData(-1, Qt::Horizontal, QVariant());
    model->setHeaderData(999999, Qt::Horizontal, QVariant());
    QMap<int, QVariant> roles;
    model->sibling(0, 0, QModelIndex());
    model->span(QModelIndex());
    model->supportedDropActions();
}

/*!
    Tests model's implementation of QAbstractItemModel::rowCount() and hasChildren()

    Models that are dynamically populated are not as fully tested here.
 */
void ModelTest::rowCount()
{
    //     qDebug() << "rc";
    // check top row
    QModelIndex topIndex = model->index(0, 0, QModelIndex());
    int rows = model->rowCount(topIndex);
    QVERIFY(rows >= 0);
    QCOMPARE(model->hasChildren(topIndex), rows > 0);

    QModelIndex secondLevelIndex = model->index(0, 0, topIndex);
    if (secondLevelIndex.isValid()) { // not the top level
        // check a row count where parent is valid
        rows = model->rowCount(secondLevelIndex);
        QVERIFY(rows >= 0);
        if (rows > 0) {
            QVERIFY(model->hasChildren(secondLevelIndex));
        }
    }

    // The models rowCount() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::columnCount() and hasChildren()
 */
void ModelTest::columnCount()
{
    // check top row
    QModelIndex topIndex = model->index(0, 0, QModelIndex());
    QVERIFY(model->columnCount(topIndex) >= 0);

    // check a column count where parent is valid
    QModelIndex childIndex = model->index(0, 0, topIndex);
    if (childIndex.isValid()) {
        QVERIFY(model->columnCount(childIndex) >= 0);
    }

    // columnCount() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::hasIndex()
 */
void ModelTest::hasIndex()
{
    //     qDebug() << "hi";
    // Make sure that invalid values returns an invalid index
    QVERIFY(!model->hasIndex(-2, -2));
    QVERIFY(!model->hasIndex(-2, 0));
    QVERIFY(!model->hasIndex(0, -2));

    int rows = model->rowCount();
    int columns = model->columnCount();

    // check out of bounds
    QVERIFY(!model->hasIndex(rows, columns));
    QVERIFY(!model->hasIndex(rows + 1, columns + 1));

    QCOMPARE(model->hasIndex(0, 0), rows > 0);

    // hasIndex() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::index()
 */
void ModelTest::index()
{
    //     qDebug() << "i";
    // Make sure that invalid values returns an invalid index
    QVERIFY(model->index(-2, -2) == QModelIndex());
    QVERIFY(model->index(-2, 0) == QModelIndex());
    QVERIFY(model->index(0, -2) == QModelIndex());

    int rows = model->rowCount();
    int columns = model->columnCount();

    if (rows == 0) {
        return;
    }

    // Catch off by one errors
    QVERIFY(model->index(rows, columns) == QModelIndex());
    QVERIFY(model->index(0, 0).isValid());

    // Make sure that the same index is *always* returned
    QModelIndex a = model->index(0, 0);
    QModelIndex b = model->index(0, 0);
    QVERIFY(a == b);

    // index() is tested more extensively in checkChildren(),
    // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::parent()
 */
void ModelTest::parent()
{
    //     qDebug() << "p";
    // Make sure the model won't crash and will return an invalid QModelIndex
    // when asked for the parent of an invalid index.
    QVERIFY(model->parent(QModelIndex()) == QModelIndex());

    if (model->rowCount() == 0) {
        return;
    }

    // Column 0                | Column 1    |
    // QModelIndex()           |             |
    //    \- topIndex          | topIndex1   |
    //         \- childIndex   | childIndex1 |

    // Common error test #1, make sure that a top level index has a parent
    // that is a invalid QModelIndex.
    QModelIndex topIndex = model->index(0, 0, QModelIndex());
    QVERIFY(model->parent(topIndex) == QModelIndex());

    // Common error test #2, make sure that a second level index has a parent
    // that is the first level index.
    if (model->rowCount(topIndex) > 0) {
        QModelIndex childIndex = model->index(0, 0, topIndex);
        if (model->parent(childIndex) != topIndex) {
            qDebug() << model->parent(childIndex) << topIndex << topIndex.data();
        }
        QVERIFY(model->parent(childIndex) == topIndex);
    }

    // Common error test #3, the second column should NOT have the same children
    // as the first column in a row.
    // Usually the second column shouldn't have children.
    QModelIndex topIndex1 = model->index(0, 1, QModelIndex());
    if (model->rowCount(topIndex1) > 0) {
        QModelIndex childIndex = model->index(0, 0, topIndex);
        QModelIndex childIndex1 = model->index(0, 0, topIndex1);
        QVERIFY(childIndex != childIndex1);
    }

    // Full test, walk n levels deep through the model making sure that all
    // parent's children correctly specify their parent.
    checkChildren(QModelIndex());
}

/*!
    Called from the parent() test.

    A model that returns an index of parent X should also return X when asking
    for the parent of the index.

    This recursive function does pretty extensive testing on the whole model in an
    effort to catch edge cases.

    This function assumes that rowCount(), columnCount() and index() already work.
    If they have a bug it will point it out, but the above tests should have already
    found the basic bugs because it is easier to figure out the problem in
    those tests then this one.
 */
void ModelTest::checkChildren(const QModelIndex &parent, int currentDepth)
{
    // First just try walking back up the tree.
    QModelIndex p = parent;
    while (p.isValid()) {
        p = p.parent();
    }

    // For models that are dynamically populated
    if (model->canFetchMore(parent)) {
        fetchingMore = true;
        model->fetchMore(parent);
        fetchingMore = false;
    }

    int rows = model->rowCount(parent);
    int columns = model->columnCount(parent);

    // Some further testing against rows(), columns(), and hasChildren()
    QVERIFY(rows >= 0);
    QVERIFY(columns >= 0);
    QCOMPARE(model->hasChildren(parent), rows > 0);

    qDebug() << "parent:" << model->data(parent).toString() << "rows:" << rows << "columns:" << columns << "parent column:" << parent.column();

    const QModelIndex topLeftChild = model->index(0, 0, parent);

    QVERIFY(!model->hasIndex(rows, 0, parent));
    QVERIFY(!model->index(rows, 0, parent).isValid());
    for (int r = 0; r < rows; ++r) {
        if (model->canFetchMore(parent)) {
            fetchingMore = true;
            model->fetchMore(parent);
            fetchingMore = false;
        }
        QVERIFY(!model->hasIndex(r, columns, parent));
        QVERIFY(!model->index(r, columns, parent).isValid());
        for (int c = 0; c < columns; ++c) {
            QVERIFY(model->hasIndex(r, c, parent));
            QModelIndex index = model->index(r, c, parent);
            // rowCount() and columnCount() said that it existed...
            QVERIFY(index.isValid());

            // index() should always return the same index when called twice in a row
            QModelIndex modifiedIndex = model->index(r, c, parent);
            QVERIFY(index == modifiedIndex);

            // Make sure we get the same index if we request it twice in a row
            QModelIndex a = model->index(r, c, parent);
            QModelIndex b = model->index(r, c, parent);
            QVERIFY(a == b);

            {
                const QModelIndex sibling = model->sibling(r, c, topLeftChild);
                QVERIFY(index == sibling);
            }
            {
                const QModelIndex sibling = topLeftChild.sibling(r, c);
                QVERIFY(index == sibling);
            }

            // Some basic checking on the index that is returned
            QVERIFY(index.model() == model);
            QCOMPARE(index.row(), r);
            QCOMPARE(index.column(), c);
            // While you can technically return a QVariant usually this is a sign
            // of a bug in data().  Disable if this really is ok in your model.
            if (!model->data(index, Qt::DisplayRole).isValid()) {
                qDebug() << index << index.data() << index.parent();
            }
            QVERIFY(model->data(index, Qt::DisplayRole).isValid());

            // If the next test fails here is some somewhat useful debug you play with.

            /*
            if (model->parent(index) != parent) {
                qDebug() << r << c << currentDepth << model->data(index).toString()
                         << model->data(parent).toString();
                qDebug() << index << parent << model->parent(index);
//                 And a view that you can even use to show the model.
//                 QTreeView view;
//                 view.setModel(model);
//                 view.show();
            }*/

            // Check that we can get back our real parent.
            QCOMPARE(model->parent(index), parent);

            // recursively go down the children
            if (model->hasChildren(index) && currentDepth < 10) {
                // qDebug() << r << c << "has children" << model->rowCount(index);
                checkChildren(index, ++currentDepth);
            } /* else { if (currentDepth >= 10) qDebug() << "checked 10 deep"; };*/

            // make sure that after testing the children that the index doesn't change.
            QModelIndex newerIndex = model->index(r, c, parent);
            QVERIFY(index == newerIndex);
        }
    }
}

/*!
    Tests model's implementation of QAbstractItemModel::data()
 */
void ModelTest::data()
{
    // Invalid index should return an invalid qvariant
    QVERIFY(!model->data(QModelIndex()).isValid());

    if (model->rowCount() == 0) {
        return;
    }

    // A valid index should have a valid QVariant data
    QVERIFY(model->index(0, 0).isValid());

    // shouldn't be able to set data on an invalid index
    QVERIFY(!model->setData(QModelIndex(), QLatin1String("foo"), Qt::DisplayRole));

    // General Purpose roles that should return a QString
    QVariant variant = model->data(model->index(0, 0), Qt::ToolTipRole);
    if (variant.isValid()) {
        QVERIFY(variant.canConvert<QString>());
    }
    variant = model->data(model->index(0, 0), Qt::StatusTipRole);
    if (variant.isValid()) {
        QVERIFY(variant.canConvert<QString>());
    }
    variant = model->data(model->index(0, 0), Qt::WhatsThisRole);
    if (variant.isValid()) {
        QVERIFY(variant.canConvert<QString>());
    }

    // General Purpose roles that should return a QSize
    variant = model->data(model->index(0, 0), Qt::SizeHintRole);
    if (variant.isValid()) {
        QVERIFY(variant.canConvert<QSize>());
    }

    // General Purpose roles that should return a QFont
    QVariant fontVariant = model->data(model->index(0, 0), Qt::FontRole);
    if (fontVariant.isValid()) {
        QVERIFY(fontVariant.canConvert<QFont>());
    }

    // Check that the alignment is one we know about
    QVariant textAlignmentVariant = model->data(model->index(0, 0), Qt::TextAlignmentRole);
    if (textAlignmentVariant.isValid()) {
        const auto alignment = textAlignmentVariant.toUInt();
        QCOMPARE(alignment, (alignment & (Qt::AlignHorizontal_Mask | Qt::AlignVertical_Mask)));
    }

    // General Purpose roles that should return a QColor
    QVariant colorVariant = model->data(model->index(0, 0), Qt::BackgroundRole);
    if (colorVariant.isValid()) {
        QVERIFY(colorVariant.canConvert<QColor>());
    }

    colorVariant = model->data(model->index(0, 0), Qt::ForegroundRole);
    if (colorVariant.isValid()) {
        QVERIFY(colorVariant.canConvert<QColor>());
    }

    // Check that the "check state" is one we know about.
    QVariant checkStateVariant = model->data(model->index(0, 0), Qt::CheckStateRole);
    if (checkStateVariant.isValid()) {
        int state = checkStateVariant.toInt();
        QVERIFY(state == Qt::Unchecked || state == Qt::PartiallyChecked || state == Qt::Checked);
    }
}

/*!
    Store what is about to be inserted to make sure it actually happens

    \sa rowsInserted()
 */
void ModelTest::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    //     Q_UNUSED(end);
    qDebug() << "rowsAboutToBeInserted"
             << "start=" << start << "end=" << end << "parent=" << model->data(parent).toString()
             << "current count of parent=" << model->rowCount(parent); // << "display of last=" << model->data( model->index(start-1, 0, parent) );
    //     qDebug() << model->index(start-1, 0, parent) << model->data( model->index(start-1, 0, parent) );
    Changing c;
    c.parent = parent;
    c.oldSize = model->rowCount(parent);
    c.last = model->data(model->index(start - 1, 0, parent));
    c.next = model->data(model->index(start, 0, parent));
    insert.push(c);
    if (pedantic) {
        ensureConsistent();
        status.type = Status::InsertingRows;
    }
}

/*!
    Confirm that what was said was going to happen actually did

    \sa rowsAboutToBeInserted()
 */
void ModelTest::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Changing c = insert.pop();
    QVERIFY(c.parent == parent);
    qDebug() << "rowsInserted"
             << "start=" << start << "end=" << end << "oldsize=" << c.oldSize << "parent=" << model->data(parent).toString()
             << "current rowcount of parent=" << model->rowCount(parent);

    for (int ii = start; ii <= end; ii++) {
        qDebug() << "itemWasInserted:" << ii << model->data(model->index(ii, 0, parent));
    }

    QVERIFY(c.oldSize + (end - start + 1) == model->rowCount(parent));
    QVERIFY(c.last == model->data(model->index(start - 1, 0, c.parent)));

    /*
    if (c.next != model->data(model->index(end + 1, 0, c.parent))) {
        qDebug() << start << end;
        for (int i=0; i < model->rowCount(); ++i)
            qDebug() << model->index(i, 0).data().toString();
        qDebug() << c.next << model->data(model->index(end + 1, 0, c.parent));
    }
    */

    QVERIFY(c.next == model->data(model->index(end + 1, 0, c.parent)));

    if (pedantic) {
        QVERIFY(status.type == Status::InsertingRows);
        refreshStatus();
    }
}

void ModelTest::modelAboutToBeReset()
{
    qDebug() << "@@@@@@@@@@@"
             << "modelAboutToBeReset";

    if (pedantic) {
        ensureConsistent();
        status.type = Status::Resetting;
    }
}

void ModelTest::modelReset()
{
    qDebug() << "@@@@@@@@@@@"
             << "modelReset";
    if (pedantic) {
        Q_ASSERT(status.type == Status::Resetting);
        refreshStatus();
    }
}

void ModelTest::layoutAboutToBeChanged()
{
    qDebug() << "@@@@@@@@@@@"
             << "layoutAboutToBeChanged";

    if (pedantic) {
        ensureConsistent();
        status.type = Status::ChangingLayout;
    }
    for (int i = 0; i < qBound(0, model->rowCount(), 100); ++i) {
        changing.append(QPersistentModelIndex(model->index(i, 0)));
    }
}

void ModelTest::layoutChanged()
{
    qDebug() << "@@@@@@@@@@@"
             << "layoutAboutToBeChanged";
    for (int i = 0; i < changing.count(); ++i) {
        QPersistentModelIndex p = changing[i];
        QVERIFY(p == model->index(p.row(), p.column(), p.parent()));
    }
    changing.clear();

    if (pedantic) {
        QVERIFY(status.type == Status::ChangingLayout);
        refreshStatus();
    }
}

void ModelTest::rowsAboutToBeMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destinationRow)
{
    qDebug() << "rowsAboutToBeMoved" << srcParent << start << end << destParent << destinationRow;

    for (int row = start, dr = destinationRow; row <= end; ++row, ++dr) {
        qDebug() << "row" << model->index(row, 0, srcParent).data() << "in " << srcParent << "will be moved to " << destParent << dr;
    }

    Changing cs;
    cs.parent = srcParent;
    cs.oldSize = model->rowCount(srcParent);
    cs.last = model->data(model->index(start - 1, 0, srcParent));
    cs.next = model->data(model->index(end + 1, 0, srcParent));
    remove.push(cs);
    Changing cd;
    cd.parent = destParent;
    cd.oldSize = model->rowCount(destParent);
    cd.last = model->data(model->index(destinationRow - 1, 0, destParent));
    cd.next = model->data(model->index(destinationRow, 0, destParent));
    insert.push(cd);
}

void ModelTest::rowsMoved(const QModelIndex &srcParent, int start, int end, const QModelIndex &destParent, int destinationRow)
{
    qDebug() << "rowsMoved" << srcParent << start << end << destParent << destinationRow;

    Changing cd = insert.pop();
    QVERIFY(cd.parent == destParent);
    if (srcParent == destParent) {
        QVERIFY(cd.oldSize == model->rowCount(destParent));

        // TODO: Find out what I can assert here about last and next.
        //     QVERIFY ( cd.last == model->data ( model->index ( destinationRow - 1, 0, cd.parent ) ) );
        //     QVERIFY ( cd.next == model->data ( model->index ( destinationRow + (end - start + 1), 0, cd.parent ) ) );

    } else {
        qDebug() << cd.oldSize << end << start << model->rowCount(destParent) << destParent.data() << "#########";
        QVERIFY(cd.oldSize + (end - start + 1) == model->rowCount(destParent));

        QVERIFY(cd.last == model->data(model->index(destinationRow - 1, 0, cd.parent)));
        QVERIFY(cd.next == model->data(model->index(destinationRow + (end - start + 1), 0, cd.parent)));
    }
    Changing cs = remove.pop();

    QVERIFY(cs.parent == srcParent);
    if (srcParent == destParent) {
        QVERIFY(cs.oldSize == model->rowCount(srcParent));
    } else {
        QVERIFY(cs.oldSize - (end - start + 1) == model->rowCount(srcParent));

        QVERIFY(cs.last == model->data(model->index(start - 1, 0, srcParent)));
        //     qDebug() << cs.next << model->data ( model->index ( start, 0, srcParent ) );
        QVERIFY(cs.next == model->data(model->index(start, 0, srcParent)));
    }
}

/*!
    Store what is about to be inserted to make sure it actually happens

    \sa rowsRemoved()
 */
void ModelTest::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    qDebug() << "ratbr" << parent << start << end;
    for (int ii = start; ii <= end; ii++) {
        qDebug() << "itemwillbe removed:" << model->data(model->index(ii, 0, parent));
    }

    if (pedantic) {
        ensureConsistent();
        status.type = Status::RemovingRows;
    }

    Changing c;
    c.parent = parent;
    c.oldSize = model->rowCount(parent);
    c.last = model->data(model->index(start - 1, 0, parent));
    c.next = model->data(model->index(end + 1, 0, parent));
    remove.push(c);
}

/*!
    Confirm that what was said was going to happen actually did

    \sa rowsAboutToBeRemoved()
 */
void ModelTest::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    qDebug() << "rr" << parent << start << end;
    Changing c = remove.pop();
    QVERIFY(c.parent == parent);
    qDebug() << (c.oldSize - (end - start + 1)) << model->rowCount(parent);
    QVERIFY(c.oldSize - (end - start + 1) == model->rowCount(parent));
    QVERIFY(c.last == model->data(model->index(start - 1, 0, c.parent)));
    QVERIFY(c.next == model->data(model->index(start, 0, c.parent)));

    if (pedantic) {
        Q_ASSERT(status.type == Status::RemovingRows);
        refreshStatus();
    }
}

void ModelTest::refreshStatus()
{
    status.type = Status::Idle;
    status.nonPersistent.clear();
    status.persistent.clear();

    persistStatus(QModelIndex());
}

void ModelTest::persistStatus(const QModelIndex &index)
{
    const int rowCount = model->rowCount(index);
    for (int row = 0; row < rowCount; ++row) {
        // TODO: Test multi columns
        static const int column = 0;
        QPersistentModelIndex idx = model->index(row, column, index);
        status.persistent.append(idx);
        status.nonPersistent.append(idx);
        persistStatus(idx);
    }
}

void ModelTest::ensureSteady()
{
    Q_ASSERT(insert.isEmpty());
    Q_ASSERT(remove.isEmpty());
    Q_ASSERT(changing.isEmpty());
    ensureConsistent();
}

void ModelTest::ensureConsistent()
{
    Q_ASSERT(status.type == Status::Idle);

    Q_ASSERT(status.nonPersistent.size() == status.persistent.size());
    for (int i = 0; i < status.nonPersistent.size(); ++i) {
        Q_ASSERT(status.nonPersistent.at(i) == status.persistent.at(i));
    }
}

void ModelTest::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QVERIFY(topLeft.isValid());
    QVERIFY(bottomRight.isValid());
    QModelIndex commonParent = bottomRight.parent();
    QVERIFY(topLeft.parent() == commonParent);
    QVERIFY(topLeft.row() <= bottomRight.row());
    QVERIFY(topLeft.column() <= bottomRight.column());
    int rowCount = model->rowCount(commonParent);
    int columnCount = model->columnCount(commonParent);
    QVERIFY(bottomRight.row() < rowCount);
    QVERIFY(bottomRight.column() < columnCount);
}

void ModelTest::headerDataChanged(Qt::Orientation orientation, int start, int end)
{
    QVERIFY(start >= 0);
    QVERIFY(end >= 0);
    QVERIFY(start <= end);
    int itemCount = orientation == Qt::Vertical ? model->rowCount() : model->columnCount();
    QVERIFY(start < itemCount);
    QVERIFY(end < itemCount);
}

#include "moc_modeltest.cpp"

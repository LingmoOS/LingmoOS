/*
    This file is part of the test suite of the Qt Toolkit.

    SPDX-FileCopyrightText: 2013 Digia Plc and/or its subsidiary(-ies) <https://www.qt.io/terms-conditions/>
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#ifndef MODELTEST_H
#define MODELTEST_H

#include <QAbstractItemModel>
#include <QObject>
#include <QStack>

#include "proxymodeltestsuite_export.h"

class PROXYMODELTESTSUITE_EXPORT ModelTest : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        Normal,
        Pedantic,
    };

    ModelTest(QAbstractItemModel *model, QObject *parent = nullptr);
    ModelTest(QAbstractItemModel *model, Mode testType, QObject *parent = nullptr);

private Q_SLOTS:
    void nonDestructiveBasicTest();
    void rowCount();
    void columnCount();
    void hasIndex();
    void index();
    void parent();
    void data();

protected Q_SLOTS:
    void runAllTests();
    void layoutAboutToBeChanged();
    void layoutChanged();
    void modelAboutToBeReset();
    void modelReset();
    void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int);
    void rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void headerDataChanged(Qt::Orientation orientation, int start, int end);

    void ensureConsistent();
    void ensureSteady();

private:
    void checkChildren(const QModelIndex &parent, int currentDepth = 0);
    void refreshStatus();
    void persistStatus(const QModelIndex &index);
    void init();

    QAbstractItemModel *const model;

    struct Status {
        enum Type {
            Idle,
            InsertingRows,
            RemovingRows,
            MovingRows,
            ChangingLayout,
            Resetting,
        };

        Type type;

        QList<QPersistentModelIndex> persistent;
        QList<QModelIndex> nonPersistent;
    } status;

    struct Changing {
        QModelIndex parent;
        int oldSize;
        QVariant last;
        QVariant next;
    };
    QStack<Changing> insert;
    QStack<Changing> remove;

    bool fetchingMore;
    const bool pedantic;

    QList<QPersistentModelIndex> changing;
};

#endif

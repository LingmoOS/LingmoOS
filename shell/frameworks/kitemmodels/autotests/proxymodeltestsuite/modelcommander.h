/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODEL_COMMANDER_H
#define MODEL_COMMANDER_H

#include "dynamictreemodel.h"

#include "proxymodeltestsuite_export.h"

class QAbstractItemModel;

#include <QSet>
#include <QStringList>

class PROXYMODELTESTSUITE_EXPORT ModelCommander : public QObject
{
    Q_OBJECT
public:
    explicit ModelCommander(DynamicTreeModel *model, QObject *parent);

    ModelChangeCommand *currentCommand();

public Q_SLOTS:
    void init_testInsertWhenEmpty(const QString &dataTag);
    void init_testInsertInRoot(const QString &dataTag);
    void init_testInsertInTopLevel(const QString &dataTag);
    void init_testInsertInSecondLevel(const QString &dataTag);

    void init_testRemoveFromRoot(const QString &dataTag);
    void init_testRemoveFromTopLevel(const QString &dataTag);
    void init_testRemoveFromSecondLevel(const QString &dataTag);

    void init_testMoveFromRoot(const QString &dataTag);
    void init_testMoveFromTopLevel(const QString &dataTag);
    void init_testMoveFromSecondLevel(const QString &dataTag);

    void init_testModifyInRoot(const QString &dataTag);
    void init_testModifyInTopLevel(const QString &dataTag);
    void init_testModifyInSecondLevel(const QString &dataTag);

    QStringList execute_testInsertWhenEmpty(const QString &dataTag);
    QStringList execute_testInsertInRoot(const QString &dataTag);
    QStringList execute_testInsertInTopLevel(const QString &dataTag);
    QStringList execute_testInsertInSecondLevel(const QString &dataTag);

    QStringList execute_testRemoveFromRoot(const QString &dataTag);
    QStringList execute_testRemoveFromTopLevel(const QString &dataTag);
    QStringList execute_testRemoveFromSecondLevel(const QString &dataTag);

    QStringList execute_testMoveFromRoot(const QString &dataTag);
    QStringList execute_testMoveFromTopLevel(const QString &dataTag);
    QStringList execute_testMoveFromSecondLevel(const QString &dataTag);

    QStringList execute_testModifyInRoot(const QString &dataTag);
    QStringList execute_testModifyInTopLevel(const QString &dataTag);
    QStringList execute_testModifyInSecondLevel(const QString &dataTag);

private:
    QStringList executeTestInsert(QList<int> rowAncestors, const QString &dataTag);
    QStringList executeTestRemove(QList<int> rowAncestors, const QString &dataTag);
    QStringList executeTestMove(QList<int> rowAncestors, const QString &dataTag);
    QStringList executeTestModify(QList<int> rowAncestors, const QString &dataTag);

    void initTestModel(const QString &dataTag);

    void execute(ModelChangeCommand *command);

private:
    int m_counter;
    DynamicTreeModel *m_model;
    ModelChangeCommand *m_currentCommand;
    QSet<QString> m_testsSeen;
};

#endif

/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelcommander.h"
#include <QDebug>

ModelCommander::ModelCommander(DynamicTreeModel *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
{
}

QStringList ModelCommander::execute_testInsertWhenEmpty(const QString &dataTag)
{
    ModelInsertCommand *ins = new ModelInsertCommand(m_model, this);
    ins->setStartRow(0);

    static const QStringList testData = QStringList() << QStringLiteral("insert01") << QStringLiteral("insert02") << QStringLiteral("insert03");

    if (dataTag.isEmpty()) {
        return testData;
    }

    if (dataTag == testData.at(0)) {
        // Insert a single item at the top.
        ins->setEndRow(0);
    } else if (dataTag == testData.at(1)) {
        // Insert 10 items.
        ins->setEndRow(9);
    } else if (dataTag == testData.at(2)) {
        // Insert 5 items, some of which are parents
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - 3"
                          " - 4"
                          " - - 5"
                          " - - - 6"
                          " - 7"
                          " - 8"
                          " - - 9"));
    } else {
        qDebug() << dataTag;
        return testData;
    }
    execute(ins);
    return testData;
}

void ModelCommander::init_testInsertWhenEmpty(const QString &dataTag)
{
    Q_UNUSED(dataTag);
}

void ModelCommander::init_testInsertInRoot(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testInsertInTopLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testInsertInSecondLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testRemoveFromRoot(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testRemoveFromSecondLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testRemoveFromTopLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testMoveFromRoot(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testMoveFromSecondLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testMoveFromTopLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testModifyInRoot(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testModifyInTopLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::init_testModifyInSecondLevel(const QString &dataTag)
{
    initTestModel(dataTag);
}

void ModelCommander::initTestModel(const QString &dataTag)
{
    Q_UNUSED(dataTag);

    // A standard initial model for all these tests.
    ModelInsertCommand *ins = new ModelInsertCommand(m_model, this);
    ins->setStartRow(0);
    ins->interpret(
        QLatin1String(" - 1"
                      " - 2"
                      " - - 3"
                      " - - 4"
                      " - - 5"
                      " - 6"
                      " - 7"
                      " - 8"
                      " - 9"
                      " - - 10"
                      " - - 11"
                      " - - - 12"
                      " - - - 13"
                      " - - 14"
                      " - - 15"
                      " - - 16"
                      " - - 17"
                      " - - - 18"
                      " - - - 19"
                      " - - - - 20"
                      " - - - - 21"
                      " - - - 22"
                      " - - - 23"
                      " - - - 24"
                      " - - - 25"
                      " - - - - 26"
                      " - - - - 27"
                      " - - - 28"
                      " - - - 29"
                      " - - - - 30"
                      " - - - 31"
                      " - - - 32"
                      " - - 33"
                      " - - 34"
                      " - - - 35"
                      " - - 36"
                      " - - 37"
                      " - 38"
                      " - 39"
                      " - - 40"
                      " - - 41"
                      " - 42"
                      " - 43"));
    execute(ins);
}

QStringList ModelCommander::execute_testInsertInRoot(const QString &dataTag)
{
    return executeTestInsert(QList<int>(), dataTag);
}

QStringList ModelCommander::execute_testInsertInTopLevel(const QString &dataTag)
{
    return executeTestInsert(QList<int>() << 5, dataTag);
}

QStringList ModelCommander::execute_testInsertInSecondLevel(const QString &dataTag)
{
    return executeTestInsert(QList<int>() << 5 << 5, dataTag);
}

QStringList ModelCommander::execute_testRemoveFromRoot(const QString &dataTag)
{
    return executeTestRemove(QList<int>(), dataTag);
}

QStringList ModelCommander::execute_testRemoveFromTopLevel(const QString &dataTag)
{
    return executeTestRemove(QList<int>() << 5, dataTag);
}

QStringList ModelCommander::execute_testRemoveFromSecondLevel(const QString &dataTag)
{
    return executeTestRemove(QList<int>() << 5 << 5, dataTag);
}

QStringList ModelCommander::execute_testMoveFromRoot(const QString &dataTag)
{
    return executeTestMove(QList<int>(), dataTag);
}

QStringList ModelCommander::execute_testMoveFromTopLevel(const QString &dataTag)
{
    return executeTestMove(QList<int>() << 5, dataTag);
}

QStringList ModelCommander::execute_testMoveFromSecondLevel(const QString &dataTag)
{
    return executeTestMove(QList<int>() << 5 << 5, dataTag);
}

QStringList ModelCommander::execute_testModifyInRoot(const QString &dataTag)
{
    return executeTestModify(QList<int>(), dataTag);
}

QStringList ModelCommander::execute_testModifyInTopLevel(const QString &dataTag)
{
    return executeTestModify(QList<int>() << 5, dataTag);
}

QStringList ModelCommander::execute_testModifyInSecondLevel(const QString &dataTag)
{
    return executeTestModify(QList<int>() << 5 << 5, dataTag);
}

void ModelCommander::execute(ModelChangeCommand *command)
{
    m_currentCommand = command;
    command->doCommand();
    delete command;
    command = nullptr;
}

ModelChangeCommand *ModelCommander::currentCommand()
{
    return m_currentCommand;
}

QStringList ModelCommander::executeTestRemove(QList<int> rowAncestors, const QString &dataTag)
{
    static const QStringList testData = QStringList() << QStringLiteral("remove01") << QStringLiteral("remove02") << QStringLiteral("remove03");

    if (dataTag.isEmpty()) {
        return testData;
    }

    ModelRemoveCommand *rem = new ModelRemoveCommand(m_model, this);
    if (dataTag == testData.at(0)) {
        // Remove a single item from the top.
        rem->setAncestorRowNumbers(rowAncestors);
        rem->setStartRow(0);
        rem->setEndRow(0);
    } else if (dataTag == testData.at(1)) {
        // Remove four items form the top.
        rem->setAncestorRowNumbers(rowAncestors);
        rem->setStartRow(0);
        rem->setEndRow(4);
    } else if (dataTag == testData.at(2)) {
        // Remove a single item from the bottom.
        rem->setAncestorRowNumbers(rowAncestors);
        rem->setStartRow(m_model->rowCount() - 1);
        rem->setEndRow(m_model->rowCount() - 1);
    }
    execute(rem);
    return testData;
}

QStringList ModelCommander::executeTestMove(QList<int> rowAncestors, const QString &dataTag)
{
    static const QStringList testData = QStringList() << QStringLiteral("move01") << QStringLiteral("move02") << QStringLiteral("move03")
                                                      << QStringLiteral("move04") << QStringLiteral("move05");

    if (dataTag.isEmpty()) {
        return testData;
    }

    ModelMoveCommand *move = new ModelMoveCommand(m_model, this);
    if (dataTag == testData.at(0)) {
        // Move a single item from the top to the middle in the same parent.
        move->setAncestorRowNumbers(rowAncestors);
        move->setStartRow(0);
        move->setEndRow(0);
        move->setDestAncestors(rowAncestors);
        move->setDestRow(5);
    } else if (dataTag == testData.at(1)) {
        // Move a single item from the middle to the top in the same parent.
        move->setAncestorRowNumbers(rowAncestors);
        move->setStartRow(4);
        move->setEndRow(4);
        move->setDestAncestors(rowAncestors);
        move->setDestRow(0);
    } else if (dataTag == testData.at(2)) {
        // Move a single item from the middle to the bottom in the same parent.
        move->setAncestorRowNumbers(rowAncestors);
        move->setStartRow(4);
        move->setEndRow(4);
        move->setDestAncestors(rowAncestors);
        move->setDestRow(10);
    } else if (dataTag == testData.at(3)) {
        // Move a single item from the bottom to the middle in the same parent.
        move->setAncestorRowNumbers(rowAncestors);
        move->setStartRow(9);
        move->setEndRow(9);
        move->setDestAncestors(rowAncestors);
        move->setDestRow(4);
    } else if (dataTag == testData.at(4)) {
        // Move a single item from the bottom to the top in the same parent.
        move->setAncestorRowNumbers(rowAncestors);
        move->setStartRow(9);
        move->setEndRow(9);
        move->setDestAncestors(rowAncestors);
        move->setDestRow(0);
    }
    execute(move);
    return testData;
}

QStringList ModelCommander::executeTestModify(QList<int> rowAncestors, const QString &dataTag)
{
    static const QStringList testData = QStringList() << QStringLiteral("modify01") << QStringLiteral("modify02") << QStringLiteral("modify03")
                                                      << QStringLiteral("modify04") << QStringLiteral("modify05") << QStringLiteral("modify06")
                                                      << QStringLiteral("modify07");

    if (dataTag.isEmpty()) {
        return testData;
    }

    ModelDataChangeCommand *modify = new ModelDataChangeCommand(m_model, this);
    if (dataTag == testData.at(0)) {
        // Modify a single item at the top.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(0);
        modify->setEndRow(0);
    } else if (dataTag == testData.at(1)) {
        // Modify four items at the top.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(0);
        modify->setEndRow(4);
    } else if (dataTag == testData.at(2)) {
        // Modify a single item at the bottom.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(m_model->rowCount() - 1);
        modify->setEndRow(m_model->rowCount() - 1);
    } else if (dataTag == testData.at(3)) {
        // Modify four items at the bottom.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(m_model->rowCount() - 4);
        modify->setEndRow(m_model->rowCount() - 1);
    } else if (dataTag == testData.at(4)) {
        // Modify a single item in the middle.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(4);
        modify->setEndRow(4);
    } else if (dataTag == testData.at(5)) {
        // Modify four items in the middle.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(3);
        modify->setEndRow(7);
    } else if (dataTag == testData.at(6)) {
        // Modify all items.
        modify->setAncestorRowNumbers(rowAncestors);
        modify->setStartRow(0);
        modify->setEndRow(m_model->rowCount() - 1);
    }
    execute(modify);
    return testData;
}

QStringList ModelCommander::executeTestInsert(QList<int> rowAncestors, const QString &dataTag)
{
    static const QStringList testData = QStringList() << QStringLiteral("insert01") << QStringLiteral("insert02") << QStringLiteral("insert03")
                                                      << QStringLiteral("insert04") << QStringLiteral("insert05") << QStringLiteral("insert06")
                                                      << QStringLiteral("insert07") << QStringLiteral("insert08") << QStringLiteral("insert09")
                                                      << QStringLiteral("insert10") << QStringLiteral("insert11") << QStringLiteral("insert12")
                                                      << QStringLiteral("insert13") << QStringLiteral("insert14") << QStringLiteral("insert15")
                                                      << QStringLiteral("insert16") << QStringLiteral("insert17") << QStringLiteral("insert18");

    if (dataTag.isEmpty()) {
        return testData;
    }

    ModelInsertCommand *ins = new ModelInsertCommand(m_model, this);
    if (dataTag == testData.at(0)) {
        // Insert a single item at the top.
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(0);
        ins->setEndRow(0);
    } else if (dataTag == testData.at(1)) {
        // Insert 10 items at the top.
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(0);
        ins->setEndRow(9);
    } else if (dataTag == testData.at(2)) {
        // Insert a single item at the bottom.
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(m_model->rowCount());
        ins->setEndRow(m_model->rowCount());
    } else if (dataTag == testData.at(3)) {
        // Insert a 10 items at the bottom.
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(m_model->rowCount());
        ins->setEndRow(m_model->rowCount() + 9);
    } else if (dataTag == testData.at(4)) {
        // Insert a single item in the middle
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(4);
        ins->setEndRow(4);
    } else if (dataTag == testData.at(5)) {
        // Insert 10 items in the middle
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(4);
        ins->setEndRow(13);
    } else if (dataTag == testData.at(6)) {
        // Insert a single item at with children at the top
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(0);
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - - - 3"
                          " - - - - 4"
                          " - - 5"));
    } else if (dataTag == testData.at(7)) {
        // Insert a single item at with children at the bottom
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(m_model->rowCount());
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - - - 3"
                          " - - - - 4"
                          " - - 5"));
    } else if (dataTag == testData.at(8)) {
        // Insert a single item at with children in the middle
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(4);
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - - - 3"
                          " - - - - 4"
                          " - - 5"));
    } else if (dataTag == testData.at(9)) {
        // Insert 5 items, some of which are parents at the top
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(0);
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - 3"
                          " - 4"
                          " - - 5"
                          " - - 6"
                          " - - - 7"
                          " - 8"
                          " - 9"
                          " - - 10"));
    } else if (dataTag == testData.at(10)) {
        // Insert 5 items, some of which are parents at the bottom
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(m_model->rowCount());
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - 3"
                          " - 4"
                          " - - 5"
                          " - - 6"
                          " - - - 7"
                          " - 8"
                          " - 9"
                          " - - 10"));
    } else if (dataTag == testData.at(11)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors);
        ins->setStartRow(4);
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - 3"
                          " - 4"
                          " - - 5"
                          " - - 6"
                          " - - - 7"
                          " - 8"
                          " - 9"
                          " - - 10"));
    } else if (dataTag == testData.at(12)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors << 0);
        ins->setStartRow(0);
        ins->setEndRow(0);
    } else if (dataTag == testData.at(13)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors << 0);
        ins->setStartRow(0);
        ins->setEndRow(9);
    } else if (dataTag == testData.at(14)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors << 0);
        ins->setStartRow(0);
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - 3"
                          " - 4"
                          " - - 5"
                          " - - 6"
                          " - - - 7"
                          " - 8"
                          " - 9"
                          " - - 10"));
    } else if (dataTag == testData.at(15)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors << 9);
        ins->setStartRow(0);
        ins->setEndRow(0);
    } else if (dataTag == testData.at(16)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors << 9);
        ins->setStartRow(0);
        ins->setEndRow(9);
    } else if (dataTag == testData.at(17)) {
        // Insert 5 items, some of which are parents in the middle
        ins->setAncestorRowNumbers(rowAncestors << 9);
        ins->setStartRow(0);
        ins->interpret(
            QLatin1String(" - 1"
                          " - - 2"
                          " - 3"
                          " - 4"
                          " - - 5"
                          " - - 6"
                          " - - - 7"
                          " - 8"
                          " - 9"
                          " - - 10"));
    } else {
        qDebug() << dataTag;
        delete ins;
        return testData;
    }
    execute(ins);
    return testData;
}

#include "moc_modelcommander.cpp"

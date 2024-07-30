/*
    SPDX-FileCopyrightText: 2013 Aurélien Gateau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klinkitemselectionmodeltest.h"

#include <klinkitemselectionmodel.h>

#include <QIdentityProxyModel>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QStandardItemModel>

#include <QTest>

void KLinkItemSelectionModelTest::init()
{
    // Init m_mainModel
    m_mainModel = new QStandardItemModel;
    for (int x = 0; x < 10; ++x) {
        m_mainModel->appendRow(new QStandardItem(QString::number(x)));
    }
    m_mainSelectionModel = new QItemSelectionModel(m_mainModel);

    // Init subModel
    m_subModel = new QSortFilterProxyModel;
    m_subModel->setFilterRegularExpression(QRegularExpression(QStringLiteral("^[5-9]")));
    m_subModel->setSourceModel(m_mainModel);
    m_subSelectionModel = new KLinkItemSelectionModel(m_subModel, m_mainSelectionModel);
}

void KLinkItemSelectionModelTest::cleanup()
{
    delete m_mainSelectionModel;
    m_mainSelectionModel = nullptr;
    delete m_mainModel;
    m_mainModel = nullptr;
    delete m_subSelectionModel;
    m_subSelectionModel = nullptr;
    delete m_subModel;
    m_subModel = nullptr;
}

void KLinkItemSelectionModelTest::testToggle()
{
    // Select last index in subModel
    QModelIndex subIndex = m_subModel->index(m_subModel->rowCount() - 1, 0);
    m_subSelectionModel->select(subIndex, QItemSelectionModel::Toggle);

    // Check selections
    QModelIndexList subList = m_subSelectionModel->selectedIndexes();
    QCOMPARE(subList.count(), 1);
    QCOMPARE(subList.first(), subIndex);

    QModelIndexList mainList = m_mainSelectionModel->selectedIndexes();
    QModelIndex mainIndex = m_mainModel->index(m_mainModel->rowCount() - 1, 0);
    QCOMPARE(mainList.count(), 1);
    QCOMPARE(mainList.first(), mainIndex);
}

void KLinkItemSelectionModelTest::testMainSetCurrent()
{
    // Set last index of mainModel as current
    QModelIndex mainIndex = m_mainModel->index(m_mainModel->rowCount() - 1, 0);
    m_mainSelectionModel->setCurrentIndex(mainIndex, QItemSelectionModel::Current);

    // Last index of subModel should be current as well
    QModelIndex subIndex = m_subSelectionModel->currentIndex();
    QVERIFY(subIndex.isValid());
    QCOMPARE(subIndex, m_subModel->index(m_subModel->rowCount() - 1, 0));

    // Set first index of mainModel as current. First index of mainModel does
    // not exist in subModel, so its current index should remain the same.
    subIndex = m_subSelectionModel->currentIndex();
    QVERIFY(subIndex.isValid());
    mainIndex = m_mainModel->index(0, 0);
    m_mainSelectionModel->setCurrentIndex(mainIndex, QItemSelectionModel::Current);
    QCOMPARE(subIndex, m_subSelectionModel->currentIndex());
}

void KLinkItemSelectionModelTest::testSubSetCurrent()
{
    // Set last index of subModel as current
    QModelIndex subIndex = m_subModel->index(m_subModel->rowCount() - 1, 0);
    m_subSelectionModel->setCurrentIndex(subIndex, QItemSelectionModel::Current);

    // Last index of mainModel should be current as well
    QModelIndex mainIndex = m_mainSelectionModel->currentIndex();
    QVERIFY(mainIndex.isValid());
    QCOMPARE(mainIndex, m_mainModel->index(m_mainModel->rowCount() - 1, 0));
}

void KLinkItemSelectionModelTest::testChangeModel()
{
    QVERIFY(m_mainSelectionModel->selection().isEmpty());

    {
        auto idx = m_mainModel->index(6, 0);
        m_mainSelectionModel->select(idx, QItemSelectionModel::Select);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 6);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 1);

    QIdentityProxyModel newIntermediateProxy;

    newIntermediateProxy.setSourceModel(m_mainModel);

    // Change the model of the KLinkItemSelectionModel
    m_subSelectionModel->setModel(&newIntermediateProxy);

    QVERIFY(m_mainSelectionModel->selection().isEmpty());
    QVERIFY(m_subSelectionModel->selection().isEmpty());
}

void KLinkItemSelectionModelTest::testChangeModelOfExternal()
{
    QVERIFY(m_mainSelectionModel->selection().isEmpty());

    {
        auto idx = m_mainModel->index(6, 0);
        m_mainSelectionModel->select(idx, QItemSelectionModel::Select);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 6);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 1);

    QIdentityProxyModel newIntermediateProxy;

    newIntermediateProxy.setSourceModel(m_mainModel);

    // Change the model of the external QISM
    m_mainSelectionModel->setModel(&newIntermediateProxy);

    QVERIFY(m_mainSelectionModel->selection().isEmpty());
    QVERIFY(m_subSelectionModel->selection().isEmpty());
}

void KLinkItemSelectionModelTest::testChangeLinkedSelectionModel()
{
    QVERIFY(m_mainSelectionModel->selection().isEmpty());

    {
        auto idx = m_mainModel->index(6, 0);
        m_mainSelectionModel->select(idx, QItemSelectionModel::Select);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 6);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 1);

    QItemSelectionModel replacementSelectionModel(m_mainModel, nullptr);
    {
        auto idx = m_mainModel->index(7, 0);
        replacementSelectionModel.select(idx, QItemSelectionModel::Select);
    }

    m_subSelectionModel->setLinkedItemSelectionModel(&replacementSelectionModel);

    QVERIFY(!replacementSelectionModel.selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(replacementSelectionModel.selection().indexes().first().row(), 7);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 2);
}

void KLinkItemSelectionModelTest::testAdditionalLink()
{
    {
        auto idx = m_mainModel->index(6, 0);
        m_mainSelectionModel->select(idx, QItemSelectionModel::Select);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 6);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 1);

    {
        QSortFilterProxyModel additionalProxy;
        additionalProxy.setFilterRegularExpression(QRegularExpression(QStringLiteral("^[3-9]")));
        additionalProxy.setSourceModel(m_mainModel);

        KLinkItemSelectionModel additionalLink;
        additionalLink.setModel(&additionalProxy);
        additionalLink.setLinkedItemSelectionModel(m_mainSelectionModel);
        QVERIFY(!additionalLink.selection().isEmpty());
        QCOMPARE(additionalLink.selection().indexes().first().row(), 3);

        auto idx = additionalProxy.index(4, 0);
        QVERIFY(idx.isValid());
        additionalLink.select(idx, QItemSelectionModel::ClearAndSelect);

        QVERIFY(!additionalLink.selection().isEmpty());
        QCOMPARE(additionalLink.selection().indexes().first().row(), 4);

        QVERIFY(!m_mainSelectionModel->selection().isEmpty());
        QVERIFY(!m_subSelectionModel->selection().isEmpty());
        QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 7);
        QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 2);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 7);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 2);
}

void KLinkItemSelectionModelTest::testClearSelection()
{
    QVERIFY(m_mainSelectionModel->selection().isEmpty());

    {
        auto idx = m_mainModel->index(6, 0);
        m_mainSelectionModel->select(idx, QItemSelectionModel::Select);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 6);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 1);

    m_subSelectionModel->clear();

    QVERIFY(m_mainSelectionModel->selection().isEmpty());
    QVERIFY(m_subSelectionModel->selection().isEmpty());

    {
        auto idx = m_mainModel->index(6, 0);
        m_mainSelectionModel->select(idx, QItemSelectionModel::Select);
    }

    QVERIFY(!m_mainSelectionModel->selection().isEmpty());
    QVERIFY(!m_subSelectionModel->selection().isEmpty());
    QCOMPARE(m_mainSelectionModel->selection().indexes().first().row(), 6);
    QCOMPARE(m_subSelectionModel->selection().indexes().first().row(), 1);

    m_mainSelectionModel->clear();

    QVERIFY(m_mainSelectionModel->selection().isEmpty());
    QVERIFY(m_subSelectionModel->selection().isEmpty());
}

QTEST_MAIN(KLinkItemSelectionModelTest)

#include "moc_klinkitemselectionmodeltest.cpp"

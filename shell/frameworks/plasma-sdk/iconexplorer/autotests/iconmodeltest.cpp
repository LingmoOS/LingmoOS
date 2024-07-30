/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include <QDebug>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>

#include "iconmodel.h"
#include "sortfiltermodel.h"

using namespace CuttleFish;

class IconModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init()
    {
        m_iconModel = new IconModel(this);
        m_proxyModel = new SortFilterModel(this);
        m_proxyModel->setSourceModel(m_iconModel);
    }

    void cleanup()
    {
        delete m_iconModel;
        delete m_proxyModel;
    }

    void initTestCase(){};

    void testCategoryFilter()
    {
        const int _all = m_iconModel->rowCount(QModelIndex());

        m_proxyModel->setFilter("edit");
        const int _edit = m_proxyModel->rowCount(QModelIndex());

        m_proxyModel->setCategory("actions");
        const int _editactions = m_proxyModel->rowCount(QModelIndex());

        m_proxyModel->setCategory("all");
        const int _alledit = m_proxyModel->rowCount(QModelIndex());

        QVERIFY(_all > _edit);
        QVERIFY(_all > _editactions);
        QVERIFY(_edit >= _editactions);
        QVERIFY(_alledit >= _editactions);
    }

private: // disable from here for testing just the above
private:
    QJsonArray m_data;
    QJsonArray m_empty;

    IconModel *m_iconModel;
    SortFilterModel *m_proxyModel;
};

QTEST_MAIN(IconModelTest)

#include "iconmodeltest.moc"

/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "personsmodeltest.h"

#include "fakecontactsource.h"

// private includes
#include "personmanager_p.h"

// public kpeople includes
#include <personpluginmanager.h>
#include <personsmodel.h>

#include <QSignalSpy>
#include <QTest>
#include <QVariant>

QTEST_GUILESS_MAIN(PersonsModelTest)

using namespace KPeople;

void PersonsModelTest::initTestCase()
{
    QVERIFY(m_database.open());

    // Called before the first testfunction is executed
    PersonManager::instance(m_database.fileName());
    m_source = new FakeContactSource(nullptr); // don't own. PersonPluginManager removes it on destruction
    QHash<QString, BasePersonsDataSource *> sources;
    sources[QStringLiteral("fakesource")] = m_source;
    PersonPluginManager::setDataSourcePlugins(sources);

    m_model = new KPeople::PersonsModel(this);
    QSignalSpy modelInit(m_model, SIGNAL(modelInitialized(bool)));

    QTRY_COMPARE(modelInit.count(), 1);
    QCOMPARE(modelInit.first().at(0).toBool(), true);
}

void PersonsModelTest::cleanupTestCase()
{
    // Called after the last testfunction was executed
    m_database.close();
}

void PersonsModelTest::loadModel()
{
    QCOMPARE(m_model->rowCount(), 4);
    QCOMPARE(m_model->data(m_model->index(0)).toString(), QStringLiteral("Contact 1"));
    QCOMPARE(m_model->data(m_model->index(1)).toString(), QStringLiteral("Contact 2"));
    QCOMPARE(m_model->data(m_model->index(2)).toString(), QStringLiteral("Contact 3"));
    QCOMPARE(m_model->data(m_model->index(3)).toString(), QStringLiteral("Contact 4"));

    m_source->changeProperty(AbstractContact::NameProperty, QStringLiteral("Contact A"));

    QCOMPARE(m_model->rowCount(), 4);
    QCOMPARE(m_model->data(m_model->index(0)).toString(), QStringLiteral("Contact A"));
    QCOMPARE(m_model->data(m_model->index(1)).toString(), QStringLiteral("Contact 2"));
    QCOMPARE(m_model->data(m_model->index(2)).toString(), QStringLiteral("Contact 3"));
    QCOMPARE(m_model->data(m_model->index(3)).toString(), QStringLiteral("Contact 4"));
}

void PersonsModelTest::mergeContacts()
{
    QStringList uris{QStringLiteral("fakesource://contact1"), QStringLiteral("fakesource://contact2")};
    QSignalSpy modelRowsInsert(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)));

    QCOMPARE(m_model->rowCount(), 4);
    QString newUri = KPeople::mergeContacts(uris);
    QCOMPARE(newUri, QStringLiteral("kpeople://1"));

    // TODO: replace with actual model signals spying
    QTRY_COMPARE(m_model->rowCount(), 3);
    QCOMPARE(m_model->rowCount(m_model->indexForPersonUri(newUri)), 2);

    // There needs to be 2 rows inserted - one for the new Person
    // and one for the new contact added to it (the other contact
    // is already a child of the person; merging just takes all
    // contacts from one person and adds them to the other)
    QCOMPARE(modelRowsInsert.count(), 2);
    // The first inserted Person must have invalid parent index
    QCOMPARE(modelRowsInsert.first().at(0).toModelIndex(), QModelIndex());
    // Second inserted row, the Contact, must have the Person index as parent
    QCOMPARE(modelRowsInsert.at(1).at(0).toModelIndex(), m_model->indexForPersonUri(newUri));

    modelRowsInsert.clear();

    QStringList uris2{QStringLiteral("fakesource://contact3"), newUri};
    QString newUri2 = KPeople::mergeContacts(uris2);
    QCOMPARE(newUri2, QStringLiteral("kpeople://1"));

    QTest::qWait(2000);

    QCOMPARE(m_model->rowCount(), 2);
    QCOMPARE(m_model->rowCount(m_model->indexForPersonUri(newUri2)), 3);
    QCOMPARE(modelRowsInsert.count(), 1);
    QCOMPARE(modelRowsInsert.first().at(0).toModelIndex(), m_model->indexForPersonUri(newUri));
}

void PersonsModelTest::gettersTests()
{
    // Find the index for "kpeople://1" using the QAIModel method
    QModelIndexList indexList =
        m_model->match(m_model->index(0, 0, QModelIndex()), KPeople::PersonsModel::PersonUriRole, QVariant(QStringLiteral("kpeople://1")), 1);
    QModelIndex personIndex = indexList.first();

    // Now get the index using our method
    QModelIndex indexForPerson = m_model->indexForPersonUri(QStringLiteral("kpeople://1"));

    // Now compare
    QCOMPARE(personIndex, indexForPerson);

    // TODO: also test the get() method?
}

void PersonsModelTest::unmergeContacts()
{
    QModelIndex personIndex = m_model->indexForPersonUri(QStringLiteral("kpeople://1"));
    QSignalSpy modelRowsInsert(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)));
    QSignalSpy modelRowsRemove(m_model, SIGNAL(rowsRemoved(QModelIndex, int, int)));

    QCOMPARE(m_model->rowCount(), 2);
    QCOMPARE(m_model->rowCount(personIndex), 3);

    KPeople::unmergeContact(QStringLiteral("fakesource://contact3"));
    QTest::qWait(2000);

    QCOMPARE(m_model->rowCount(), 3);
    QCOMPARE(m_model->rowCount(personIndex), 2);

    // The unmerged Contact is turned into new Person (the fake Person where Person == Contact)
    // There must be 1 insertion and the parent must be invalid index
    QCOMPARE(modelRowsInsert.count(), 1);
    QCOMPARE(modelRowsInsert.first().at(0).toModelIndex(), QModelIndex());

    // Similarly, there must be one row removed and the parent must be
    // the old Person index
    QCOMPARE(modelRowsRemove.count(), 1);
    QCOMPARE(modelRowsRemove.first().at(0).toModelIndex(), personIndex);

    modelRowsInsert.clear();
    modelRowsRemove.clear();

    KPeople::unmergeContact(QStringLiteral("kpeople://1"));
    QTest::qWait(2000);
    QCOMPARE(m_model->rowCount(), 4);
    // Check that the person is gone from the model
    QCOMPARE(m_model->indexForPersonUri(QStringLiteral("kpeople://1")), QModelIndex());

    QCOMPARE(modelRowsInsert.count(), 2);
    QCOMPARE(modelRowsInsert.first().at(0).toModelIndex(), QModelIndex());
    QCOMPARE(modelRowsInsert.at(1).at(0).toModelIndex(), QModelIndex());

    // There must be exactly 3 rows removed when unmerging a Person
    // with 2 contacts - first the subcontacts are removed and then
    // the parent Person itself
    QCOMPARE(modelRowsRemove.count(), 3);
    // The first two Contacts must have parent the Person index
    // and both should have 0 0 as the "first last" args of removeRows
    QCOMPARE(modelRowsRemove.first().at(0).toModelIndex(), personIndex);
    QCOMPARE(modelRowsRemove.first().at(1).toInt(), 0);
    QCOMPARE(modelRowsRemove.first().at(2).toInt(), 0);

    QCOMPARE(modelRowsRemove.at(1).at(0).toModelIndex(), personIndex);
    QCOMPARE(modelRowsRemove.at(1).at(1).toInt(), 0);
    QCOMPARE(modelRowsRemove.at(1).at(2).toInt(), 0);

    // The parent Person should have just invalid index as parent
    // (and we don't care about the position)
    QCOMPARE(modelRowsRemove.at(2).at(0).toModelIndex(), QModelIndex());
}

#include "moc_personsmodeltest.cpp"

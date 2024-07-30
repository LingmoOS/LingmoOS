/*
    SPDX-FileCopyrightText: 2015 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "personsproxymodeltest.h"

#include "fakecontactsource.h"

// private includes
#include "personmanager_p.h"

// public kpeople includes
#include <personpluginmanager.h>
#include <personsmodel.h>
#include <personssortfilterproxymodel.h>

#include <QSignalSpy>
#include <QTest>

QTEST_GUILESS_MAIN(PersonsProxyModelTest)

using namespace KPeople;

void PersonsProxyModelTest::initTestCase()
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

void PersonsProxyModelTest::cleanupTestCase()
{
    // Called after the last testfunction was executed
    m_database.close();
}

void PersonsProxyModelTest::testFiltering()
{
    PersonsSortFilterProxyModel proxy;
    proxy.setSourceModel(m_model);

    QCOMPARE(proxy.rowCount(), 4);

    proxy.setRequiredProperties(QStringList() << AbstractContact::PhoneNumberProperty);

    QCOMPARE(proxy.rowCount(), 2);

    proxy.setRequiredProperties(QStringList() << AbstractContact::PhoneNumberProperty << KPeople::AbstractContact::PresenceProperty);

    QCOMPARE(proxy.rowCount(), 3);
}

#include "moc_personsproxymodeltest.cpp"

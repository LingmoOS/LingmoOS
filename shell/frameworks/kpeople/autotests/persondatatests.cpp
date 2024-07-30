/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "persondatatests.h"

#include "fakecontactsource.h"

// private includes
#include "personmanager_p.h"

// public kpeople includes
#include <persondata.h>
#include <personpluginmanager.h>

#include <QSignalSpy>
#include <QTest>

QTEST_GUILESS_MAIN(PersonDataTests)

using namespace KPeople;

// this tests PersonData but also implicitly tests the private classes
// - BasePersonsDataSource
// - DefaultContactMonitor
// - MetaContact

void PersonDataTests::initTestCase()
{
    QVERIFY(m_database.open());

    // Called before the first testfunction is executed
    PersonManager::instance(m_database.fileName());
    PersonManager::instance()->mergeContacts(QStringList() << QStringLiteral("fakesource://contact2") << QStringLiteral("fakesource://contact3"));

    m_source = new FakeContactSource(nullptr); // don't own. PersonPluginManager removes it on destruction
    QHash<QString, BasePersonsDataSource *> sources;
    sources[QStringLiteral("fakesource")] = m_source;
    PersonPluginManager::setDataSourcePlugins(sources);
}

void PersonDataTests::cleanupTestCase()
{
    // Called after the last testfunction was executed
    m_database.close();
}

void PersonDataTests::init()
{
    // Called before each testfunction is executed
}

void PersonDataTests::cleanup()
{
    // Called after every testfunction
}

void PersonDataTests::loadContact()
{
    QString personUri = QStringLiteral("fakesource://contact1");
    PersonData person(personUri);
    // in this case we know the datasource is synchronous, but we should extend the test to cope with it not being async.

    QCOMPARE(person.contactUris().size(), 1);
    QCOMPARE(person.name(), QStringLiteral("Contact 1"));
    QCOMPARE(person.allEmails(), QStringList(QStringLiteral("contact1@example.com")));
    QCOMPARE(person.personUri(), personUri);
}

void PersonDataTests::loadPerson()
{
    // loading contact 2 which is already merged should return person1
    // which is both contact 2 and 3
    PersonData person(QStringLiteral("fakesource://contact2"));

    QCOMPARE(person.contactUris().size(), 2);
    QCOMPARE(person.name(), QStringLiteral("Contact 2"));
    QCOMPARE(person.allEmails().size(), 2);
    QCOMPARE(person.personUri(), QStringLiteral("kpeople://1"));

    // convert to set as order is not important
    const QStringList allEmails = person.allEmails();
    const QSet<QString> allEmailsSet(allEmails.begin(), allEmails.end());
    QCOMPARE(allEmailsSet, QSet<QString>() << QStringLiteral("contact2@example.com") << QStringLiteral("contact3@example.com"));
}

void PersonDataTests::contactChanged()
{
    PersonData person(QStringLiteral("fakesource://contact1"));

    QCOMPARE(person.allEmails().at(0), QStringLiteral("contact1@example.com"));

    QSignalSpy spy(&person, SIGNAL(dataChanged()));
    m_source->changeProperty(AbstractContact::EmailProperty, QStringLiteral("newaddress@yahoo.com"));
    QCOMPARE(spy.count(), 1);

    QCOMPARE(person.allEmails().at(0), QStringLiteral("newaddress@yahoo.com"));
}

void PersonDataTests::nullPerson()
{
    PersonData person(QStringLiteral("fakesource://unexisting"));
    QCOMPARE(QString(), person.name());
    QVERIFY(!person.isValid());

    PersonData invalidPerson(QStringLiteral());
    QVERIFY(!invalidPerson.isValid());
}

void PersonDataTests::removeContact()
{
    PersonData person(QStringLiteral("fakesource://contact1"));

    QCOMPARE(person.allEmails().at(0), QStringLiteral("contact1@example.com"));

    QSignalSpy spy(&person, &PersonData::dataChanged);
    m_source->remove(person.personUri());
    QCOMPARE(spy.count(), 1);

    QCOMPARE(person.name(), QString());
}

#include "moc_persondatatests.cpp"

/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// unit test for entry

#include <QDomDocument>
#include <QString>
#include <QTest>
#include <QXmlStreamReader>

#include "entry.h"

#include <knewstuffcore_debug.h>

const char *entryXML =
    "<stuff category=\"Category\"> "
    "<name>Name</name>"
    "<providerid>https://api.opendesktop.org/v1/</providerid>"
    "<author homepage=\"http://testhomepage\">testauthor</author>"
    "<homepage>https://testhomepage</homepage>"
    "<licence>3</licence>" // krazy:exclude=spelling
    "<version>4.0</version>"
    "<rating>82</rating>"
    "<downloads>128891</downloads>"
    "<installedfile>/some/test/path.jpg</installedfile>"
    "<id>12345</id>"
    "<releasedate>2008-08-12<!--randomcomment--></releasedate>"
    "<summary>new version 3.0</summary>"
    "<!--randomcomment-->"
    "<changelog>Updated</changelog>"
    "<preview>https://testpreview</preview>"
    "<previewBig>https://testpreview</previewBig>"
    "<payload>http://testpayload</payload>"
    "<status>"
    "<!--randomcomment-->"
    "installed"
    "<!--randomcomment-->"
    "</status>"
    "<tags>ghns_excluded=1</tags>"
    "</stuff>";

const QString name = QStringLiteral("Name");
const QString category = QStringLiteral("Category");
const QString summary = QStringLiteral("new version 3.0");
const QString version = QStringLiteral("4.0");
const QString license = QStringLiteral("3");

class testEntry : public QObject
{
    Q_OBJECT
private:
    KNSCore::Entry createEntryOld();
    KNSCore::Entry createEntry();
private Q_SLOTS:
    void testProperties();
    void testCopy();
    void testAssignment();
    void testDomImplementation();
};

KNSCore::Entry testEntry::createEntryOld()
{
    QDomDocument document;
    document.setContent(QString::fromLatin1(entryXML));
    QDomElement node = document.documentElement();
    KNSCore::Entry entry;
    bool xmlResult = entry.setEntryXML(node);
    qCDebug(KNEWSTUFFCORE) << "Created entry from XML " << xmlResult;
    return entry;
}

KNSCore::Entry testEntry::createEntry()
{
    QXmlStreamReader reader;
    reader.addData(entryXML);
    KNSCore::Entry entry;
    bool xmlResult = reader.readNextStartElement() && entry.setEntryXML(reader);
    qCDebug(KNEWSTUFFCORE) << "Created entry from XML " << xmlResult;
    return entry;
}

void testEntry::testProperties()
{
    KNSCore::Entry entry = createEntry();

    QCOMPARE(entry.name(), name);
    QCOMPARE(entry.category(), category);
    QCOMPARE(entry.license(), license);
    QCOMPARE(entry.summary(), summary);
    QCOMPARE(entry.version(), version);
}

void testEntry::testCopy()
{
    KNSCore::Entry entry = createEntry();
    KNSCore::Entry entry2(entry);

    QCOMPARE(entry.name(), entry2.name());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary(), entry2.summary());
    QCOMPARE(entry.version(), entry2.version());
}

void testEntry::testAssignment()
{
    KNSCore::Entry entry = createEntry();
    KNSCore::Entry entry2 = entry;

    QCOMPARE(entry.name(), entry2.name());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary(), entry2.summary());
    QCOMPARE(entry.version(), entry2.version());
}

void testEntry::testDomImplementation()
{
    KNSCore::Entry entry = createEntry();
    KNSCore::Entry entry2 = createEntryOld();

    QCOMPARE(entry.name(), entry2.name());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary(), entry2.summary());
    QCOMPARE(entry.version(), entry2.version());
}

QTEST_GUILESS_MAIN(testEntry)
#include "knewstuffentrytest.moc"

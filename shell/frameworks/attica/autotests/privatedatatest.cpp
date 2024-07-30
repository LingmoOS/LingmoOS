/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Martin Sandsmark <martin.sandsmark@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "../privatedata.h"
#include "../privatedataparser.h"
#include <QTest>

using namespace Attica;

class PersonTest : public QObject
{
    Q_OBJECT

private slots:
    void testParsing();
    void testMergeType();
};

void PersonTest::testParsing()
{
    PrivateData::Parser parser;
    QString validData(
        "<?xml version=\"1.0\"?><ocs><attributes>"
        "<key>keyfoo</key>"
        "<value>valuebar</value>"
        "<changed>01.01.1998</changed>"
        "</attributes></ocs>");
    PrivateData attributes = parser.parse(validData);
    QVERIFY(attributes.attributeChanged("keyfoo").isValid());
    QVERIFY(!attributes.attribute("keyfoo").isNull());
}

void PersonTest::testMergeType()
{
    PrivateData a;
    a.setMergeType(PrivateData::OverwriteLocal);
    QVERIFY(a.mergeType() == PrivateData::OverwriteLocal);
}

QTEST_MAIN(PersonTest);

#include "moc_persontest.cxx"

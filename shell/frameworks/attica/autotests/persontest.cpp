/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Martin Sandsmark <martin.sandsmark@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>
#include <personparser.h>

using namespace Attica;

class PersonTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testParsing();
};

void PersonTest::testParsing()
{
    Person::Parser parser;
    QString validData(
        QLatin1String("<?xml version=\"1.0\"?>"
                      "<ocs><person>"
                      "<personid>10</personid>"
                      "<firstname>Ola</firstname>"
                      "<lastname>Nordmann</lastname>"
                      "<homepage>http://kde.org/</homepage>"
                      "<avatarpic>http://techbase.kde.org/skins/oxygen/top-kde.png</avatarpic>"
                      "<avatarpicfound>1</avatarpicfound>"
                      "<birthday>2010-06-21</birthday>"
                      "<city>Oslo</city>"
                      "<country>Norway</country>"
                      "<latitude>59.56</latitude>"
                      "<longitude>10.41</longitude>"
                      "</person></ocs>"));
    Person person = parser.parse(validData);
    QVERIFY(person.isValid());

    QString invalidData = QLatin1String("<ocs><braaaaaaaaaaawrlawrf></braaaaaaaaaaawrlawrf></ocs>");
    person = parser.parse(invalidData);
    QVERIFY(!person.isValid());
}

QTEST_MAIN(PersonTest)

#include "persontest.moc"

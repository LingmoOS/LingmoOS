/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>
#include <configparser.h>

using namespace Attica;

class ConfigTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testParsing();
};

void ConfigTest::testParsing()
{
    Config::Parser parser;
    QString validData(
        QLatin1String("<?xml version=\"1.0\"?>"
                      "<ocs><data>"
                      "<version>1.7</version>"
                      "<website>store.kde.org</website>"
                      "<host>api.kde-look.org</host>"
                      "<contact>contact@opendesktop.org</contact>"
                      "<ssl>true</ssl>"
                      "</data></ocs>"));
    Config config = parser.parse(validData);
    QVERIFY(config.isValid());

    QString invalidData = QLatin1String("<ocs><braaaaaaaaaaawrlawrf></braaaaaaaaaaawrlawrf></ocs>");
    config = parser.parse(invalidData);
    QVERIFY(!config.isValid());
}

QTEST_GUILESS_MAIN(ConfigTest)

#include "configtest.moc"

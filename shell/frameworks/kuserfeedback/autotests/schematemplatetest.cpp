/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <core/aggregation.h>
#include <core/product.h>
#include <core/schemaentry.h>
#include <core/schemaentryelement.h>
#include <core/schemaentrytemplates.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QStandardPaths>

using namespace KUserFeedback::Console;

class SchemaTemplateTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(schematemplates);
        QStandardPaths::setTestModeEnabled(true);
    }

    void testTemplateLoading()
    {
        QVERIFY(SchemaEntryTemplates::availableTemplates().size() > 5);

        for (const auto &t : SchemaEntryTemplates::availableTemplates()) {
            qDebug() << t.name();
            QVERIFY(!t.name().isEmpty());
            QVERIFY(!t.schema().isEmpty());
            for (const auto &entry : t.schema()) {
                QVERIFY(!entry.name().isEmpty());
                QVERIFY(!entry.elements().isEmpty());
                for (const auto &elem : entry.elements()) {
                    QVERIFY(!elem.name().isEmpty());
                }
            }
            QVERIFY(!t.aggregations().isEmpty());
            for (const auto &aggr : t.aggregations()) {
                QVERIFY(!aggr.elements().isEmpty());
                for (const auto &elem : aggr.elements()) {
                    QVERIFY(!elem.schemaEntry().name().isEmpty());
                    if (elem.type() == AggregationElement::Value)
                        QVERIFY(!elem.schemaEntryElement().name().isEmpty());
                }
            }
        }
    }
};

QTEST_MAIN(SchemaTemplateTest)

#include "schematemplatetest.moc"

/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <console/model/numericaggregationmodel.h>
#include <console/model/datamodel.h>
#include <console/model/timeaggregationmodel.h>
#include <console/core/sample.h>
#include <console/core/schemaentrytemplates.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QStandardPaths>
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
#include <QAbstractItemModelTester>
#endif

using namespace KUserFeedback::Console;

class NumericAggregationModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(schematemplates);
        QStandardPaths::setTestModeEnabled(true);
    }

    void testEmptyModel()
    {
        NumericAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        model.setAggregation(AggregationElement());
        AggregationElement aggr;
        {
            SchemaEntry entry;
            entry.setDataType(SchemaEntry::Scalar);
            entry.setName(QLatin1String("usageTime"));
            aggr.setSchemaEntry(entry);
            SchemaEntryElement elem;
            elem.setName(QLatin1String("value"));
            aggr.setSchemaEntryElement(elem);
            aggr.setType(AggregationElement::Value);
        }
        model.setAggregation(aggr);

        TimeAggregationModel timeModel;
        model.setSourceModel(&timeModel);

        DataModel srcModel;
        timeModel.setSourceModel(&srcModel);
        srcModel.setProduct({});
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 6);

        Product p;
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTest"));
        srcModel.setProduct(p);
        QCOMPARE(model.columnCount(), 6);
        QCOMPARE(model.rowCount(), 0);
    }

    void testModelContent()
    {
        NumericAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        AggregationElement aggr;
        {
            SchemaEntry entry;
            entry.setDataType(SchemaEntry::Scalar);
            entry.setName(QLatin1String("usageTime"));
            aggr.setSchemaEntry(entry);
            SchemaEntryElement elem;
            elem.setName(QLatin1String("value"));
            aggr.setSchemaEntryElement(elem);
            aggr.setType(AggregationElement::Value);
        }
        model.setAggregation(aggr);

        TimeAggregationModel timeModel;
        model.setSourceModel(&timeModel);

        DataModel srcModel;
        timeModel.setSourceModel(&srcModel);
        timeModel.setAggregationMode(TimeAggregationModel::AggregateDay);
        Product p;
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTest"));
        srcModel.setProduct(p);

        auto samples = Sample::fromJson(R"([
            { "timestamp": "2016-11-26 12:00:00" },
            { "timestamp": "2016-11-27 12:00:00", "usageTime": { "value": "1.0" } },
            { "timestamp": "2016-11-27 12:00:00", "usageTime": { "value": "2.0" } },
            { "timestamp": "2016-11-27 12:00:00", "usageTime": { "value": "3.0" } },
            { "timestamp": "2016-11-27 12:00:00", "usageTime": { "value": "0.0" } },
            { "timestamp": "2016-11-27 12:00:00", "usageTime": { "value": "4.0" } },
            { "timestamp": "2016-11-27 12:00:00", "usageTime": { "value": "5.0" } },
            { "timestamp": "2016-11-28 12:00:00", "usageTime": { "value": "1.0" } },
            { "timestamp": "2016-11-28 12:00:00", "usageTime": { "value": "2.0" } },
            { "timestamp": "2016-11-28 12:00:00" }
        ])", p);
        QCOMPARE(samples.size(), 10);
        srcModel.setSamples(samples);

        QCOMPARE(model.columnCount(), 6);
        QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("Lower Extreme"));
        QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("Median"));
        QCOMPARE(model.headerData(5, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("Upper Extreme"));

        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-26"));
        QCOMPARE(model.index(0, 1).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(0, 3).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(0, 4).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(0, 5).data(Qt::DisplayRole).toDouble(), 0.0);

        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-27"));
        QCOMPARE(model.index(1, 1).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(1, 2).data(Qt::DisplayRole).toDouble(), 1.0);
        QCOMPARE(model.index(1, 3).data(Qt::DisplayRole).toDouble(), 3.0);
        QCOMPARE(model.index(1, 4).data(Qt::DisplayRole).toDouble(), 4.0);
        QCOMPARE(model.index(1, 5).data(Qt::DisplayRole).toDouble(), 5.0);

        QCOMPARE(model.index(2, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-28"));
        QCOMPARE(model.index(2, 1).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(2, 2).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(2, 3).data(Qt::DisplayRole).toDouble(), 1.0);
        QCOMPARE(model.index(2, 4).data(Qt::DisplayRole).toDouble(), 2.0);
        QCOMPARE(model.index(2, 5).data(Qt::DisplayRole).toDouble(), 2.0);

        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::MaximumValueRole).toDouble(), 5.0);
    }
};

QTEST_MAIN(NumericAggregationModelTest)

#include "numericaggregationmodeltest.moc"


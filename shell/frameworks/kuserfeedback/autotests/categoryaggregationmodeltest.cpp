/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <console/model/categoryaggregationmodel.h>
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

class CategoryAggregationModelTest : public QObject
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
        CategoryAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        model.setAggregation(Aggregation());
        AggregationElement aggrElem;
        {
            SchemaEntry entry;
            entry.setName(QLatin1String("applicationVersion"));
            aggrElem.setSchemaEntry(entry);
            SchemaEntryElement elem;
            elem.setName(QLatin1String("value"));
            aggrElem.setSchemaEntryElement(elem);
            aggrElem.setType(AggregationElement::Value);
        }
        Aggregation aggr;
        aggr.setType(Aggregation::Category);
        aggr.setElements({aggrElem});
        model.setAggregation(aggr);

        TimeAggregationModel timeModel;
        model.setSourceModel(&timeModel);

        DataModel srcModel;
        timeModel.setSourceModel(&srcModel);
        srcModel.setProduct({});
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 1);

        Product p;
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTest"));
        srcModel.setProduct(p);
        QCOMPARE(model.columnCount(), 1);
        QCOMPARE(model.rowCount(), 0);
    }

    void testModelContentDepth1()
    {
        CategoryAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        AggregationElement aggrElem;
        {
            SchemaEntry entry;
            entry.setName(QLatin1String("applicationVersion"));
            aggrElem.setSchemaEntry(entry);
            SchemaEntryElement elem;
            elem.setName(QLatin1String("value"));
            aggrElem.setSchemaEntryElement(elem);
            aggrElem.setType(AggregationElement::Value);
        }
        Aggregation aggr;
        aggr.setType(Aggregation::Category);
        aggr.setElements({aggrElem});
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
            { "timestamp": "2016-11-27 12:00:00", "applicationVersion": { "value": "1.0" } },
            { "timestamp": "2016-11-27 12:00:00", "applicationVersion": { "value": "1.9.84" } },
            { "timestamp": "2016-11-27 12:00:00", "applicationVersion": { "value": "1.9.84" } },
            { "timestamp": "2016-11-28 12:00:00", "applicationVersion": { "value": "1.9.84" } },
            { "timestamp": "2016-11-28 12:00:00", "applicationVersion": { "value": "1.9.84" } },
            { "timestamp": "2016-11-28 12:00:00" }
        ])", p);
        QCOMPARE(samples.size(), 6);
        srcModel.setSamples(samples);

        QCOMPARE(model.columnCount(), 4);
        QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("[empty]"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("1.0"));
        QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("1.9.84"));

        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-27"));
        QCOMPARE(model.index(0, 1).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toInt(), 1);
        QCOMPARE(model.index(0, 3).data(Qt::DisplayRole).toInt(), 2);
        QCOMPARE(model.index(0, 3).data(TimeAggregationModel::AccumulatedDisplayRole).toInt(), 3);

        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-28"));
        QCOMPARE(model.index(1, 1).data(Qt::DisplayRole).toInt(), 1);
        QCOMPARE(model.index(1, 2).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(0, 3).data(Qt::DisplayRole).toInt(), 2);
        QCOMPARE(model.index(0, 3).data(TimeAggregationModel::AccumulatedDisplayRole).toInt(), 3);

        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::MaximumValueRole).toInt(), 3);
    }

    void testModelContentDepth2()
    {
        const auto p = Product::fromJson(R"({
            "name": "depth2test",
            "schema": [{
                "name": "platform",
                "type": "scalar",
                "elements": [
                    { "name": "os", "type": "string" },
                    { "name": "version", "type": "string" }
                ]
            }],
            "aggregation": [{
                "type": "category",
                "name": "OS Details",
                "elements": [
                    { "type": "value", "schemaEntry": "platform", "schemaEntryElement": "os" },
                    { "type": "value", "schemaEntry": "platform", "schemaEntryElement": "version" }
                ]
            }]
        })").at(0);
        QVERIFY(p.isValid());
        QCOMPARE(p.aggregations().size(), 1);

        CategoryAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        model.setAggregation(p.aggregations().at(0));

        TimeAggregationModel timeModel;
        model.setSourceModel(&timeModel);

        DataModel srcModel;
        timeModel.setSourceModel(&srcModel);
        timeModel.setAggregationMode(TimeAggregationModel::AggregateDay);
        srcModel.setProduct(p);

        auto samples = Sample::fromJson(R"([
            { "timestamp": "2016-11-27 12:00:00", "platform": { "os": "windows", "version": "10" } },
            { "timestamp": "2016-11-27 12:00:00", "platform": { "os": "linux", "version": "10" } },
            { "timestamp": "2016-11-27 12:00:00", "platform": { "os": "linux", "version": "10" } },
            { "timestamp": "2016-11-28 12:00:00", "platform": { "os": "windows", "version": "10" } },
            { "timestamp": "2016-11-28 12:00:00", "platform": { "os": "linux", "version": "42" } },
            { "timestamp": "2016-11-28 12:00:00" }
        ])", p);
        QCOMPARE(samples.size(), 6);
        srcModel.setSamples(samples);

        QCOMPARE(model.columnCount(), 5);
        QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("[empty]"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("10")); // linux
        QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("42"));
        QCOMPARE(model.headerData(4, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("10")); // windows

        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-27"));
        QCOMPARE(model.index(0, 1).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toInt(), 2);
        QCOMPARE(model.index(0, 3).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(0, 4).data(Qt::DisplayRole).toInt(), 1);

        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-28"));
        QCOMPARE(model.index(1, 1).data(Qt::DisplayRole).toInt(), 1);
        QCOMPARE(model.index(1, 2).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(1, 3).data(Qt::DisplayRole).toInt(), 1);
        QCOMPARE(model.index(1, 4).data(Qt::DisplayRole).toInt(), 1);

        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::MaximumValueRole).toInt(), 3);

        model.setDepth(1);
        QCOMPARE(model.columnCount(), 4);
        QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("[empty]"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("linux"));
        QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("windows"));

        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 1).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toInt(), 2);
        QCOMPARE(model.index(0, 3).data(Qt::DisplayRole).toInt(), 1);
    }

    void testModelContentDepth3()
    {
        const auto p = Product::fromJson(R"({
            "name": "depth2test",
            "schema": [{
                "name": "opengl",
                "type": "scalar",
                "elements": [
                    { "name": "os", "type": "string" },
                    { "name": "type", "type": "string" },
                    { "name": "version", "type": "string" }
                ]
            }],
            "aggregation": [{
                "type": "category",
                "name": "OpenGL Details",
                "elements": [
                    { "type": "value", "schemaEntry": "opengl", "schemaEntryElement": "os" },
                    { "type": "value", "schemaEntry": "opengl", "schemaEntryElement": "type" },
                    { "type": "value", "schemaEntry": "opengl", "schemaEntryElement": "version" }
                ]
            }]
        })").at(0);
        QVERIFY(p.isValid());
        QCOMPARE(p.aggregations().size(), 1);

        CategoryAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        model.setAggregation(p.aggregations().at(0));

        TimeAggregationModel timeModel;
        model.setSourceModel(&timeModel);

        DataModel srcModel;
        timeModel.setSourceModel(&srcModel);
        timeModel.setAggregationMode(TimeAggregationModel::AggregateDay);
        srcModel.setProduct(p);

        auto samples = Sample::fromJson(R"([
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "windows", "type": "GL", "version": "2.0" } },
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "windows", "type": "GLES", "version": "2.0" } },
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "windows" } },
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "linux", "type": "GL", "version": "4.4" } },
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "linux", "type": "GL", "version": "3.1" } },
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "linux", "type": "GL", "version": "4.4" } },
            { "timestamp": "2016-11-27 12:00:00", "opengl": { "os": "linux", "type": "GLES", "version": "2.0" } },
            { "timestamp": "2016-11-28 12:00:00" }
        ])", p);
        QCOMPARE(samples.size(), 8);
        srcModel.setSamples(samples);

        QCOMPARE(model.columnCount(), 8);
        QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("[empty]"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("3.1")); // linux GL
        QCOMPARE(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("4.4")); // linux GL
        QCOMPARE(model.headerData(4, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("2.0")); // linux GLES
        QCOMPARE(model.headerData(5, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("[empty]")); // windows empty
        QCOMPARE(model.headerData(6, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("2.0")); // windows GL
        QCOMPARE(model.headerData(7, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("2.0")); // windows GLES

        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-27"));
        QCOMPARE(model.index(0, 1).data(Qt::DisplayRole).toInt(), 0); // empty
        QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toInt(), 1); // linux GL
        QCOMPARE(model.index(0, 3).data(Qt::DisplayRole).toInt(), 2); // linux GL
        QCOMPARE(model.index(0, 4).data(Qt::DisplayRole).toInt(), 1); // linux GLES
        QCOMPARE(model.index(0, 5).data(Qt::DisplayRole).toInt(), 1); // windows empty
        QCOMPARE(model.index(0, 6).data(Qt::DisplayRole).toInt(), 1); // windows GL
        QCOMPARE(model.index(0, 7).data(Qt::DisplayRole).toInt(), 1); // windows GLES

        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-28"));
        QCOMPARE(model.index(1, 1).data(Qt::DisplayRole).toInt(), 1);
        QCOMPARE(model.index(1, 2).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(1, 3).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(1, 4).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(1, 5).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(1, 6).data(Qt::DisplayRole).toInt(), 0);
        QCOMPARE(model.index(1, 7).data(Qt::DisplayRole).toInt(), 0);
    }
};

QTEST_MAIN(CategoryAggregationModelTest)

#include "categoryaggregationmodeltest.moc"

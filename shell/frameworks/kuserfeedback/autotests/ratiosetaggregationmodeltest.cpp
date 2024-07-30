/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <console/model/ratiosetaggregationmodel.h>
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

class RatioSetAggregationModelTest : public QObject
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
        RatioSetAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        model.setAggregationValue(QString());
        model.setAggregationValue(QLatin1String("applicationVersion.value"));

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

    void testModelContent()
    {
        RatioSetAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif
        model.setAggregationValue(QLatin1String("newPropertyRatio"));

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
            { "timestamp": "2016-11-27 12:00:00", "newPropertyRatio": { "cat1": { "property": 0.4 }, "cat2": { "property": 0.6 } } },
            { "timestamp": "2016-11-27 12:00:00", "newPropertyRatio": { "cat1": { "property": 0.3 }, "cat2": { "property": 0.7 } } },
            { "timestamp": "2016-11-27 12:00:00", "newPropertyRatio": { "cat1": { "property": 0.2 }, "cat2": { "property": 0.8 } } },
            { "timestamp": "2016-11-28 12:00:00", "newPropertyRatio": { "cat1": { "property": 1.0 } } },
            { "timestamp": "2016-11-28 12:00:00", "newPropertyRatio": { "cat1": { "property": 0.0 }, "cat2": { "property": 0.0 } } },
            { "timestamp": "2016-11-29 12:00:00" }
        ])", p);
        QCOMPARE(samples.size(), 6);
        srcModel.setSamples(samples);

        QCOMPARE(model.columnCount(), 3);
        QCOMPARE(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("cat1"));
        QCOMPARE(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), QLatin1String("cat2"));

        QCOMPARE(model.rowCount(), 3);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-27"));
        QCOMPARE(model.index(0, 1).data(Qt::DisplayRole).toDouble(), 0.3);
        QCOMPARE(model.index(0, 2).data(Qt::DisplayRole).toDouble(), 0.7);
        QCOMPARE(model.index(0, 2).data(TimeAggregationModel::AccumulatedDisplayRole).toDouble(), 1.0);

        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-28"));
        QCOMPARE(model.index(1, 1).data(Qt::DisplayRole).toDouble(), 1.0);
        QCOMPARE(model.index(1, 2).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(1, 2).data(TimeAggregationModel::AccumulatedDisplayRole).toDouble(), 1.0);

        QCOMPARE(model.index(2, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-29"));
        QCOMPARE(model.index(2, 1).data(Qt::DisplayRole).toDouble(), 0.0);
        QCOMPARE(model.index(2, 2).data(Qt::DisplayRole).toDouble(), 0.0);

        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::MaximumValueRole).toDouble(), 1.0);
    }
};

QTEST_MAIN(RatioSetAggregationModelTest)

#include "ratiosetaggregationmodeltest.moc"


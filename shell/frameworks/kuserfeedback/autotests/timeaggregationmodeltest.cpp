/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <console/core/sample.h>
#include <console/core/schemaentrytemplates.h>
#include <console/model/datamodel.h>
#include <console/model/timeaggregationmodel.h>

#include <QDebug>
#include <QtTest/qtest.h>
#include <QObject>
#include <QStandardPaths>
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
#include <QAbstractItemModelTester>
#endif

using namespace KUserFeedback::Console;

class TimeAggregationModelTest : public QObject
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
        TimeAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif

        DataModel srcModel;
        model.setSourceModel(&srcModel);
        srcModel.setProduct({});
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.columnCount(), 2);

        Product p;
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTest"));
        srcModel.setProduct(p);
        QCOMPARE(model.columnCount(), 2);
        QCOMPARE(model.rowCount(), 0);
    }

    void testModelContent()
    {
        TimeAggregationModel model;
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        QAbstractItemModelTester modelTest(&model);
#endif

        DataModel srcModel;
        model.setSourceModel(&srcModel);
        Product p;
        for (const auto &tpl : SchemaEntryTemplates::availableTemplates())
            p.addTemplate(tpl);
        p.setName(QStringLiteral("org.kde.UserFeedback.UnitTest"));
        srcModel.setProduct(p);

        auto samples = Sample::fromJson(R"([{
            "id": 42,
            "timestamp": "2016-11-27 16:09:06",
            "platform": { "os": "linux", "version": "suse" },
            "applicationVersion": { "value": "1.9.84" },
            "screens": [ { "width": 1920, "height": 1200 }, { "width": 640, "height": 480 } ],
            "newPropertyRatio": { "value1": { "property": 0.55 }, "value2": { "property": 0.45 } }
        }, {
            "id": 43,
            "timestamp": "2016-12-09 12:00:00",
            "platform": { "os": "linux", "version": "suse" },
            "applicationVersion": { "value": "1.9.84" },
            "screens": [ { "width": 1920, "height": 1200 } ],
            "newPropertyRatio": { "value1": { "property": 0.1 }, "value2": { "property": 0.9 } }
        }])", p);
        QCOMPARE(samples.size(), 2);
        srcModel.setSamples(samples);

        model.setAggregationMode(TimeAggregationModel::AggregateYear);
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016"));
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::DateTimeRole).toDate(), QDate(2016, 01, 01));
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::SamplesRole).value<QVector<Sample>>().size(), 2);
        QCOMPARE(model.index(0, 1).data().toInt(), 2);

        model.setAggregationMode(TimeAggregationModel::AggregateMonth);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11"));
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::DateTimeRole).toDate(), QDate(2016, 11, 01));
        QCOMPARE(model.index(0, 1).data().toInt(), 1);
        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-12"));
        QCOMPARE(model.index(1, 1).data().toInt(), 1);

        model.setAggregationMode(TimeAggregationModel::AggregateWeek);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-w47"));
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::DateTimeRole).toDate(), QDate(2016, 11, 21));
        QCOMPARE(model.index(0, 1).data().toInt(), 1);
        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-w49"));
        QCOMPARE(model.index(1, 1).data().toInt(), 1);

        model.setAggregationMode(TimeAggregationModel::AggregateDay);
        QCOMPARE(model.rowCount(), 2);
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-11-27"));
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::DateTimeRole).toDate(), QDate(2016, 11, 27));
        QCOMPARE(model.index(0, 1).data().toInt(), 1);
        QCOMPARE(model.index(1, 0).data(TimeAggregationModel::TimeDisplayRole).toString(), QLatin1String("2016-12-09"));
        QCOMPARE(model.index(0, 0).data(TimeAggregationModel::SamplesRole).value<QVector<Sample>>().size(), 1);
        QCOMPARE(model.index(1, 1).data().toInt(), 1);
    }
};

QTEST_MAIN(TimeAggregationModelTest)

#include "timeaggregationmodeltest.moc"

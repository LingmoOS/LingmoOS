/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCalendarCore/CalendarListModel>
#include <KCalendarCore/CalendarPluginLoader>

#include <QAbstractItemModelTester>
#include <QDebug>
#include <QTest>

using namespace KCalendarCore;

class CalendarListModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testModel()
    {
        CalendarListModel model;
        QAbstractItemModelTester modelTest(&model);

        if (CalendarPluginLoader::hasPlugin()) {
            QCOMPARE(model.rowCount(), CalendarPluginLoader::plugin()->calendars().size());

            for (auto i = 0; i < model.rowCount(); ++i) {
                auto idx = model.index(i, 0);
                QVERIFY(idx.data(CalendarListModel::CalendarRole).value<KCalendarCore::Calendar *>());
                QVERIFY(!idx.data(CalendarListModel::NameRole).toString().isEmpty());
                QVERIFY(!idx.data(CalendarListModel::IdRole).toString().isEmpty());
            }

        } else {
            QCOMPARE(model.rowCount(), 0);
        }
    }
};

QTEST_MAIN(CalendarListModelTest)

#include "calendarlistmodeltest.moc"

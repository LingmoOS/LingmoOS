/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include "test_state_config.h"
#include <QTest>

class TestStateConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testStateConfig()
    {
        auto stateConfig = KSharedConfig::openStateConfig(QStringLiteral("test_statedatarc"));

        // Clean the group at every start
        stateConfig->deleteGroup(QStringLiteral("General"));
        stateConfig->sync();

        // It should have the default value
        QCOMPARE(MyStateConfig().someStateData(), 0);

        // the updated value should be read from the generated config class
        stateConfig->group(QStringLiteral("General")).writeEntry("SomeStateData", 1);
        QCOMPARE(MyStateConfig().someStateData(), 1);

        // Make sure writing the value works as expected
        MyStateConfig cfg;
        cfg.setSomeStateData(2);
        QVERIFY(cfg.isSaveNeeded());
        cfg.save();
        stateConfig->reparseConfiguration();
        QCOMPARE(stateConfig->group(QStringLiteral("General")).readEntry("SomeStateData", -1), 2);
    }
};
QTEST_MAIN(TestStateConfig)

#include "test_state_config_main.moc"

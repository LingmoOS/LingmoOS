/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define QT_GUI_LIB

#include <QObject>
#include <QtTest>

#include "../src/config.h"
#include "../src/getconfigoperation.h"
#include "../src/output.h"

using namespace KScreen;

class testXRandR : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void singleOutput();

private:
    QProcess m_process;
};

void testXRandR::initTestCase()
{
}

void testXRandR::singleOutput()
{
    qputenv("KSCREEN_BACKEND", "XRandR");

    GetConfigOperation *op = new GetConfigOperation();
    QVERIFY(op->exec());

    const ConfigPtr config = op->config();
    if (!config) {
        QSKIP("XRandR X extension is not available", SkipAll);
    }

    QCOMPARE(config->outputs().count(), 1);

    const OutputPtr output = config->outputs().take(327);

    QCOMPARE(output->name(), QString("default"));
    QCOMPARE(output->type(), Output::Unknown);
    QCOMPARE(output->modes().count(), 15);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->currentModeId(), QLatin1String("338"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), false);
    QVERIFY2(output->clones().isEmpty(), "In singleOutput is impossible to have clones");
}

QTEST_MAIN(testXRandR)

#include "testxrandr.moc"

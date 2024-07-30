/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "../src/backendmanager_p.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/getconfigoperation.h"
#include "../src/mode.h"
#include "../src/output.h"

using namespace KScreen;

class TestModeListChange : public QObject
{
    Q_OBJECT

private:
    KScreen::ConfigPtr getConfig();
    KScreen::ModeList createModeList();
    bool compareModeList(KScreen::ModeList before, KScreen::ModeList &after);

    QSize s0 = QSize(1920, 1080);
    QSize s1 = QSize(1600, 1200);
    QSize s2 = QSize(1280, 1024);
    QSize s3 = QSize(800, 600);
    QSize snew = QSize(777, 888);
    QString idnew = QStringLiteral("666");

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void modeListChange();
};

ConfigPtr TestModeListChange::getConfig()
{
    qputenv("KSCREEN_BACKEND_INPROCESS", "1");
    auto *op = new GetConfigOperation();
    if (!op->exec()) {
        qWarning("ConfigOperation error: %s", qPrintable(op->errorString()));
        BackendManager::instance()->shutdownBackend();
        return ConfigPtr();
    }

    BackendManager::instance()->shutdownBackend();

    return op->config();
}

KScreen::ModeList TestModeListChange::createModeList()
{
    KScreen::ModeList newmodes;
    {
        QString _id = QString::number(11);
        KScreen::ModePtr kscreenMode(new KScreen::Mode);
        kscreenMode->setId(_id);
        kscreenMode->setName(_id);
        kscreenMode->setSize(s0);
        kscreenMode->setRefreshRate(60);
        newmodes.insert(_id, kscreenMode);
    }
    {
        QString _id = QString::number(22);
        KScreen::ModePtr kscreenMode(new KScreen::Mode);
        kscreenMode->setId(_id);
        kscreenMode->setName(_id);
        kscreenMode->setSize(s1);
        kscreenMode->setRefreshRate(60);
        newmodes.insert(_id, kscreenMode);
    }
    {
        QString _id = QString::number(33);
        KScreen::ModePtr kscreenMode(new KScreen::Mode);
        kscreenMode->setId(_id);
        kscreenMode->setName(_id);
        kscreenMode->setSize(s2);
        kscreenMode->setRefreshRate(60);
        newmodes.insert(_id, kscreenMode);
    }
    return newmodes;
}

void TestModeListChange::initTestCase()
{
    qputenv("KSCREEN_LOGGING", "false");
    qputenv("KSCREEN_BACKEND", "Fake");
}

void TestModeListChange::cleanupTestCase()
{
    BackendManager::instance()->shutdownBackend();
}

void TestModeListChange::modeListChange()
{
    // json file for the fake backend
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "singleoutput.json"}});

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());

    auto output = config->outputs().first();
    QVERIFY(!output.isNull());
    auto modelist = output->modes();

    auto mode = modelist.first();
    mode->setId(QStringLiteral("44"));
    mode->setSize(QSize(880, 440));
    output->setModes(modelist);

    QCOMPARE(output->modes().first()->id(), QStringLiteral("44"));
    QCOMPARE(output->modes().first()->size(), QSize(880, 440));
    QVERIFY(!modelist.isEmpty());

    ConfigMonitor::instance()->addConfig(config);
    QSignalSpy outputChangedSpy(output.data(), &Output::outputChanged);
    QVERIFY(outputChangedSpy.isValid());
    QSignalSpy modesChangedSpy(output.data(), &Output::modesChanged);
    QVERIFY(modesChangedSpy.isValid());

    auto before = createModeList();
    output->setModes(before);
    QCOMPARE(modesChangedSpy.count(), 1);
    output->setModes(before);
    QCOMPARE(modesChangedSpy.count(), 1);
    output->setModes(before);
    QCOMPARE(modesChangedSpy.count(), 1);
    QCOMPARE(output->modes().first()->size(), s0);
    QCOMPARE(output->modes().first()->id(), QStringLiteral("11"));

    auto after = createModeList();
    auto firstmode = after.first();
    QVERIFY(!firstmode.isNull());
    QCOMPARE(firstmode->size(), s0);
    QCOMPARE(firstmode->id(), QStringLiteral("11"));
    firstmode->setSize(snew);
    firstmode->setId(idnew);
    output->setModes(after);
    QCOMPARE(modesChangedSpy.count(), 2);

    QString _id = QString::number(11);
    KScreen::ModePtr kscreenMode(new KScreen::Mode);
    kscreenMode->setId(_id);
    kscreenMode->setName(_id);
    kscreenMode->setSize(s0);
    kscreenMode->setRefreshRate(60);
    before.insert(_id, kscreenMode);
    output->setModes(before);
    QCOMPARE(modesChangedSpy.count(), 3);
    QCOMPARE(outputChangedSpy.count(), modesChangedSpy.count());
}

QTEST_MAIN(TestModeListChange)

#include "testmodelistchange.moc"

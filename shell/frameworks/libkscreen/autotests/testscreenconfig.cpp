/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QObject>
#include <QTest>

#include "../src/backendmanager_p.h"
#include "../src/config.h"
#include "../src/getconfigoperation.h"
#include "../src/mode.h"
#include "../src/output.h"
#include "../src/screen.h"
#include "../src/setconfigoperation.h"

using namespace KScreen;

class testScreenConfig : public QObject
{
    Q_OBJECT

private:
    KScreen::ConfigPtr getConfig();

private Q_SLOTS:
    void initTestCase();
    void singleOutput();
    void singleOutputWithoutPreferred();
    void multiOutput();
    void clonesOutput();
    void configCanBeApplied();
    void supportedFeatures();
    void testInvalidMode();
    void cleanupTestCase();
    void testOutputPositionNormalization();
};

ConfigPtr testScreenConfig::getConfig()
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

void testScreenConfig::initTestCase()
{
    qputenv("KSCREEN_LOGGING", "false");
    qputenv("KSCREEN_BACKEND", "Fake");
}

void testScreenConfig::cleanupTestCase()
{
    BackendManager::instance()->shutdownBackend();
}

void testScreenConfig::singleOutput()
{
    // json file for the fake backend
    KScreen::BackendManager::instance()->setBackendArgs(
        {{QStringLiteral("TEST_DATA"), TEST_DATA "singleoutput.json"},
         {QStringLiteral("SUPPORTED_FEATURES"), QVariant::fromValue(KScreen::Config::Feature::PerOutputScaling)}});

    //     QVERIFY2(kscreen, KScreen::errorString().toLatin1());

    //     QVERIFY2(!kscreen->backend().isEmpty(), "No backend loaded");

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const ScreenPtr screen = config->screen();
    QVERIFY(!screen.isNull());

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(1280, 800));

    QCOMPARE(config->outputs().count(), 1);

    const OutputPtr output = config->outputs().take(1);
    QVERIFY(!output.isNull());

    QCOMPARE(output->name(), QLatin1String("LVDS1"));
    QCOMPARE(output->type(), Output::Panel);
    QCOMPARE(output->modes().count(), 3);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->geometry(), QRect(0, 0, 1280, 800));
    QCOMPARE(output->currentModeId(), QLatin1String("3"));
    QCOMPARE(output->preferredModeId(), QLatin1String("3"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->scale(), 1.0);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);
    // QCOMPARE(output->isEmbedded(), true);
    QVERIFY2(output->clones().isEmpty(), "In singleOutput is impossible to have clones");

    const ModePtr mode = output->currentMode();
    QCOMPARE(mode->size(), QSize(1280, 800));
    QCOMPARE(mode->refreshRate(), (float)59.9);
}

void testScreenConfig::singleOutputWithoutPreferred()
{
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "singleOutputWithoutPreferred.json"}});

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const OutputPtr output = config->outputs().take(1);
    QVERIFY(!output.isNull());

    QVERIFY(output->preferredModes().isEmpty());
    QCOMPARE(output->preferredModeId(), QLatin1String("3"));
}

void testScreenConfig::multiOutput()
{
    KScreen::BackendManager::instance()->setBackendArgs(
        {{QStringLiteral("TEST_DATA"), TEST_DATA "multipleoutput.json"},
         {QStringLiteral("SUPPORTED_FEATURES"), QVariant::fromValue(KScreen::Config::Feature::PerOutputScaling)}});

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const ScreenPtr screen = config->screen();
    QVERIFY(!screen.isNull());

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(3200, 1880));

    QCOMPARE(config->outputs().count(), 2);

    const OutputPtr output = config->outputs().take(2);
    QVERIFY(!output.isNull());

    QCOMPARE(output->name(), QStringLiteral("HDMI1"));
    QCOMPARE(output->type(), Output::HDMI);
    QCOMPARE(output->modes().count(), 4);
    QCOMPARE(output->pos(), QPoint(1280, 0));
    QCOMPARE(output->geometry(), QRect(1280, 0, std::ceil(1920 / 1.4), std::ceil(1080 / 1.4)));
    QCOMPARE(output->currentModeId(), QLatin1String("4"));
    QCOMPARE(output->preferredModeId(), QLatin1String("4"));
    QCOMPARE(output->rotation(), Output::None);
    QCOMPARE(output->scale(), 1.4);
    QCOMPARE(output->isConnected(), true);
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), false);
    QVERIFY2(output->clones().isEmpty(), "This simulates extended output, no clones");

    const ModePtr mode = output->currentMode();
    QVERIFY(!mode.isNull());
    QCOMPARE(mode->size(), QSize(1920, 1080));
    QCOMPARE(mode->refreshRate(), (float)60.0);
}

void testScreenConfig::clonesOutput()
{
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "multipleclone.json"}});

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    const ScreenPtr screen = config->screen();
    QVERIFY(!screen.isNull());

    QCOMPARE(screen->minSize(), QSize(320, 200));
    QCOMPARE(screen->maxSize(), QSize(8192, 8192));
    QCOMPARE(screen->currentSize(), QSize(1024, 768));

    const OutputPtr one = config->outputs()[1];
    const OutputPtr two = config->outputs()[2];

    QCOMPARE(one->currentMode()->size(), two->currentMode()->size());
    QCOMPARE(one->clones().count(), 1);
    QCOMPARE(one->clones().first(), two->id());
    QVERIFY2(two->clones().isEmpty(), "Output two should have no clones");
}

void testScreenConfig::configCanBeApplied()
{
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "singleoutputBroken.json"}});

    const ConfigPtr brokenConfig = getConfig();

    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "singleoutput.json"}});

    const ConfigPtr currentConfig = getConfig();
    QVERIFY(!currentConfig.isNull());
    const OutputPtr primaryBroken = brokenConfig->outputs()[2];
    QVERIFY(!primaryBroken.isNull());
    const OutputPtr currentPrimary = currentConfig->outputs()[1];
    QVERIFY(!currentPrimary.isNull());

    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setId(currentPrimary->id());
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setConnected(currentPrimary->isConnected());
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setCurrentModeId(QStringLiteral("42"));
    QVERIFY(!Config::canBeApplied(brokenConfig));
    primaryBroken->setCurrentModeId(currentPrimary->currentModeId());
    QVERIFY(!Config::canBeApplied(brokenConfig));
    qDebug() << "brokenConfig.modes" << primaryBroken->mode(QStringLiteral("3"));
    primaryBroken->mode(QStringLiteral("3"))->setSize(QSize(1280, 800));
    qDebug() << "brokenConfig.modes" << primaryBroken->mode(QStringLiteral("3"));
    QVERIFY(Config::canBeApplied(brokenConfig));

    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "tooManyOutputs.json"}});

    const ConfigPtr brokenConfig2 = getConfig();
    QVERIFY(!brokenConfig2.isNull());

    int enabledOutputsCount = 0;
    for (const OutputPtr &output : brokenConfig2->outputs()) {
        if (output->isEnabled()) {
            ++enabledOutputsCount;
        }
    }
    QVERIFY(brokenConfig2->screen()->maxActiveOutputsCount() < enabledOutputsCount);
    QVERIFY(!Config::canBeApplied(brokenConfig2));

    const ConfigPtr nulllConfig;
    QVERIFY(!Config::canBeApplied(nulllConfig));
}

void testScreenConfig::supportedFeatures()
{
    ConfigPtr config = getConfig();

    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::None));
    QVERIFY(!config->supportedFeatures().testFlag(KScreen::Config::Feature::Writable));
    QVERIFY(!config->supportedFeatures().testFlag(KScreen::Config::Feature::PrimaryDisplay));
    QVERIFY(!config->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));

    config->setSupportedFeatures(KScreen::Config::Feature::Writable | KScreen::Config::Feature::PrimaryDisplay);
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::Writable));
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::PrimaryDisplay));

    config->setSupportedFeatures(KScreen::Config::Feature::None);
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::None));

    config->setSupportedFeatures(KScreen::Config::Feature::PerOutputScaling | KScreen::Config::Feature::Writable);
    QVERIFY(!config->supportedFeatures().testFlag(KScreen::Config::Feature::None));
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::Writable));
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));

    config->setSupportedFeatures(KScreen::Config::Feature::PerOutputScaling | KScreen::Config::Feature::Writable | KScreen::Config::Feature::PrimaryDisplay);
    QVERIFY(!config->supportedFeatures().testFlag(KScreen::Config::Feature::None));
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::Writable));
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::PrimaryDisplay));
    QVERIFY(config->supportedFeatures().testFlag(KScreen::Config::Feature::PerOutputScaling));
}

void testScreenConfig::testInvalidMode()
{
    ModeList modes;
    ModePtr invalidMode = modes.value(QStringLiteral("99"));
    QVERIFY(invalidMode.isNull());

    auto output = new KScreen::Output();
    auto currentMode = output->currentMode();
    QVERIFY(currentMode.isNull());
    QVERIFY(!currentMode);
    delete output;
}

void testScreenConfig::testOutputPositionNormalization()
{
    KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "multipleoutput.json"}});

    const ConfigPtr config = getConfig();
    QVERIFY(!config.isNull());
    auto left = config->outputs().first();
    auto right = config->outputs().last();
    QVERIFY(!left.isNull());
    QVERIFY(!right.isNull());
    left->setPos(QPoint(-5000, 700));
    right->setPos(QPoint(-3720, 666));
    QCOMPARE(left->pos(), QPoint(-5000, 700));
    QCOMPARE(right->pos(), QPoint(-3720, 666));

    // start a set operation to fix up the positions
    {
        auto setop = new SetConfigOperation(config);
        setop->exec();
    }
    QCOMPARE(left->pos(), QPoint(0, 34));
    QCOMPARE(right->pos(), QPoint(1280, 0));

    // make sure it doesn't touch a valid config
    {
        auto setop = new SetConfigOperation(config);
        setop->exec();
    }
    QCOMPARE(left->pos(), QPoint(0, 34));
    QCOMPARE(right->pos(), QPoint(1280, 0));

    // positions of single outputs should be at 0, 0
    left->setEnabled(false);
    {
        auto setop = new SetConfigOperation(config);
        setop->exec();
    }
    QCOMPARE(right->pos(), QPoint());
}

QTEST_MAIN(testScreenConfig)

#include "testscreenconfig.moc"

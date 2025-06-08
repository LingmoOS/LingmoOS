/*
    SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "../../kded/config.h"
#include "../../common/globals.h"

#include <QObject>
#include <QtTest>

#include <KScreen/Config>
#include <KScreen/EDID>
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/Screen>

#include <memory>

class TestConfig : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testSimpleConfig();
    void testTwoScreenConfig();
    void testRotatedScreenConfig();
    void testDisabledScreenConfig();
    void testConfig404();
    void testCorruptConfig();
    void testCorruptEmptyConfig();
    void testCorruptUselessConfig();
    void testNullConfig();
    void testIdenticalOutputs();
    void testMoveConfig();
    void testFixedConfig();

private:
    QTemporaryDir m_temporaryDir;
    std::unique_ptr<Config> createConfig(bool output1Connected, bool output2Conected);
};

std::unique_ptr<Config> TestConfig::createConfig(bool output1Connected, bool output2Connected)
{
    KScreen::ScreenPtr screen = KScreen::ScreenPtr::create();
    screen->setCurrentSize(QSize(1920, 1080));
    screen->setMaxSize(QSize(32768, 32768));
    screen->setMinSize(QSize(8, 8));

    QList<QSize> sizes({QSize(320, 240), QSize(640, 480), QSize(1024, 768), QSize(1280, 1024), QSize(1920, 1280)});
    KScreen::ModeList modes;
    for (int i = 0; i < sizes.count(); ++i) {
        const QSize &size = sizes[i];
        KScreen::ModePtr mode = KScreen::ModePtr::create();
        mode->setId(QStringLiteral("MODE-%1").arg(i));
        mode->setName(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()));
        mode->setSize(size);
        mode->setRefreshRate(60.0);
        modes.insert(mode->id(), mode);
    }

    KScreen::OutputPtr output1 = KScreen::OutputPtr::create();
    output1->setId(1);
    output1->setName(QStringLiteral("OUTPUT-1"));
    output1->setPos(QPoint(0, 0));
    output1->setConnected(output1Connected);
    output1->setEnabled(output1Connected);
    if (output1Connected) {
        output1->setModes(modes);
    }

    KScreen::OutputPtr output2 = KScreen::OutputPtr::create();
    output2->setId(2);
    output2->setName(QStringLiteral("OUTPUT-2"));
    output2->setPos(QPoint(0, 0));
    output2->setConnected(output2Connected);
    if (output2Connected) {
        output2->setModes(modes);
    }

    KScreen::ConfigPtr config = KScreen::ConfigPtr::create();
    config->setScreen(screen);
    config->addOutput(output1);
    config->addOutput(output2);

    auto configWrapper = std::unique_ptr<Config>(new Config(config));

    return configWrapper;
}

void TestConfig::initTestCase()
{
    qputenv("XDG_DATA_HOME", m_temporaryDir.path().toUtf8());
    QFile::link(QStringLiteral(TEST_DATA "serializerdata"), Config::configsDirPath().chopped(1));
    qputenv("KSCREEN_LOGGING", "false");
}

void TestConfig::testSimpleConfig()
{
    auto configWrapper = createConfig(true, false);
    configWrapper = configWrapper->readFile(QStringLiteral("simpleConfig.json"));

    auto config = configWrapper->data();
    QVERIFY(config);
    QCOMPARE(config->connectedOutputs().count(), 1);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(1920, 1280));
}

void TestConfig::testTwoScreenConfig()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("twoScreenConfig.json"));

    auto config = configWrapper->data();
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    output = config->connectedOutputs().last();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-3"));
    QCOMPARE(output->currentMode()->size(), QSize(1280, 1024));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(1920, 0));
    QCOMPARE(output->isPrimary(), false);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(3200, 1280));
}

void TestConfig::testRotatedScreenConfig()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("rotatedScreenConfig.json"));

    auto config = configWrapper->data();
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    output = config->connectedOutputs().last();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-3"));
    QCOMPARE(output->currentMode()->size(), QSize(1280, 1024));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::Left);
    QCOMPARE(output->pos(), QPoint(1920, 0));
    QCOMPARE(output->isPrimary(), false);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(2944, 1280));
}

void TestConfig::testDisabledScreenConfig()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("disabledScreenConfig.json"));

    auto config = configWrapper->data();
    QVERIFY(config);

    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->currentModeId(), QLatin1String("MODE-4"));
    QCOMPARE(output->currentMode()->size(), QSize(1920, 1280));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->rotation(), KScreen::Output::None);
    QCOMPARE(output->pos(), QPoint(0, 0));
    QCOMPARE(output->isPrimary(), true);

    output = config->connectedOutputs().last();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output->isEnabled(), false);

    auto screen = config->screen();
    QCOMPARE(screen->currentSize(), QSize(1920, 1280));
}

void TestConfig::testConfig404()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("filenotfoundConfig.json"));

    QVERIFY(!configWrapper);
}

void TestConfig::testCorruptConfig()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("corruptConfig.json"));
    auto config = configWrapper->data();

    QVERIFY(config);
    QCOMPARE(config->outputs().count(), 2);
    QVERIFY(config->isValid());
}

void TestConfig::testCorruptEmptyConfig()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("corruptEmptyConfig.json"));
    auto config = configWrapper->data();

    QVERIFY(config);
    QCOMPARE(config->outputs().count(), 2);
    QVERIFY(config->isValid());
}

void TestConfig::testCorruptUselessConfig()
{
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("corruptUselessConfig.json"));
    auto config = configWrapper->data();

    QVERIFY(config);
    QCOMPARE(config->outputs().count(), 2);
    QVERIFY(config->isValid());
}

void TestConfig::testNullConfig()
{
    QSKIP("crashes");
    Config nullConfig(nullptr);
    QVERIFY(!nullConfig.data());

    // Null configs have empty configIds
    QVERIFY(nullConfig.id().isEmpty());

    // Load config from a file not found results in a nullptr
    auto config = createConfig(true, true);
    QVERIFY(!config->readFile(QString()));

    // Wrong config file name should fail to save
    QVERIFY(!config->writeFile(QString()));
}

void TestConfig::testIdenticalOutputs()
{
    // Test configuration of a video wall with 6 identical outputs connected
    // this is the autotest for https://bugs.kde.org/show_bug.cgi?id=325277
    KScreen::ScreenPtr screen = KScreen::ScreenPtr::create();
    screen->setCurrentSize(QSize(1920, 1080));
    screen->setMaxSize(QSize(32768, 32768));
    screen->setMinSize(QSize(8, 8));

    QList<QSize> sizes({QSize(640, 480), QSize(1024, 768), QSize(1920, 1080), QSize(1280, 1024), QSize(1920, 1280)});
    KScreen::ModeList modes;
    for (int i = 0; i < sizes.count(); ++i) {
        const QSize &size = sizes[i];
        KScreen::ModePtr mode = KScreen::ModePtr::create();
        mode->setId(QStringLiteral("MODE-%1").arg(i));
        mode->setName(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()));
        mode->setSize(size);
        mode->setRefreshRate(60.0);
        modes.insert(mode->id(), mode);
    }
    // This one is important, the output id in the config file is a hash of it
    QByteArray data = QByteArray::fromBase64(
        "AP///////wAQrBbwTExLQQ4WAQOANCB46h7Frk80sSYOUFSlSwCBgKlA0QBxTwEBAQEBAQEBKDyAoHCwI0AwIDYABkQhAAAaAAAA/wBGNTI1TTI0NUFLTEwKAAAA/ABERUxMIFUyNDEwCiAgAAAA/"
        "QA4TB5REQAKICAgICAgAToCAynxUJAFBAMCBxYBHxITFCAVEQYjCQcHZwMMABAAOC2DAQAA4wUDAQI6gBhxOC1AWCxFAAZEIQAAHgEdgBhxHBYgWCwlAAZEIQAAngEdAHJR0B4gbihVAAZEIQAAHow"
        "K0Iog4C0QED6WAAZEIQAAGAAAAAAAAAAAAAAAAAAAPg==");

    // When setting up the outputs, make sure they're not added in alphabetical order
    // or in the same order of the config file, as that makes the tests accidentally pass

    KScreen::OutputPtr output1 = KScreen::OutputPtr::create();
    output1->setId(1);
    output1->setEdid(data);
    output1->setName(QStringLiteral("DisplayPort-0"));
    output1->setPos(QPoint(0, 0));
    output1->setConnected(true);
    output1->setEnabled(false);
    output1->setModes(modes);

    KScreen::OutputPtr output2 = KScreen::OutputPtr::create();
    output2->setId(2);
    output2->setEdid(data);
    output2->setName(QStringLiteral("DisplayPort-1"));
    output2->setPos(QPoint(0, 0));
    output2->setConnected(true);
    output2->setEnabled(false);
    output2->setModes(modes);

    KScreen::OutputPtr output3 = KScreen::OutputPtr::create();
    output3->setId(3);
    output3->setEdid(data);
    output3->setName(QStringLiteral("DisplayPort-2"));
    output3->setPos(QPoint(0, 0));
    output3->setConnected(true);
    output3->setEnabled(false);
    output3->setModes(modes);

    KScreen::OutputPtr output6 = KScreen::OutputPtr::create();
    output6->setId(6);
    output6->setEdid(data);
    output6->setName(QStringLiteral("DVI-0"));
    output6->setPos(QPoint(0, 0));
    output6->setConnected(true);
    output6->setEnabled(false);
    output6->setModes(modes);

    KScreen::OutputPtr output4 = KScreen::OutputPtr::create();
    output4->setId(4);
    output4->setEdid(data);
    output4->setName(QStringLiteral("DisplayPort-3"));
    output4->setPos(QPoint(0, 0));
    output4->setConnected(true);
    output4->setEnabled(false);
    output4->setModes(modes);

    KScreen::OutputPtr output5 = KScreen::OutputPtr::create();
    output5->setId(5);
    output5->setEdid(data);
    output5->setName(QStringLiteral("DVI-1"));
    output5->setPos(QPoint(0, 0));
    output5->setConnected(true);
    output5->setEnabled(false);
    output5->setModes(modes);

    KScreen::ConfigPtr config = KScreen::ConfigPtr::create();
    config->setScreen(screen);
    config->addOutput(output6);
    config->addOutput(output2);
    config->addOutput(output5);
    config->addOutput(output4);
    config->addOutput(output3);
    config->addOutput(output1);

    Config configWrapper(config);

    QHash<QString, QPoint> positions;
    positions[QStringLiteral("DisplayPort-0")] = QPoint(0, 1080);
    positions[QStringLiteral("DisplayPort-1")] = QPoint(2100, 30);
    positions[QStringLiteral("DisplayPort-2")] = QPoint(2100, 1080);
    positions[QStringLiteral("DisplayPort-3")] = QPoint(4020, 0);
    positions[QStringLiteral("DVI-0")] = QPoint(4020, 1080);
    positions[QStringLiteral("DVI-1")] = QPoint(0, 0);

    auto configWrapper2 = configWrapper.readFile(QStringLiteral("outputgrid_2x3.json"));
    KScreen::ConfigPtr config2 = configWrapper2->data();
    QVERIFY(config2);
    QVERIFY(config != config2);

    QCOMPARE(config2->connectedOutputs().count(), 6);
    Q_FOREACH (auto output, config2->connectedOutputs()) {
        QVERIFY(positions.keys().contains(output->name()));
        QVERIFY(output->name() != output->hash());
        QCOMPARE(positions[output->name()], output->pos());
        QCOMPARE(output->currentMode()->size(), QSize(1920, 1080));
        QCOMPARE(output->currentMode()->refreshRate(), 60.0);
        QVERIFY(output->isEnabled());
    }
    QCOMPARE(config2->screen()->currentSize(), QSize(5940, 2160));
}

void TestConfig::testMoveConfig()
{
    QSKIP("crashes");
    // Test if restoring a config using Serializer::moveConfig(src, dest) works
    // https://bugs.kde.org/show_bug.cgi?id=353029

    // Load a dualhead config
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("twoScreenConfig.json"));

    auto config = configWrapper->data();
    QVERIFY(config);

    // Make sure we don't write into TEST_DATA
    QStandardPaths::setTestModeEnabled(true);
    // TODO: this needs setup of the control directory

    // Basic assumptions for the remainder of our tests, this is the situation where the lid is opened
    QCOMPARE(config->connectedOutputs().count(), 2);

    auto output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);

    auto output2 = config->connectedOutputs().last();
    QCOMPARE(output2->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output2->isEnabled(), true);
    QCOMPARE(output2->isPrimary(), false);

    // we fake the lid being closed, first save our current config to _lidOpened
    configWrapper->writeOpenLidFile();

    // ... then switch off the panel, set primary to the other output
    output->setEnabled(false);
    config->setPrimaryOutput(output2);

    // save config as the current one, this is the config we don't want restored, and which we'll overwrite
    configWrapper->writeFile();

    QCOMPARE(output->isEnabled(), false);
    QCOMPARE(output->isPrimary(), false);
    QCOMPARE(output2->isPrimary(), true);

    // Check if both files exist
    const QString closedPath = Config::configsDirPath() % configWrapper->id();
    const QString openedPath = closedPath % QStringLiteral("_lidOpened");

    QFile openCfg(openedPath);
    QFile closedCfg(closedPath);
    QVERIFY(openCfg.exists());
    QVERIFY(closedCfg.exists());

    // Switcheroolooloo...
    configWrapper = configWrapper->readOpenLidFile();
    QVERIFY(configWrapper);

    // Check actual files, src should be gone, dest must exist
    QVERIFY(!openCfg.exists());
    QVERIFY(closedCfg.exists());

    // Make sure the laptop panel is enabled and primary again
    config = configWrapper->data();

    output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);

    output2 = config->connectedOutputs().last();
    QCOMPARE(output2->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output2->isEnabled(), true);
    QCOMPARE(output2->isPrimary(), false);

    // Make sure we don't screw up when there's no _lidOpened config
    configWrapper = configWrapper->readOpenLidFile();
    config = configWrapper->data();

    output = config->connectedOutputs().first();
    QCOMPARE(output->name(), QLatin1String("OUTPUT-1"));
    QCOMPARE(output->isEnabled(), true);
    QCOMPARE(output->isPrimary(), true);

    output2 = config->connectedOutputs().last();
    QCOMPARE(output2->name(), QLatin1String("OUTPUT-2"));
    QCOMPARE(output2->isEnabled(), true);
    QCOMPARE(output2->isPrimary(), false);
}

void TestConfig::testFixedConfig()
{
    // Load a dualhead config
    auto configWrapper = createConfig(true, true);
    configWrapper = configWrapper->readFile(QStringLiteral("twoScreenConfig.json"));
    auto config = configWrapper->data();
    QVERIFY(config);

    // TODO: this needs setup of the control directory

    const QString fixedCfgPath = Config::configsDirPath() % Config::s_fixedConfigFileName;
    // save config as the current one, this is the config we don't want restored, and which we'll overwrite
    configWrapper->writeFile(fixedCfgPath);

    // Check if both files exist
    QFile fixedCfg(fixedCfgPath);
    QVERIFY(fixedCfg.exists());
    // Cleanup
    fixedCfg.remove();
}

QTEST_MAIN(TestConfig)

#include "configtest.moc"

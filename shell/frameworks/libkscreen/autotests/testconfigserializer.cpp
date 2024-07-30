/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include <QObject>
#include <QTest>
#include <cstdint>
#include <utility>

#include "../src/configserializer_p.h"
#include "../src/mode.h"
#include "../src/output.h"
#include "../src/screen.h"
#include "../src/types.h"

class TestConfigSerializer : public QObject
{
    Q_OBJECT

public:
    TestConfigSerializer()
    {
    }

private Q_SLOTS:
    void testSerializePoint()
    {
        const QPoint point(42, 24);

        const QJsonObject obj = KScreen::ConfigSerializer::serializePoint(point);
        QVERIFY(!obj.isEmpty());

        QCOMPARE(obj[QLatin1String("x")].toInt(), point.x());
        QCOMPARE(obj[QLatin1String("y")].toInt(), point.y());
    }

    void testSerializeSize()
    {
        const QSize size(800, 600);

        const QJsonObject obj = KScreen::ConfigSerializer::serializeSize(size);
        QVERIFY(!obj.isEmpty());

        QCOMPARE(obj[QLatin1String("width")].toInt(), size.width());
        QCOMPARE(obj[QLatin1String("height")].toInt(), size.height());
    }

    void testSerializeList()
    {
        QStringList stringList;
        stringList << QStringLiteral("Item 1") << QStringLiteral("Item 2") << QStringLiteral("Item 3") << QStringLiteral("Item 4");

        QJsonArray arr = KScreen::ConfigSerializer::serializeList<QString>(stringList);
        QCOMPARE(arr.size(), stringList.size());

        for (int i = 0; i < arr.size(); ++i) {
            QCOMPARE(arr.at(i).toString(), stringList.at(i));
        }

        QList<int> intList;
        intList << 4 << 3 << 2 << 1;

        arr = KScreen::ConfigSerializer::serializeList<int>(intList);
        QCOMPARE(arr.size(), intList.size());

        for (int i = 0; i < arr.size(); ++i) {
            QCOMPARE(arr.at(i).toInt(), intList[i]);
        }
    }

    void testSerializeScreen()
    {
        KScreen::ScreenPtr screen(new KScreen::Screen);
        screen->setId(12);
        screen->setMinSize(QSize(360, 360));
        screen->setMaxSize(QSize(8192, 8192));
        screen->setCurrentSize(QSize(3600, 1280));
        screen->setMaxActiveOutputsCount(3);

        const QJsonObject obj = KScreen::ConfigSerializer::serializeScreen(screen);
        QVERIFY(!obj.isEmpty());

        QCOMPARE(obj[QLatin1String("id")].toInt(), screen->id());
        QCOMPARE(obj[QLatin1String("maxActiveOutputsCount")].toInt(), screen->maxActiveOutputsCount());
        const QJsonObject minSize = obj[QLatin1String("minSize")].toObject();
        QCOMPARE(minSize[QLatin1String("width")].toInt(), screen->minSize().width());
        QCOMPARE(minSize[QLatin1String("height")].toInt(), screen->minSize().height());
        const QJsonObject maxSize = obj[QLatin1String("maxSize")].toObject();
        QCOMPARE(maxSize[QLatin1String("width")].toInt(), screen->maxSize().width());
        QCOMPARE(maxSize[QLatin1String("height")].toInt(), screen->maxSize().height());
        const QJsonObject currSize = obj[QLatin1String("currentSize")].toObject();
        QCOMPARE(currSize[QLatin1String("width")].toInt(), screen->currentSize().width());
        QCOMPARE(currSize[QLatin1String("height")].toInt(), screen->currentSize().height());
    }

    void testSerializeMode()
    {
        KScreen::ModePtr mode(new KScreen::Mode);
        mode->setId(QStringLiteral("755"));
        mode->setName(QStringLiteral("1280x1024"));
        mode->setRefreshRate(50.666);
        mode->setSize(QSize(1280, 1024));

        const QJsonObject obj = KScreen::ConfigSerializer::serializeMode(mode);
        QVERIFY(!obj.isEmpty());

        QCOMPARE(obj[QLatin1String("id")].toString(), mode->id());
        QCOMPARE(obj[QLatin1String("name")].toString(), mode->name());
        QCOMPARE((float)obj[QLatin1String("refreshRate")].toDouble(), mode->refreshRate());
        const QJsonObject size = obj[QLatin1String("size")].toObject();
        QCOMPARE(size[QLatin1String("width")].toInt(), mode->size().width());
        QCOMPARE(size[QLatin1String("height")].toInt(), mode->size().height());
    }

    void testSerializeOutput()
    {
        KScreen::ModeList modes;
        KScreen::ModePtr mode(new KScreen::Mode);
        mode->setId(QStringLiteral("1"));
        mode->setName(QStringLiteral("800x600"));
        mode->setSize(QSize(800, 600));
        mode->setRefreshRate(50.4);
        modes.insert(mode->id(), mode);

        KScreen::OutputPtr output(new KScreen::Output);
        output->setId(60);
        output->setName(QStringLiteral("LVDS-0"));
        output->setType(KScreen::Output::Panel);
        output->setIcon(QString());
        output->setModes(modes);
        output->setPos(QPoint(1280, 0));
        output->setSize(mode->size());
        output->setRotation(KScreen::Output::None);
        output->setCurrentModeId(QStringLiteral("1"));
        output->setPreferredModes(QStringList() << QStringLiteral("1"));
        output->setConnected(true);
        output->setEnabled(true);
        output->setPriority(1);
        output->setClones(QList<int>() << 50 << 60);
        output->setSizeMm(QSize(310, 250));

        const QJsonObject obj = KScreen::ConfigSerializer::serializeOutput(output);
        QVERIFY(!obj.isEmpty());

        QCOMPARE(obj[QLatin1String("id")].toInt(), output->id());
        QCOMPARE(obj[QLatin1String("name")].toString(), output->name());
        QCOMPARE(static_cast<KScreen::Output::Type>(obj[QLatin1String("type")].toInt()), output->type());
        QCOMPARE(obj[QLatin1String("icon")].toString(), output->icon());
        const QJsonArray arr = obj[QLatin1String("modes")].toArray();
        QCOMPARE(arr.size(), output->modes().count());

        QJsonObject pos = obj[QLatin1String("pos")].toObject();
        QCOMPARE(pos[QLatin1String("x")].toInt(), output->pos().x());
        QCOMPARE(pos[QLatin1String("y")].toInt(), output->pos().y());
        const QJsonObject size = obj[QLatin1String("size")].toObject();
        QCOMPARE(size[QLatin1String("width")].toInt(), output->size().width());
        QCOMPARE(size[QLatin1String("height")].toInt(), output->size().height());

        QCOMPARE(static_cast<KScreen::Output::Rotation>(obj[QLatin1String("rotation")].toInt()), output->rotation());
        QCOMPARE(obj[QLatin1String("currentModeId")].toString(), output->currentModeId());
        QCOMPARE(obj[QLatin1String("connected")].toBool(), output->isConnected());
        QCOMPARE(obj[QLatin1String("enabled")].toBool(), output->isEnabled());
        QCOMPARE(obj[QLatin1String("priority")].toInt(), output->priority());
        const QJsonArray clones = obj[QLatin1String("clones")].toArray();
        QCOMPARE(clones.size(), output->clones().count());
        for (int i = 0; i < clones.size(); ++i) {
            QCOMPARE(clones[i].toInt(), output->clones()[i]);
        }
        const QJsonObject sizeMm = obj[QLatin1String("sizeMM")].toObject();
        QCOMPARE(sizeMm[QLatin1String("width")].toInt(), output->sizeMm().width());
        QCOMPARE(sizeMm[QLatin1String("height")].toInt(), output->sizeMm().height());
    }
};

QTEST_MAIN(TestConfigSerializer)

#include "testconfigserializer.moc"

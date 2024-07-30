/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QQmlApplicationEngine>
#include <QSignalSpy>
#include <QTest>

#ifdef Q_OS_UNIX
#include <KConfigGroup>
#include <KSharedConfig>
#endif

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include "common.h"

using namespace Qt::StringLiterals;

class IntegrationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

#ifdef Q_OS_UNIX
    void testAnimationSpeedModifier_kconfig();
#endif
#ifdef Q_OS_WIN
    void testAnimationSpeedModifier_windows();
#endif

private:
    QQmlApplicationEngine engine;
};

void IntegrationTest::initTestCase()
{
    Common::initTestCase();

    QSignalSpy objectCreatedSpy(&engine, &QQmlApplicationEngine::objectCreated);
    engine.load(QFINDTESTDATA(u"animationspeedmodifiertest.qml"_s));
    if (objectCreatedSpy.empty()) {
        QVERIFY(objectCreatedSpy.wait());
    }
    QCOMPARE(engine.rootObjects().size(), 1);
}

void IntegrationTest::cleanupTestCase()
{
}

#ifdef Q_OS_UNIX
void IntegrationTest::testAnimationSpeedModifier_kconfig()
{
    QObject *rootObject = engine.rootObjects().constFirst();
    const double defaultLongDuration = rootObject->property("longDuration").toDouble();
    QSignalSpy longDurationSpy(rootObject, SIGNAL(longDurationChanged()));

    KConfigGroup kdeGroup(KSharedConfig::openConfig(u"kdeglobals"_s, KConfig::SimpleConfig), u"KDE"_s);
    QVERIFY(kdeGroup.isValid());
    kdeGroup.writeEntry("AnimationDurationFactor", 0.5, KConfig::Notify);
    QVERIFY(kdeGroup.sync());

    if (longDurationSpy.empty()) {
        QVERIFY(longDurationSpy.wait());
    }
    QCOMPARE(rootObject->property("longDuration").toDouble(), defaultLongDuration * 0.5);

    kdeGroup.writeEntry("AnimationDurationFactor", 1);
}
#endif

#ifdef Q_OS_WIN
void IntegrationTest::testAnimationSpeedModifier_windows()
{
    bool isAnimated = true;
    QVERIFY(SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &isAnimated, 0));

    QObject *rootObject = engine.rootObjects().constFirst();
    const double defaultLongDuration = rootObject->property("longDuration").toDouble();
    QSignalSpy longDurationSpy(rootObject, SIGNAL(longDurationChanged()));

    if (isAnimated) {
        QVERIFY(defaultLongDuration > 0);
    } else {
        QCOMPARE(defaultLongDuration, 0);
    }
    isAnimated = !isAnimated;
    SystemParametersInfoW(SPI_SETCLIENTAREAANIMATION, 0, &isAnimated, SPIF_SENDCHANGE);
    if (GetLastError() == 1459) {
        return;
    }

    if (longDurationSpy.empty()) {
        QVERIFY(longDurationSpy.wait());
    }

    if (isAnimated) {
        QVERIFY(defaultLongDuration > 0);
    } else {
        QCOMPARE(defaultLongDuration, 0);
    }
}
#endif

QTEST_MAIN(IntegrationTest)

#include "animationspeedmodifiertest.moc"

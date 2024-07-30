/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/

#include <QTest>

#include "../src/inputcapturebarrier.h"

using Barrier = QPair<QPoint, QPoint>;

template<typename T, typename... Ts>
bool operator==(const T &t, const std::variant<Ts...> &variant)
{
    return std::holds_alternative<T>(variant) && std::get<T>(variant) == t;
}

template<typename... Ts>
QDebug &operator<<(QDebug &debug, const std::variant<Ts...> &variant)
{
    auto printer = [&debug](auto &&value) {
        return QTest::toString(value);
    };
    return debug << "variant: " << std::visit(printer, variant) << " (index=" << variant.index() << ")";
}

class InputCaptureBarrierTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidBarriers();
    void testValidBarriersSwapped();
    void testDiagonalBarriers();
    void testBarrierNotOnEdge();
    void testBarrierTooShortOrLong();
    void testMultipleScreens();
};

void InputCaptureBarrierTest::testValidBarriers()
{
    const QList<QRect> screens = {{0, 0, 1920, 1080}};
    QCOMPARE(checkAndMakeBarrier(0, 0, 0, 1079, screens), Barrier({0, 0}, {0, 1079}));
    QCOMPARE(checkAndMakeBarrier(0, 0, 1919, 0, screens), Barrier({0, 0}, {1919, 0}));
    QCOMPARE(checkAndMakeBarrier(1920, 0, 1920, 1079, screens), Barrier({1919, 0}, {1919, 1079}));
    QCOMPARE(checkAndMakeBarrier(0, 1080, 1919, 1080, screens), Barrier({0, 1079}, {1919, 1079}));
}

void InputCaptureBarrierTest::testValidBarriersSwapped()
{
    const QList<QRect> screens = {{0, 0, 1920, 1080}};
    QCOMPARE(checkAndMakeBarrier(0, 1079, 0, 0, screens), Barrier({0, 0}, {0, 1079}));
    QCOMPARE(checkAndMakeBarrier(1919, 0, 0, 0, screens), Barrier({0, 0}, {1919, 0}));
    QCOMPARE(checkAndMakeBarrier(1920, 1079, 1920, 0, screens), Barrier({1919, 0}, {1919, 1079}));
    QCOMPARE(checkAndMakeBarrier(1919, 1080, 0, 1080, screens), Barrier({0, 1079}, {1919, 1079}));
}

void InputCaptureBarrierTest::testDiagonalBarriers()
{
    const QList<QRect> screens = {{0, 0, 1920, 1080}};
    QCOMPARE(checkAndMakeBarrier(0, 0, 1920, 1080, screens), BarrierFailureReason::Diagonal);
    QCOMPARE(checkAndMakeBarrier(1, 1, 2, 2, screens), BarrierFailureReason::Diagonal);
}

void InputCaptureBarrierTest::testBarrierNotOnEdge()
{
    const QList<QRect> screens = {{0, 0, 1920, 1080}};
    QCOMPARE(checkAndMakeBarrier(1, 0, 1, 1079, screens), BarrierFailureReason::NotOnEdge);
    QCOMPARE(checkAndMakeBarrier(0, 1, 1919, 1, screens), BarrierFailureReason::NotOnEdge);

    QCOMPARE(checkAndMakeBarrier(-1, 0, -1, 1079, screens), BarrierFailureReason::NotOnEdge);
    QCOMPARE(checkAndMakeBarrier(0, -1, 1919, -1, screens), BarrierFailureReason::NotOnEdge);
    QCOMPARE(checkAndMakeBarrier(1921, 0, 1921, 1079, screens), BarrierFailureReason::NotOnEdge);
    QCOMPARE(checkAndMakeBarrier(0, 1081, 1919, 1081, screens), BarrierFailureReason::NotOnEdge);
}

void InputCaptureBarrierTest::testBarrierTooShortOrLong()
{
    const QList<QRect> screens = {{0, 0, 1920, 1080}};
    QCOMPARE(checkAndMakeBarrier(0, 0, 0, 1078, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
    QCOMPARE(checkAndMakeBarrier(0, 1, 0, 1079, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
    QCOMPARE(checkAndMakeBarrier(0, 0, 1918, 0, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
    QCOMPARE(checkAndMakeBarrier(1, 0, 1919, 0, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
}

void InputCaptureBarrierTest::testMultipleScreens()
{
    const QList<QRect> screens = {{0, 0, 1920, 1080}, {1920, 0, 1280, 1024}};

    QCOMPARE(checkAndMakeBarrier(0, 0, 0, 1079, screens), Barrier({0, 0}, {0, 1079}));
    QCOMPARE(checkAndMakeBarrier(0, 0, 1919, 0, screens), Barrier({0, 0}, {1919, 0}));
    QCOMPARE(checkAndMakeBarrier(0, 1080, 1919, 1080, screens), Barrier({0, 1079}, {1919, 1079}));

    QCOMPARE(checkAndMakeBarrier(3200, 0, 3200, 1023, screens), Barrier({3199, 0}, {3199, 1023}));
    QCOMPARE(checkAndMakeBarrier(1920, 0, 3199, 0, screens), Barrier({1920, 0}, {3199, 0}));
    QCOMPARE(checkAndMakeBarrier(1920, 1024, 3199, 1024, screens), Barrier({1920, 1023}, {3199, 1023}));

    // Barriers between screens are not allowed
    QCOMPARE(checkAndMakeBarrier(1920, 0, 1920, 1079, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
    QCOMPARE(checkAndMakeBarrier(1920, 0, 1920, 1023, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);

    // Barriers cant span multiple screens
    QCOMPARE(checkAndMakeBarrier(0, 0, 3840, 0, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
    QCOMPARE(checkAndMakeBarrier(0, 1080, 3199, 1080, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
    QCOMPARE(checkAndMakeBarrier(0, 1024, 3199, 1024, screens), BarrierFailureReason::BetweenScreensOrDoesNotFill);
}

QTEST_GUILESS_MAIN(InputCaptureBarrierTest)

#include "inputcapturebarriertest.moc"

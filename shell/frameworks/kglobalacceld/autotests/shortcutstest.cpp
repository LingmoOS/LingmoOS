/*
    SPDX-FileCopyrightText: 2024 Yifan Zhu <fanzhuyifan@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "dummy.h"
#include "kglobalacceld.h"

#include <QPluginLoader>
#include <QSignalSpy>
#include <QStandardPaths>

Q_IMPORT_PLUGIN(KGlobalAccelImpl)

class ShortcutsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testShortcuts_data();
    void testShortcuts();

private:
    std::unique_ptr<KGlobalAccelD> m_globalacceld;
    KGlobalAccelInterface *m_interface;
    KGlobalAccel *m_globalaccel;
};

void ShortcutsTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    qputenv("KGLOBALACCELD_PLATFORM", "dummy");
    m_globalacceld = std::make_unique<KGlobalAccelD>();
    QVERIFY(m_globalacceld->init());
    m_interface = KGlobalAccelImpl::instance();
    QVERIFY(m_interface);
    m_globalaccel = KGlobalAccel::self();
    QVERIFY(m_globalaccel);
}

typedef std::pair<QEvent::Type, int> Event;
typedef QList<Event> Events;

void ShortcutsTest::testShortcuts_data()
{
    QTest::addColumn<QKeySequence>("shortcut");
    QTest::addColumn<Events>("events");
    QTest::addColumn<bool>("triggered");

    // make sure all pressed modifiers are released
    QTest::newRow("no mod") << QKeySequence(Qt::Key_A)
                            << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_A) << std::make_pair(QEvent::KeyRelease, Qt::Key_A)) << true;
    QTest::newRow("mod+key trigger") << QKeySequence(Qt::ControlModifier | Qt::Key_P)
                                     << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                                                  << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_P).toCombined())
                                                  << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_P).toCombined())
                                                  << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                     << true;
    QTest::newRow("mods+key trigger") << QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_M)
                                      << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                                                   << std::make_pair(QEvent::KeyPress, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                                                   << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_M).toCombined())
                                                   << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_M).toCombined())
                                                   << std::make_pair(QEvent::KeyRelease, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                                                   << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                      << true;
    QTest::newRow("mods+key does not trigger mod+key")
        << QKeySequence(Qt::ControlModifier | Qt::Key_P)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::AltModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::Key_Alt | Qt::ControlModifier).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("mod+key does not trigger mods+key")
        << QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_M)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_M).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_M).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;

    QTest::newRow("modifier-only single mod") << QKeySequence(
        Qt::ControlModifier) << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                              << true;
    QTest::newRow("mod+key does not trigger modifier-only single mod")
        << QKeySequence(Qt::ControlModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_P).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("modifier-only multiple mods") << QKeySequence(Qt::ControlModifier | Qt::AltModifier)
                                                 << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                                                              << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                                                              << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                                                              << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
                                                 << true;
    QTest::newRow("modifier-only multiple mods trigger when released out of order")
        << QKeySequence(Qt::ControlModifier | Qt::AltModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Alt) << std::make_pair(QEvent::KeyPress, (Qt::AltModifier | Qt::Key_Control).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << true;
    QTest::newRow("modifier-only multiple mods does not trigger modifier-only single mod")
        << QKeySequence(Qt::ControlModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("modifier-only multiple mods trigger when part of the sequence")
        << QKeySequence(Qt::ControlModifier | Qt::AltModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::AltModifier | Qt::Key_Control).toCombined()) << std::make_pair(QEvent::KeyRelease, Qt::Key_Alt))
        << true;
    QTest::newRow("modifier-only multiple mods trigger when any mod is released")
        << QKeySequence(Qt::ControlModifier | Qt::ShiftModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control)
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::ControlModifier | Qt::Key_Shift).toCombined())
                     << std::make_pair(QEvent::KeyPress, (Qt::ControlModifier | Qt::Key_Alt).toCombined())
                     << std::make_pair(QEvent::KeyRelease, (Qt::AltModifier | Qt::Key_Control).toCombined()) << std::make_pair(QEvent::KeyRelease, Qt::Key_Alt))
        << true;
    QTest::newRow("mod+pointer does not trigger modifier-only single mod")
        << QKeySequence(Qt::ControlModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::MouseButtonPress, Qt::LeftButton)
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("mod+wheel does not trigger modifier-only single mod")
        << QKeySequence(Qt::ControlModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::Key_Control) << std::make_pair(QEvent::Wheel, 0)
                     << std::make_pair(QEvent::KeyRelease, Qt::Key_Control))
        << false;
    QTest::newRow("mod+invalid does not trigger modifier-only single mod")
        << QKeySequence(Qt::ControlModifier)
        << (Events() << std::make_pair(QEvent::KeyPress, Qt::ControlModifier) << std::make_pair(QEvent::KeyRelease, Qt::ControlModifier)) << false;
}

void ShortcutsTest::testShortcuts()
{
    auto action = std::make_unique<QAction>();
    action->setObjectName(QStringLiteral("ActionForShortcutTest"));
    QFETCH(QKeySequence, shortcut);
    QVERIFY(KGlobalAccel::setGlobalShortcut(action.get(), shortcut));
    QCOMPARE(m_globalaccel->shortcut(action.get()), QList<QKeySequence>() << shortcut);

    QSignalSpy spy(action.get(), &QAction::triggered);

    QFETCH(Events, events);
    for (const auto &event : events) {
        switch (event.first) {
        case QEvent::KeyPress:
            QMetaObject::invokeMethod(m_interface, "checkKeyPressed", Qt::DirectConnection, Q_ARG(int, event.second));
            break;
        case QEvent::KeyRelease:
            QMetaObject::invokeMethod(m_interface, "checkKeyReleased", Qt::DirectConnection, Q_ARG(int, event.second));
            break;
        case QEvent::MouseButtonPress:
            QMetaObject::invokeMethod(m_interface,
                                      "checkPointerPressed",
                                      Qt::DirectConnection,
                                      Q_ARG(Qt::MouseButtons, static_cast<Qt::MouseButtons>(event.second)));
            break;
        case QEvent::Wheel:
            QMetaObject::invokeMethod(m_interface, "checkAxisTriggered", Qt::DirectConnection, Q_ARG(int, event.second));
            break;
        default:
            qFatal("Unknown event type");
        }
    }

    QFETCH(bool, triggered);
    if (triggered) {
        QVERIFY(spy.wait());
        QCOMPARE(spy.count(), 1);
    } else {
        QVERIFY(!spy.wait());
        QCOMPARE(spy.count(), 0);
    }
    m_globalaccel->removeAllShortcuts(action.get());
}

QTEST_MAIN(ShortcutsTest)

#include "shortcutstest.moc"

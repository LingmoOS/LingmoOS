/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "ktooltiphelper_unittest.h"

#include <KToolTipHelper>

#include <QTest>

#include <QAction>
#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QStyle>
#include <QToolButton>
#include <QToolTip>
#include <QWhatsThis>

#include <memory>

QString KToolTipHelper_UnitTest::shownToolTip(QWidget *widget)
{
    QTest::mouseMove(m_frameWithoutToolTip);
    const bool resetWorked = QTest::qWaitFor(
        []() {
            return !QToolTip::isVisible();
        },
        3000);
    if (!resetWorked) {
        qWarning("The tooltip did not properly hide itself after moving to an area without tooltip.");
    }

    QTest::mouseMove(widget);
    if (!QTest::qWaitFor(&QToolTip::isVisible, widget->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay, nullptr, widget) + 1000)) {
        return QStringLiteral("");
    }
    return QToolTip::text();
}

void KToolTipHelper_UnitTest::initTestCase()
{
    m_window.reset(new QMainWindow());
    m_centralWidget = new QWidget(m_window.get());
    m_centralWidget->setGeometry(0, 0, 100, 100);
    m_window->setCentralWidget(m_centralWidget);

    QHBoxLayout *layout = new QHBoxLayout(m_centralWidget);

    m_frame = new QFrame(m_centralWidget);
    m_frame->setFrameStyle(3);
    new QLabel(QStringLiteral("test area"), m_frame);
    layout->addWidget(m_frame);

    m_toolButton = new QToolButton(m_centralWidget);
    layout->addWidget(m_toolButton);

    m_frameWithoutToolTip = new QFrame(m_centralWidget);
    m_frameWithoutToolTip->setFrameStyle(2);
    new QLabel(QStringLiteral("no tooltip"), m_frameWithoutToolTip);
    layout->addWidget(m_frameWithoutToolTip);

    qApp->installEventFilter(KToolTipHelper::instance());

    m_window->show();
    m_window->ensurePolished();
}

void KToolTipHelper_UnitTest::testGeneralWidget()
{
    // tests without whatsThis()
    QVERIFY(shownToolTip(m_frameWithoutToolTip).isEmpty());
    m_frame->setToolTip(QStringLiteral("frame's tooltip"));
    QCOMPARE(shownToolTip(m_frame), m_frame->toolTip());

    QHelpEvent *helpEvent = new QHelpEvent(QEvent::ToolTip, QPoint(1, 1), m_frame->mapToGlobal(QPoint(1, 1)));
    QVERIFY2(!KToolTipHelper::instance()->eventFilter(m_frame, helpEvent),
             "These most basic tooltips should not be filtered so applications can still rely"
             "on tooltip events in most cases.");

    // tests with whatsThis()
    m_frame->setToolTip(QStringLiteral(""));
    m_frame->setWhatsThis(QStringLiteral("frame's whatsThis"));
    QVERIFY2(shownToolTip(m_frame).isEmpty(), "No whatsThisHint should be shown when no toolTip was set.");

    m_frame->setToolTip(KToolTipHelper::whatsThisHintOnly());
    QVERIFY2(shownToolTip(m_frame) != KToolTipHelper::whatsThisHintOnly(),
             "The KToolTipHelper::whatsThisHintOnly()-string is no user-facing string"
             "and should therefore never actually be displayed.");

    // test when whatsThis() == toolTip()
    m_frame->setToolTip(QStringLiteral("frame's whatsThis"));
    const QString noWhatsThisToolTip = shownToolTip(m_frame);
    QVERIFY(noWhatsThisToolTip.contains(m_frame->toolTip()));
    QVERIFY2(noWhatsThisToolTip.length() == m_frame->toolTip().length(), "No whatsThisHint should be shown when the toolTip is a similar string.");

    m_frame->setToolTip(QStringLiteral("frame's tooltip"));
    const QString toolTip = shownToolTip(m_frame);
    QVERIFY(toolTip.contains(m_frame->toolTip()));
    QVERIFY2(toolTip.length() > m_frame->toolTip().length(), "The frame's toolTip is supposed to contain the whatsThisHint.");

    auto layout = new QVBoxLayout(m_frame);
    auto subFrame = std::unique_ptr<QFrame>(new QFrame(m_frame));
    new QLabel(QStringLiteral("subFrame"), subFrame.get());
    layout->addWidget(subFrame.get());
    QCOMPARE(shownToolTip(subFrame.get()), toolTip);

    subFrame->setToolTip(QStringLiteral("subFrame's tooltip"));
    QCOMPARE(shownToolTip(subFrame.get()), subFrame->toolTip());
}

void KToolTipHelper_UnitTest::testInvokingWhatsThis()
{
    m_frame->setWhatsThis(QStringLiteral("frame's whatsThis"));
    m_frame->setToolTip(KToolTipHelper::whatsThisHintOnly());
    shownToolTip(m_frame);
    QTest::keyClick(m_frame, Qt::Key_Shift);
    QVERIFY2(QTest::qWaitFor(
                 []() {
                     return !QToolTip::isVisible();
                 },
                 4000),
             "whatsThis should be shown now.");
    QVERIFY2(shownToolTip(m_frame).isEmpty(),
             "A toolTip was shown which shouldn't be possible because a WhatsThis widget"
             "should be displayed at this moment.");
    QWhatsThis::hideText();
    QVERIFY2(!shownToolTip(m_frame).isEmpty(),
             "A toolTip was not shown although this should now be possible again "
             "because the WhatsThis widget was just hidden.");
}

void KToolTipHelper_UnitTest::testQToolButton()
{
    QVERIFY(shownToolTip(m_toolButton).isEmpty());

    auto action = std::unique_ptr<QAction>(new QAction(QStringLiteral("action")));
    action->setToolTip(QStringLiteral("action's tooltip"));
    m_toolButton->setDefaultAction(action.get());
    QCOMPARE(shownToolTip(m_toolButton), action->toolTip());

    auto helpEvent = std::unique_ptr<QHelpEvent>(new QHelpEvent(QEvent::ToolTip, QPoint(1, 1), m_toolButton->mapToGlobal(QPoint(1, 1))));
    QVERIFY2(!KToolTipHelper::instance()->eventFilter(m_toolButton, helpEvent.get()),
             "These most basic tooltips should not be filtered so applications can still rely"
             "on tooltip events in most cases.");

    action->setShortcut(Qt::CTRL | Qt::Key_K);
    const QString toolTip(shownToolTip(m_toolButton));
    QVERIFY(toolTip.contains(action->toolTip()));
    // qDebug("%s > %s", qPrintable(toolTip), qPrintable(action->toolTip()));
    QVERIFY2(toolTip.length() > action->toolTip().length(), "The Keyboard shortcut should be visible.");

    action->setWhatsThis(QStringLiteral("action's whatsThis"));
    const QString toolTipWithWhatsThisHint(shownToolTip(m_toolButton));
    QVERIFY(toolTipWithWhatsThisHint.contains(toolTip));
    // qDebug("%s > %s", qPrintable(toolTipWithWhatsThisHint), qPrintable(toolTip));
    QVERIFY2(toolTipWithWhatsThisHint.length() > toolTip.length(), "The whatsThisHint should be visible.");

    action->setShortcut(QKeySequence());
    QVERIFY(shownToolTip(m_toolButton).length() < toolTipWithWhatsThisHint.length());

    action->setWhatsThis(QStringLiteral(""));
    QCOMPARE(shownToolTip(m_toolButton), action->toolTip());

    action->setToolTip(KToolTipHelper::whatsThisHintOnly());
    QVERIFY2(shownToolTip(m_toolButton).isEmpty(), "It should not show the whatsThisHint if there is no whatsThis text.");

    action->setWhatsThis(QStringLiteral("action's whatsThis"));
    QVERIFY2(!shownToolTip(m_toolButton).isEmpty(), "The whatsThisHint should be shown.");
}

void KToolTipHelper_UnitTest::testQMenu()
{
    auto menu = std::unique_ptr<QMenu>(new QMenu(m_centralWidget));
    auto action = std::unique_ptr<QAction>(new QAction(QStringLiteral("action")));
    action->setDisabled(true); // The tooltip should also be shown for disabled actions.
    menu->addAction(action.get());
    menu->show();
    QVERIFY(shownToolTip(menu.get()).isEmpty());

    action->setToolTip(QStringLiteral("action's tooltip"));
    const QString toolTip(shownToolTip(menu.get()));
    QCOMPARE(toolTip, action->toolTip());
    action->setShortcut(Qt::CTRL | Qt::Key_K);
    QCOMPARE(shownToolTip(menu.get()), toolTip);

    action->setWhatsThis(QStringLiteral("action's whatsThis"));
    const QString toolTipWithWhatsThisHint(shownToolTip(menu.get()));
    QVERIFY2(toolTipWithWhatsThisHint.length() > toolTip.length(), "The tooltip is supposed to contain a whatsThisHint.");

    action->setToolTip(KToolTipHelper::whatsThisHintOnly());
    QVERIFY(shownToolTip(menu.get()).length() < toolTipWithWhatsThisHint.length());

    action->setWhatsThis(QStringLiteral(""));
    QVERIFY(shownToolTip(menu.get()).isEmpty());
}

void KToolTipHelper_UnitTest::cleanupTestCase()
{
    qApp->removeEventFilter(KToolTipHelper::instance());
}

QTEST_MAIN(KToolTipHelper_UnitTest)

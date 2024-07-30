/*
    SPDX-FileCopyrightText: 2007 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstandardactiontest.h"

#include <QAction>
#include <QStandardPaths>
#include <QTest>

#include "kstandardaction.h"

void tst_KStandardAction::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void tst_KStandardAction::shortcutForActionId()
{
    QList<QKeySequence> stdShortcut = KStandardShortcut::shortcut(KStandardShortcut::Cut);

    QAction *cut = KStandardAction::cut(nullptr);
    QList<QKeySequence> actShortcut = cut->shortcuts();
    QCOMPARE(cut->property("defaultShortcuts").value<QList<QKeySequence>>(), actShortcut);
    QVERIFY(stdShortcut == actShortcut);
    delete cut;

    cut = KStandardAction::create(KStandardAction::Cut, nullptr, nullptr, nullptr);
    actShortcut = cut->shortcuts();
    QVERIFY(stdShortcut == actShortcut);
    delete cut;
}

void tst_KStandardAction::changingShortcut()
{
#if !HAVE_QTDBUS
    QSKIP("DBus notifications are disabled");
#endif

    // GIVEN
    KStandardShortcut::saveShortcut(KStandardShortcut::Cut, KStandardShortcut::hardcodedDefaultShortcut(KStandardShortcut::Cut));
    const QList<QKeySequence> newShortcut{Qt::CTRL | Qt::Key_Adiaeresis};
    QVERIFY(newShortcut != KStandardShortcut::cut());

    std::unique_ptr<QAction> action(KStandardAction::cut(nullptr));
    std::unique_ptr<QAction> action2(KStandardAction::create(KStandardAction::Cut, nullptr, nullptr, nullptr));
    QCOMPARE(action->shortcuts(), KStandardShortcut::cut());
    QCOMPARE(action2->shortcuts(), KStandardShortcut::cut());

    // WHEN
    KStandardShortcut::saveShortcut(KStandardShortcut::Cut, newShortcut);

    // THEN
    // (wait for KStandardShortcut::shortcutWatcher to notify about the config file change, and for KStandardAction to update the actions...)
    QTRY_COMPARE(action->shortcuts(), newShortcut);
    QTRY_COMPARE(action2->shortcuts(), newShortcut);
}

class Receiver : public QObject
{
    Q_OBJECT
public:
    Receiver()
        : triggered(false)
    {
    }

    bool triggered;
    QUrl lastUrl;

public Q_SLOTS:
    void onTriggered()
    {
        triggered = true;
    }

    void onUrlSelected(const QUrl &url)
    {
        lastUrl = url;
    }
};

void tst_KStandardAction::testCreateNewStyle()
{
    Receiver receiver;
    QAction *action1 = KStandardAction::create(KStandardAction::Next, &receiver, &Receiver::onTriggered, &receiver);
    QVERIFY(!receiver.triggered);
    action1->trigger();
    QVERIFY(receiver.triggered);

    // check that it works with lambdas as well
    bool triggered = false;
    auto onTriggered = [&] {
        triggered = true;
    };
    QAction *action2 = KStandardAction::create(KStandardAction::Copy, &receiver, onTriggered, &receiver);
    QVERIFY(!triggered);
    action2->trigger();
    QVERIFY(triggered);

    // check ConfigureToolbars
    triggered = false;
    QAction *action3 = KStandardAction::create(KStandardAction::ConfigureToolbars, &receiver, onTriggered, &receiver);
    QVERIFY(!triggered);
    action3->trigger(); // a queued connection should be used here
    QVERIFY(!triggered);
    QCoreApplication::processEvents();
    QVERIFY(triggered);

    QUrl expectedUrl = QUrl(QStringLiteral("file:///foo/bar"));
    KRecentFilesAction *recent = KStandardAction::openRecent(&receiver, &Receiver::onUrlSelected, &receiver);
    QCOMPARE(receiver.lastUrl, QUrl());
    recent->urlSelected(expectedUrl);
    QCOMPARE(receiver.lastUrl, expectedUrl);

    // same again with lambda
    QUrl url;
    KRecentFilesAction *recent2 = KStandardAction::openRecent(
        &receiver,
        [&](const QUrl &u) {
            url = u;
        },
        &receiver);
    QCOMPARE(url, QUrl());
    recent2->urlSelected(expectedUrl);
    QCOMPARE(url, expectedUrl);

    // make sure the asserts don't trigger (action has the correct type)
    KToggleAction *toggle1 = KStandardAction::showMenubar(&receiver, &Receiver::onTriggered, &receiver);
    QVERIFY(toggle1);
    KToggleAction *toggle2 = KStandardAction::showStatusbar(&receiver, &Receiver::onTriggered, &receiver);
    QVERIFY(toggle2);
    KToggleFullScreenAction *toggle3 = KStandardAction::fullScreen(&receiver, &Receiver::onTriggered, new QWidget, &receiver);
    QVERIFY(toggle3);
}

void tst_KStandardAction::testCreateOldStyle()
{
    Receiver receiver;
    QAction *action1 = KStandardAction::create(KStandardAction::Next, &receiver, SLOT(onTriggered()), &receiver);
    QVERIFY(!receiver.triggered);
    action1->trigger();
    QVERIFY(receiver.triggered);

    // check ConfigureToolbars
    receiver.triggered = false;
    QAction *action3 = KStandardAction::create(KStandardAction::ConfigureToolbars, &receiver, SLOT(onTriggered()), &receiver);
    QVERIFY(!receiver.triggered);
    action3->trigger(); // a queued connection should be used here
    QVERIFY(!receiver.triggered);
    QCoreApplication::processEvents();
    QVERIFY(receiver.triggered);

    QUrl expectedUrl = QUrl(QStringLiteral("file:///foo/bar"));
    KRecentFilesAction *recent = KStandardAction::openRecent(&receiver, SLOT(onUrlSelected(QUrl)), &receiver);
    QCOMPARE(receiver.lastUrl, QUrl());
    recent->urlSelected(expectedUrl);
    QCOMPARE(receiver.lastUrl, expectedUrl);

    // make sure the asserts don't trigger (action has the correct type)
    KToggleAction *toggle1 = KStandardAction::showMenubar(&receiver, SLOT(onTriggered()), &receiver);
    QVERIFY(toggle1);
    KToggleAction *toggle2 = KStandardAction::showStatusbar(&receiver, SLOT(onTriggered()), &receiver);
    QVERIFY(toggle2);
    auto w = new QWidget;
    KToggleFullScreenAction *toggle3 = KStandardAction::fullScreen(&receiver, SLOT(onTriggered()), w, &receiver);
    QVERIFY(toggle3);
    delete w;
}

QTEST_MAIN(tst_KStandardAction)

#include "kstandardactiontest.moc"
#include "moc_kstandardactiontest.cpp"

/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <functional>
#include <kacceleratormanager.h>

#include <QMenu>
#include <QPushButton>
#include <QTest>

#define QSL QStringLiteral

static QStringList extractActionTexts(QMenu &menu, std::function<QString(const QAction *)> func)
{
    menu.aboutToShow(); // signals are now public in Qt5, how convenient :-)

    QStringList ret;
    bool lastIsSeparator = false;
    const auto menuActions = menu.actions();
    for (const QAction *action : menuActions) {
        if (action->isSeparator()) {
            if (!lastIsSeparator) { // Qt gets rid of duplicate separators, so we should too
                ret.append(QStringLiteral("separator"));
            }
            lastIsSeparator = true;
        } else {
            lastIsSeparator = false;
            if (action->menu()) {
                ret.append(QStringLiteral("submenu"));
            } else {
                const QString text = func(action);
                ret.append(text);
            }
        }
    }
    return ret;
}

class KAcceleratorManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void testActionTexts_data()
    {
        QTest::addColumn<QStringList>("initialTexts");
        QTest::addColumn<QStringList>("expectedTexts");

        QTest::newRow("basic") << QStringList{QSL("Open"), QSL("Close"), QSL("Quit")} << QStringList{QSL("&Open"), QSL("&Close"), QSL("&Quit")};
        QTest::newRow("same_first") << QStringList{QSL("Open"), QSL("Close"), QSL("Clone"), QSL("Quit")}
                                    << QStringList{QSL("&Open"), QSL("&Close"), QSL("C&lone"), QSL("&Quit")};
        QTest::newRow("cjk") << QStringList{QSL("Open (&O)"), QSL("Close (&C)"), QSL("Clone (&C)"), QSL("Quit (&Q)")}
                             << QStringList{QSL("Open (&O)"), QSL("Close (&C)"), QSL("C&lone (C)"), QSL("Quit (&Q)")};
    }

    void testActionTexts()
    {
        // GIVEN
        QFETCH(QStringList, initialTexts);
        QFETCH(QStringList, expectedTexts);
        QMenu menu;
        for (const QString &text : std::as_const(initialTexts)) {
            menu.addAction(text);
        }
        // WHEN
        KAcceleratorManager::manage(&menu);
        // THEN
        const QStringList texts = extractActionTexts(menu, &QAction::text);
        QCOMPARE(texts, expectedTexts);
    }

    void testExistingActionsShortcutsAreTakenIntoAccount()
    {
        std::unique_ptr<QWidget> w(new QWidget());

        QPushButton *pb = new QPushButton(QSL("Open"), w.get());

        KAcceleratorManager::manage(w.get());
        QCOMPARE(pb->text(), QSL("&Open"));

        delete pb;
        pb = new QPushButton(QSL("Open"), w.get());

        QAction *a = new QAction();
        a->setShortcut(QSL("Alt+O"));
        w->addAction(a);

        KAcceleratorManager::manage(w.get());
        QCOMPARE(pb->text(), QSL("O&pen"));
    }

    void testActionIconTexts_data()
    {
        QTest::addColumn<QStringList>("initialTexts");
        QTest::addColumn<QStringList>("expectedTexts");

        QTest::newRow("basic") << QStringList{QSL("Open"), QSL("Close"), QSL("Quit")} << QStringList{QSL("Open"), QSL("Close"), QSL("Quit")};
        QTest::newRow("same_first") << QStringList{QSL("Open"), QSL("Close"), QSL("Clone"), QSL("Quit")}
                                    << QStringList{QSL("Open"), QSL("Close"), QSL("Clone"), QSL("Quit")};
        QTest::newRow("cjk") << QStringList{QSL("Open (&O)"), QSL("Close (&C)"), QSL("Clone (&C)"), QSL("Quit (&Q)")}
                             << QStringList{QSL("Open"), QSL("Close"), QSL("Clone"), QSL("Quit")};
    }

    void testActionIconTexts()
    {
        // GIVEN
        QFETCH(QStringList, initialTexts);
        QFETCH(QStringList, expectedTexts);
        QMenu menu;
        for (const QString &text : std::as_const(initialTexts)) {
            menu.addAction(text);
        }
        // WHEN
        KAcceleratorManager::manage(&menu);
        // THEN
        const QStringList iconTexts = extractActionTexts(menu, &QAction::iconText);
        QCOMPARE(iconTexts, expectedTexts);
    }
};

QTEST_MAIN(KAcceleratorManagerTest)

#include "kacceleratormanagertest.moc"

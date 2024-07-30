/*
 * SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "kopenaction_p.h"
#include "kstandardaction.h"

#include <QAction>
#include <QStandardPaths>
#include <QTest>
#include <QToolBar>

// Fake KActionCollection, just needs the name.
class KActionCollection : public QObject
{
    Q_OBJECT

public:
    explicit KActionCollection(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    Q_INVOKABLE QAction *action(const QString &name) const
    {
        return m_actions.value(name);
    }

    void addAction(QAction *action)
    {
        m_actions.insert(action->objectName(), action);
    }

private:
    QMap<QString, QAction *> m_actions;
};

class KOpenActionTest : public QObject
{
    Q_OBJECT
public:
    explicit KOpenActionTest(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();

    void testDefaults();
    void testActionCollectionNoRecents();
    void testActionCollectionWithRecents();
};

KOpenActionTest::KOpenActionTest(QObject *parent)
    : QObject(parent)
{
}

void KOpenActionTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void KOpenActionTest::testDefaults()
{
    std::unique_ptr<QAction> action{KStandardAction::open(nullptr, nullptr, nullptr)};

    auto *openAction = qobject_cast<KOpenAction *>(action.get());
    QVERIFY(openAction);

    QCOMPARE(openAction->popupMode(), KToolBarPopupAction::NoPopup);
}

void KOpenActionTest::testActionCollectionNoRecents()
{
    KActionCollection collection;

    QAction *openAction = KStandardAction::open(nullptr, nullptr, nullptr);
    openAction->setParent(&collection);
    collection.addAction(openAction);

    QToolBar toolBar;
    toolBar.addAction(openAction);

    // No recent documents actions, no popup.
    QCOMPARE(qobject_cast<KOpenAction *>(openAction)->popupMode(), KToolBarPopupAction::NoPopup);
}

void KOpenActionTest::testActionCollectionWithRecents()
{
    KActionCollection collection;

    QAction *openAction = KStandardAction::open(nullptr, nullptr, nullptr);
    // Simulates KActionCollection which creates the actions without a parent
    // and then sets it later.
    openAction->setParent(&collection);
    collection.addAction(openAction);

    KRecentFilesAction *openRecentAction = KStandardAction::openRecent(nullptr, nullptr, nullptr);
    openRecentAction->setParent(&collection);
    collection.addAction(openRecentAction);

    QToolBar toolBar;
    toolBar.addAction(openAction);

    // No recent documents, no popup.
    QCOMPARE(qobject_cast<KOpenAction *>(openAction)->popupMode(), KToolBarPopupAction::NoPopup);

    openRecentAction->addUrl(QUrl(QStringLiteral("http://www.kde.org")));

    // Got some recent documents, popup should be there now.
    QCOMPARE(qobject_cast<KOpenAction *>(openAction)->popupMode(), KToolBarPopupAction::MenuButtonPopup);
}

QTEST_MAIN(KOpenActionTest)

#include "kopenactiontest.moc"

#include "kactioncollectiontest.h"

#include <QAction>
#include <QPointer>
#include <QSignalSpy>
#include <QtTestWidgets>

#include <KSharedConfig>
#include <KStandardAction>

void tst_KActionCollection::init()
{
    collection = new KActionCollection(static_cast<QObject *>(nullptr));
}

void tst_KActionCollection::cleanup()
{
    delete collection;
    collection = nullptr;
}

void tst_KActionCollection::clear()
{
    QPointer<QAction> action1 = collection->add<QAction>(QStringLiteral("test1"));
    QPointer<QAction> action2 = collection->add<QAction>(QStringLiteral("test2"));
    QPointer<QAction> action3 = collection->add<QAction>(QStringLiteral("test3"));
    QPointer<QAction> action4 = collection->add<QAction>(QStringLiteral("test4"));
    QPointer<QAction> action5 = collection->add<QAction>(QStringLiteral("test5"));
    QPointer<QAction> action6 = collection->add<QAction>(QStringLiteral("test6"));
    QPointer<QAction> action7 = collection->add<QAction>(QStringLiteral("test7"));

    collection->clear();
    QVERIFY(collection->isEmpty());

    QVERIFY(action1.isNull());
    QVERIFY(action2.isNull());
    QVERIFY(action3.isNull());
    QVERIFY(action4.isNull());
    QVERIFY(action5.isNull());
    QVERIFY(action6.isNull());
    QVERIFY(action7.isNull());
}

void tst_KActionCollection::addStandardActionFunctorSignal()
{
    bool received = false;
    QAction *a = collection->addAction(KStandardAction::New, QStringLiteral("test"), this, [&]() {
        received = true;
    });
    a->trigger();
    QVERIFY(received);
    delete a;
    QVERIFY(collection->isEmpty());
}

void tst_KActionCollection::deleted()
{
    // Delete action -> automatically removed from collection
    QAction *a = collection->add<QAction>(QStringLiteral("test"));
    delete a;
    QVERIFY(collection->isEmpty());

    // Delete action's parent -> automatically removed from collection
    QWidget *myWidget = new QWidget(nullptr);
    QPointer<QAction> action = new QAction(/*i18n()*/ QStringLiteral("Foo"), myWidget);
    collection->addAction(QStringLiteral("foo"), action);
    delete myWidget;
    QVERIFY(collection->isEmpty());
    QVERIFY(action.isNull());

    // Delete action's parent, but the action was added to another widget with setAssociatedWidget
    // and that widget gets deleted first.
    myWidget = new QWidget(nullptr);
    QWidget *myAssociatedWidget = new QWidget(myWidget); // child widget
    action = new QAction(/*i18n()*/ QStringLiteral("Foo"), myWidget); // child action
    collection->addAction(QStringLiteral("foo"), action);
    collection->addAssociatedWidget(myAssociatedWidget);
    QVERIFY(myAssociatedWidget->actions().contains(action));
    delete myAssociatedWidget; // would be done by the line below, but let's make sure it happens first
    delete myWidget;
    QVERIFY(collection->isEmpty());
    QVERIFY(action.isNull());
}

void tst_KActionCollection::take()
{
    QAction *a = collection->add<QAction>(QStringLiteral("test"));
    collection->takeAction(a);
    QVERIFY(collection->isEmpty());
    delete a;
}

void tst_KActionCollection::writeSettings()
{
    KConfigGroup cfg = clearConfig();

    QList<QKeySequence> defaultShortcut;
    defaultShortcut << Qt::Key_A << Qt::Key_B;

    QList<QKeySequence> temporaryShortcut;
    temporaryShortcut << Qt::Key_C << Qt::Key_D;

    QAction *actionWithDifferentShortcut = new QAction(this);
    collection->setDefaultShortcuts(actionWithDifferentShortcut, defaultShortcut);
    actionWithDifferentShortcut->setShortcuts(temporaryShortcut);
    collection->addAction(QStringLiteral("actionWithDifferentShortcut"), actionWithDifferentShortcut);

    QAction *immutableAction = new QAction(this);
    collection->setDefaultShortcuts(immutableAction, defaultShortcut);
    immutableAction->setShortcuts(temporaryShortcut);
    collection->setShortcutsConfigurable(immutableAction, false);
    collection->addAction(QStringLiteral("immutableAction"), immutableAction);

    QAction *actionWithSameShortcut = new QAction(this);
    collection->setDefaultShortcuts(actionWithSameShortcut, defaultShortcut);
    collection->addAction(QStringLiteral("actionWithSameShortcut"), actionWithSameShortcut);

    cfg.writeEntry("actionToDelete", QStringLiteral("Foobar"));
    QAction *actionToDelete = new QAction(this);
    collection->setDefaultShortcuts(actionToDelete, defaultShortcut);
    collection->addAction(QStringLiteral("actionToDelete"), actionToDelete);

    collection->writeSettings(&cfg);

    QCOMPARE(cfg.readEntry("actionWithDifferentShortcut", QString()), QKeySequence::listToString(actionWithDifferentShortcut->shortcuts()));
    QCOMPARE(cfg.readEntry("immutableAction", QString()), QString());
    QCOMPARE(cfg.readEntry("actionWithSameShortcut", QString()), QString());
    QCOMPARE(cfg.readEntry("actionToDelete", QString()), QString());

    qDeleteAll(collection->actions());
}

void tst_KActionCollection::readSettings()
{
    KConfigGroup cfg = clearConfig();

    QList<QKeySequence> defaultShortcut;
    defaultShortcut << Qt::Key_A << Qt::Key_B;

    QList<QKeySequence> temporaryShortcut;
    temporaryShortcut << Qt::Key_C << Qt::Key_D;

    cfg.writeEntry("normalAction", QKeySequence::listToString(defaultShortcut));
    cfg.writeEntry("immutable", QKeySequence::listToString(defaultShortcut));
    cfg.writeEntry("empty", QString());

    QAction *normal = new QAction(this);
    collection->addAction(QStringLiteral("normalAction"), normal);

    QAction *immutable = new QAction(this);
    immutable->setShortcuts(temporaryShortcut);
    collection->setDefaultShortcuts(immutable, temporaryShortcut);
    collection->setShortcutsConfigurable(immutable, false);
    collection->addAction(QStringLiteral("immutable"), immutable);

    QAction *empty = new QAction(this);
    collection->addAction(QStringLiteral("empty"), empty);
    collection->setDefaultShortcuts(empty, defaultShortcut);
    empty->setShortcuts(temporaryShortcut);
    QCOMPARE(QKeySequence::listToString(empty->shortcuts()), QKeySequence::listToString(temporaryShortcut));

    collection->readSettings(&cfg);

    QCOMPARE(QKeySequence::listToString(normal->shortcuts()), QKeySequence::listToString(defaultShortcut));
    QCOMPARE(QKeySequence::listToString(empty->shortcuts()), QKeySequence::listToString(defaultShortcut));

    QCOMPARE(QKeySequence::listToString(immutable->shortcuts()), QKeySequence::listToString(temporaryShortcut));

    qDeleteAll(collection->actions());
}

void tst_KActionCollection::insertReplaces1()
{
    QAction *a = new QAction(nullptr);
    QAction *b = new QAction(nullptr);

    collection->addAction(QStringLiteral("a"), a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(collection->action(QStringLiteral("a")) == a);

    collection->addAction(QStringLiteral("a"), b);
    QVERIFY(!collection->actions().contains(a));
    QVERIFY(collection->actions().contains(b));
    QVERIFY(collection->action(QStringLiteral("a")) == b);

    delete a;
    delete b;
}

/**
 * Check that a action added twice under different names only ends up once in
 * the collection
 */
void tst_KActionCollection::insertReplaces2()
{
    QAction *a = new QAction(nullptr);

    collection->addAction(QStringLiteral("a"), a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(collection->action(QStringLiteral("a")) == a);

    // Simple test: Just add it twice
    collection->addAction(QStringLiteral("b"), a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(!collection->action(QStringLiteral("a")));
    QVERIFY(collection->action(QStringLiteral("b")) == a);

    // Complex text: Mesh with the objectname
    a->setObjectName(QStringLiteral("c"));
    collection->addAction(QStringLiteral("d"), a);
    QVERIFY(collection->actions().contains(a));
    QVERIFY(!collection->action(QStringLiteral("b")));
    QVERIFY(!collection->action(QStringLiteral("c")));
    QVERIFY(collection->action(QStringLiteral("d")) == a);

    delete a;
}

KConfigGroup tst_KActionCollection::clearConfig()
{
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();
    cfg->deleteGroup(collection->configGroup());
    return KConfigGroup(cfg, collection->configGroup());
}

void tst_KActionCollection::testSetShortcuts()
{
    QAction *action = new QAction(/*i18n*/ (QStringLiteral("Next Unread &Folder")), this);
    collection->addAction(QStringLiteral("go_next_unread_folder"), action);
    collection->setDefaultShortcut(action, QKeySequence(Qt::ALT | Qt::Key_Plus));
    QList<QKeySequence> shortcut = action->shortcuts();
    shortcut << QKeySequence(Qt::CTRL | Qt::Key_Plus);
    action->setShortcuts(shortcut);
    QCOMPARE(QKeySequence::listToString(action->shortcuts()), QStringLiteral("Alt++; Ctrl++"));

    // Simpler way:
    QList<QKeySequence> shortcut2;
    shortcut2 << QKeySequence(Qt::ALT | Qt::Key_Plus) << QKeySequence(Qt::CTRL | Qt::Key_Plus);
    QCOMPARE(QKeySequence::listToString(shortcut2), QStringLiteral("Alt++; Ctrl++"));
}

void tst_KActionCollection::implicitStandardActionInsertionUsingCreate()
{
    KActionCollection collection(static_cast<QObject *>(nullptr));
    QAction *a = KStandardAction::create(KStandardAction::Undo, qApp, &QCoreApplication::quit, &collection);
    QVERIFY(a);

    QVERIFY(a->parent() == &collection);
    QVERIFY(collection.action(QString::fromLatin1(KStandardAction::name(KStandardAction::Undo))) == a);
}

void tst_KActionCollection::implicitStandardActionInsertionUsingCut()
{
    KActionCollection collection(static_cast<QObject *>(nullptr));
    QAction *cut = KStandardAction::cut(&collection);
    QAction *a = collection.action(QString::fromLatin1(KStandardAction::name(KStandardAction::Cut)));
    QVERIFY(a);
    QVERIFY(a == cut);
}

void tst_KActionCollection::shouldEmitSignals()
{
    QAction *a = new QAction(nullptr);
    QAction *b = new QAction(nullptr);

    QSignalSpy insertedSpy(collection, &KActionCollection::inserted);
    QSignalSpy changedSpy(collection, &KActionCollection::changed);

    // Insert "first"
    collection->addAction(QStringLiteral("first"), a);
    QVERIFY(collection->actions().contains(a));
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(0).value<QAction *>(), a);
    QCOMPARE(changedSpy.count(), 1);
    insertedSpy.clear();
    changedSpy.clear();

    // Replace "first"
    collection->addAction(QStringLiteral("first"), b);
    QVERIFY(!collection->actions().contains(a));
    QVERIFY(collection->actions().contains(b));
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(0).value<QAction *>(), b);
    QCOMPARE(changedSpy.count(), 2); // once for removing a, once for inserting b
    insertedSpy.clear();
    changedSpy.clear();

    // Insert "second"
    collection->addAction(QStringLiteral("second"), a);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(0).value<QAction *>(), a);
    QCOMPARE(changedSpy.count(), 1);
    insertedSpy.clear();
    changedSpy.clear();

    // Remove and delete "second" (which is a)
    collection->removeAction(a);
    QCOMPARE(changedSpy.count(), 1);
    changedSpy.clear();

    // Delete b directly, should automatically remove it from the collection and emit changed
    delete b;
    QCOMPARE(changedSpy.count(), 1);
    changedSpy.clear();
}

QTEST_MAIN(tst_KActionCollection)

#include "kactioncategorytest.h"

#include <QtTestWidgets>

#include "kactioncategory.h"
#include "kactioncollection.h"
#include <KSelectAction>
#include <KStandardAction>

void tst_KActionCategory::tstCreation()
{
    KActionCollection collection((QObject *)nullptr);
    KActionCategory category1(QStringLiteral("category1"), &collection);
    KActionCategory category2(QStringLiteral("category2"), &collection);

    // Check that the name is correct
    QCOMPARE(category1.text(), QStringLiteral("category1"));
    QCOMPARE(category2.text(), QStringLiteral("category2"));

    // Check that the parent is correct
    QCOMPARE(category1.collection(), &collection);
    QCOMPARE(category2.collection(), &collection);

    // Check that the category is available as a child of the collection
    QList<KActionCategory *> categories = collection.findChildren<KActionCategory *>();
    QCOMPARE(categories.size(), 2);
    QCOMPARE(categories.count(&category1), 1);
    QCOMPARE(categories.count(&category2), 1);

    // Change the text
    category1.setText(QStringLiteral("Other Text"));
    QCOMPARE(category1.text(), QStringLiteral("Other Text"));
}

void tst_KActionCategory::tstSynchronization()
{
    KActionCollection collection((QObject *)nullptr);
    KActionCategory category1(QStringLiteral("category1"), &collection);
    KActionCategory category2(QStringLiteral("category2"), &collection);

    // The collection is empty
    QCOMPARE(collection.count(), 0);

    // Now add a action to category1
    QAction *action1 = category1.addAction(QStringLiteral("action1"));
    // Check it was added to the category.
    QCOMPARE(category1.actions().count(), 1);
    QCOMPARE(category1.actions().count(action1), 1);
    // Check it was added to the collection
    QCOMPARE(collection.actions().count(), 1);
    QCOMPARE(collection.actions().count(action1), 1);

    // Short intermezzo. Add the action a second time
    category1.addAction(QStringLiteral("action1_new"), action1);
    QCOMPARE(category1.actions().count(), 1);
    QCOMPARE(category1.actions().count(action1), 1);
    QCOMPARE(collection.actions().count(), 1);
    QCOMPARE(collection.actions().count(action1), 1);

    // Now add a action to category2
    QAction *action2 = category2.addAction(QStringLiteral("action2"));
    // Check it was added to the category.
    QCOMPARE(category2.actions().count(), 1);
    QCOMPARE(category2.actions().count(action2), 1);
    // Check it was added to the collection
    QCOMPARE(collection.actions().count(), 2);
    QCOMPARE(collection.actions().count(action2), 1);

    // Delete action1
    delete action1;
    // Check it was removed from the collection
    QCOMPARE(collection.actions().count(), 1);
    QCOMPARE(collection.actions().count(action1), 0);
    // Check it was removed from the category.
    QCOMPARE(category1.actions().count(), 0);
    QCOMPARE(category1.actions().count(action1), 0);

    // Remove action2 from the collection
    collection.removeAction(action2);
    // Check it was removed from the collection
    QCOMPARE(collection.actions().count(), 0);
    QCOMPARE(collection.actions().count(action2), 0);
    // Check it was removed from the category.
    QCOMPARE(category2.actions().count(), 0);
    QCOMPARE(category2.actions().count(action2), 0);

    // Create another category, add a action, delete the category and check
    // if the action is still part of the collection.
    KActionCategory *category3 = new KActionCategory(QStringLiteral("category3"), &collection);
    QAction *action3 = category3->addAction(QStringLiteral("action3"));
    // Check it was added to the collection
    QCOMPARE(collection.actions().count(action3), 1);
    // delete the category
    delete category3;
    // Make sure the action is still there.
    QCOMPARE(collection.actions().count(action3), 1);
}

void tst_KActionCategory::tstActionCreation()
{
    KActionCollection collection((QObject *)nullptr);
    KActionCategory category(QStringLiteral("category"), &collection);

    // QAction * addAction(const QString &name, QAction *action);
    QAction *action1 = new QAction(nullptr);
    category.addAction(QStringLiteral("action1"), action1);
    QCOMPARE(category.actions().count(action1), 1);
    QCOMPARE(collection.actions().count(action1), 1);

    // QAction * addAction(const QString &name, QAction *action);
    QAction *action2 = new QAction(nullptr);
    category.addAction(QStringLiteral("action2"), action2);
    QCOMPARE(category.actions().count(action2), 1);
    QCOMPARE(collection.actions().count(action2), 1);

    // QAction * addAction(
    //         KStandardAction::StandardAction actionType,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL);
    QAction *action3 = category.addAction(KStandardAction::Revert);
    QCOMPARE(category.actions().count(action3), 1);
    QCOMPARE(collection.actions().count(action3), 1);

    // QAction * addAction(
    //         KStandardAction::StandardAction actionType,
    //         const QString &name,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL);
    QAction *action4 = category.addAction(KStandardAction::Quit, QStringLiteral("myownname"));
    QCOMPARE(action4->objectName(), QStringLiteral("myownname"));
    QCOMPARE(category.actions().count(action4), 1);
    QCOMPARE(collection.actions().count(action4), 1);

    // QAction *addAction(
    //         const QString &name,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL);
    QAction *action5 = category.addAction(QStringLiteral("action5"));
    QCOMPARE(category.actions().count(action5), 1);
    QCOMPARE(collection.actions().count(action5), 1);

    // template<class ActionType>
    // ActionType *add(
    //         const QString &name,
    //         const QObject *receiver = NULL,
    //         const char *member = NULL)
    KSelectAction *action6 = category.add<KSelectAction>(QStringLiteral("action6"));
    QCOMPARE(category.actions().count(action6), 1);
    QCOMPARE(collection.actions().count(action6), 1);

    // There should be 6 actions inside the collection and category
    QCOMPARE(category.actions().count(), 6);
    QCOMPARE(collection.actions().count(), 6);
    delete action1;
    delete action2;
}

QTEST_MAIN(tst_KActionCategory)

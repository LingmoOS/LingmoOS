/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Daniel Calviño Sánchez <danxuliu@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kselectaction_unittest.h"
#include <QComboBox>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTest>
#include <kselectaction.h>
#include <qtoolbar.h>

QTEST_MAIN(KSelectAction_UnitTest)

void KSelectAction_UnitTest::testSetToolTipBeforeRequestingComboBoxWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    selectAction.setToolTip(QStringLiteral("Test"));
    selectAction.setEnabled(false); // also test disabling the action

    QWidget parent;
    QWidget *widget = selectAction.requestWidget(&parent);

    QVERIFY(widget);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->toolTip(), QStringLiteral("Test"));
    QCOMPARE(comboBox->isEnabled(), false);
}

void KSelectAction_UnitTest::testSetToolTipAfterRequestingComboBoxWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    QWidget parent;
    QWidget *widget = selectAction.requestWidget(&parent);

    selectAction.setToolTip(QStringLiteral("Test"));
    selectAction.setEnabled(false); // also test disabling the action

    QVERIFY(widget);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->toolTip(), QStringLiteral("Test"));
    QCOMPARE(comboBox->isEnabled(), false);
}

void KSelectAction_UnitTest::testSetToolTipBeforeRequestingToolButtonWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);
    selectAction.setToolTip(QStringLiteral("Test"));

    QToolBar toolBar;
    // Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    // destroyed). When the action is added to the QToolBar, it requests and
    // releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton *toolButton = qobject_cast<QToolButton *>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->toolTip(), QStringLiteral("Test"));
}

void KSelectAction_UnitTest::testSetToolTipAfterRequestingToolButtonWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    // Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    // destroyed). When the action is added to the QToolBar, it requests and
    // releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    selectAction.setToolTip(QStringLiteral("Test"));

    QVERIFY(widget);
    QToolButton *toolButton = qobject_cast<QToolButton *>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->toolTip(), QStringLiteral("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisBeforeRequestingComboBoxWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    selectAction.setWhatsThis(QStringLiteral("Test"));

    QWidget parent;
    QWidget *widget = selectAction.requestWidget(&parent);

    QVERIFY(widget);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->whatsThis(), QStringLiteral("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisAfterRequestingComboBoxWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    QWidget parent;
    QWidget *widget = selectAction.requestWidget(&parent);

    selectAction.setWhatsThis(QStringLiteral("Test"));

    QVERIFY(widget);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    QCOMPARE(comboBox->whatsThis(), QStringLiteral("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisBeforeRequestingToolButtonWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);
    selectAction.setWhatsThis(QStringLiteral("Test"));

    QToolBar toolBar;
    // Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    // destroyed). When the action is added to the QToolBar, it requests and
    // releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton *toolButton = qobject_cast<QToolButton *>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->whatsThis(), QStringLiteral("Test"));
}

void KSelectAction_UnitTest::testSetWhatsThisAfterRequestingToolButtonWidget()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    // Don't use requestWidget, as it needs a releaseWidget when used in MenuMode
    //(in ComboBoxMode the widget is released automatically when it is
    // destroyed). When the action is added to the QToolBar, it requests and
    // releases the widget as needed.
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    selectAction.setWhatsThis(QStringLiteral("Test"));

    QVERIFY(widget);
    QToolButton *toolButton = qobject_cast<QToolButton *>(widget);
    QVERIFY(toolButton);
    QCOMPARE(toolButton->whatsThis(), QStringLiteral("Test"));
}

void KSelectAction_UnitTest::testChildActionStateChangeComboMode()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);
    QWidget parent;
    QWidget *widget = selectAction.requestWidget(&parent);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    const QString itemText = QStringLiteral("foo");
    QAction *childAction = selectAction.addAction(itemText);
    QCOMPARE(comboBox->itemText(0), itemText);
    childAction->setEnabled(false);
    // There's no API for item-is-enabled, need to go via the internal model like kselectaction does...
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(comboBox->model());
    QVERIFY(model);
    QVERIFY(!model->item(0)->isEnabled());

    // Now remove the action
    selectAction.removeAction(childAction);
    QCOMPARE(comboBox->count(), 0);
    delete childAction;
}

// Bug 436808
void KSelectAction_UnitTest::testCrashComboBoxDestruction()
{
    QWidget parentWidget;
    QMainWindow mainWindow(&parentWidget);

    // Create a KSelectAction in a QWidget
    auto *comboSelect = new KSelectAction(QStringLiteral("selectAction"), &mainWindow);

    // Add some actions and create some connections to them
    for (int i = 0; i < 7; ++i) {
        QAction *action = comboSelect->addAction(QStringLiteral("Combo Action %1").arg(i));
        connect(action, &QAction::triggered, &mainWindow, []() {
            // Empty, just creating a connection to trigger a crash
        });
    }

    comboSelect->setToolBarMode(KSelectAction::ComboBoxMode);
    QToolBar *toolBar = mainWindow.addToolBar(QStringLiteral("Test"));

    // Add the KSelectAction to a toolbar
    toolBar->addAction(comboSelect);
    mainWindow.show();
    mainWindow.activateWindow();
    // Wait for window to show
    QVERIFY(QTest::qWaitForWindowActive(&mainWindow));

    // When this method finishes and the QWidget is destroyed, and there
    // should be no crash in KSelectActionPrivate::comboBoxDeleted()
}

void KSelectAction_UnitTest::testRequestWidgetComboBoxModeWidgetParent()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    QVERIFY(!comboBox->isEnabled());
}

void KSelectAction_UnitTest::testRequestWidgetComboBoxModeWidgetParentSeveralActions()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::ComboBoxMode);

    selectAction.addAction(new QAction(QStringLiteral("action1"), &selectAction));
    selectAction.addAction(new QAction(QStringLiteral("action2"), &selectAction));
    selectAction.addAction(new QAction(QStringLiteral("action3"), &selectAction));

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QComboBox *comboBox = qobject_cast<QComboBox *>(widget);
    QVERIFY(comboBox);
    QVERIFY(comboBox->isEnabled());
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParent()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton *toolButton = qobject_cast<QToolButton *>(widget);
    QVERIFY(toolButton);
    QVERIFY(!toolButton->isEnabled());
    QVERIFY(toolButton->autoRaise());
    QCOMPARE((int)toolButton->focusPolicy(), (int)Qt::NoFocus);
    QCOMPARE(toolButton->defaultAction(), (QAction *)&selectAction);
    QCOMPARE(toolButton->actions().count(), 1);
    QCOMPARE(toolButton->actions().at(0)->text(), QStringLiteral("selectAction"));
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParentSeveralActions()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    selectAction.addAction(new QAction(QStringLiteral("action1"), &selectAction));
    selectAction.addAction(new QAction(QStringLiteral("action2"), &selectAction));
    selectAction.addAction(new QAction(QStringLiteral("action3"), &selectAction));

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QToolButton *toolButton = qobject_cast<QToolButton *>(widget);
    QVERIFY(toolButton);
    QVERIFY(toolButton->isEnabled());
    QVERIFY(toolButton->autoRaise());
    QCOMPARE((int)toolButton->focusPolicy(), (int)Qt::NoFocus);
    QCOMPARE(toolButton->defaultAction(), (QAction *)&selectAction);
    QCOMPARE(toolButton->actions().count(), 4);
    QCOMPARE(toolButton->actions().at(0)->text(), QStringLiteral("selectAction"));
    QCOMPARE(toolButton->actions().at(1)->text(), QStringLiteral("action1"));
    QCOMPARE(toolButton->actions().at(2)->text(), QStringLiteral("action2"));
    QCOMPARE(toolButton->actions().at(3)->text(), QStringLiteral("action3"));
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParentAddActions()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);
    QVERIFY(!widget->isEnabled());

    selectAction.addAction(new QAction(QStringLiteral("action1"), &selectAction));
    selectAction.addAction(new QAction(QStringLiteral("action2"), &selectAction));
    selectAction.addAction(new QAction(QStringLiteral("action3"), &selectAction));

    QVERIFY(widget->isEnabled());
    QCOMPARE(widget->actions().count(), 4);
    QCOMPARE(widget->actions().at(0)->text(), QStringLiteral("selectAction"));
    QCOMPARE(widget->actions().at(1)->text(), QStringLiteral("action1"));
    QCOMPARE(widget->actions().at(2)->text(), QStringLiteral("action2"));
    QCOMPARE(widget->actions().at(3)->text(), QStringLiteral("action3"));
}

void KSelectAction_UnitTest::testRequestWidgetMenuModeWidgetParentRemoveActions()
{
    KSelectAction selectAction(QStringLiteral("selectAction"), nullptr);
    selectAction.setToolBarMode(KSelectAction::MenuMode);

    QToolBar toolBar;
    toolBar.addAction(&selectAction);
    QWidget *widget = toolBar.widgetForAction(&selectAction);

    QVERIFY(widget);

    QAction *action1 = new QAction(QStringLiteral("action1"), &selectAction);
    selectAction.addAction(action1);
    QAction *action2 = new QAction(QStringLiteral("action2"), &selectAction);
    selectAction.addAction(action2);
    QAction *action3 = new QAction(QStringLiteral("action3"), &selectAction);
    selectAction.addAction(action3);

    delete selectAction.removeAction(action1);
    delete selectAction.removeAction(action2);
    delete selectAction.removeAction(action3);

    QVERIFY(!widget->isEnabled());
    QCOMPARE(widget->actions().count(), 1);
    QCOMPARE(widget->actions().at(0)->text(), QStringLiteral("selectAction"));
}

#include "moc_kselectaction_unittest.cpp"

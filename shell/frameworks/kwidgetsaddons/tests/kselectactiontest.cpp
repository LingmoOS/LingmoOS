/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2006 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kselectactiontest.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>

#include <QDebug>

#include <kselectaction.h>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kselectactiontest"));
    QApplication app(argc, argv);

    QWidget parentWidget;
    SelectActionTest *test = new SelectActionTest(&parentWidget);
    test->show();

    return app.exec();
}

SelectActionTest::SelectActionTest(QWidget *parent)
    : QMainWindow(parent)
    , m_comboSelect(new KSelectAction(QStringLiteral("Combo Selection"), this))
    , m_buttonSelect(new KSelectAction(QStringLiteral("Button Selection"), this))
{
    for (int i = 0; i < 7; ++i) {
        QAction *action = m_comboSelect->addAction(QStringLiteral("Combo Action %1").arg(i));
        connect(action, &QAction::triggered, this, &SelectActionTest::slotActionTriggered);
        action = m_buttonSelect->addAction(QStringLiteral("Action %1").arg(i));
        connect(action, &QAction::triggered, this, &SelectActionTest::slotActionTriggered);
    }

    m_comboSelect->setToolBarMode(KSelectAction::ComboBoxMode);
    m_comboSelect->setWhatsThis(QStringLiteral("What's this?"));

    connect(m_comboSelect, &KSelectAction::actionTriggered, this, &SelectActionTest::actionTriggered);
    connect(m_comboSelect, &KSelectAction::indexTriggered, this, &SelectActionTest::indexTriggered);
    connect(m_comboSelect, &KSelectAction::textTriggered, this, &SelectActionTest::textTriggered);

    m_buttonSelect->setToolBarMode(KSelectAction::MenuMode);
    m_buttonSelect->setWhatsThis(QStringLiteral("What's this?"));
    connect(m_buttonSelect, &KSelectAction::actionTriggered, this, &SelectActionTest::actionTriggered);
    connect(m_buttonSelect, &KSelectAction::indexTriggered, this, &SelectActionTest::indexTriggered);
    connect(m_buttonSelect, &KSelectAction::textTriggered, this, &SelectActionTest::textTriggered);

    menuBar()->addAction(m_comboSelect);
    menuBar()->addAction(m_buttonSelect);
    menuBar()->addAction(QStringLiteral("Add an action"), this, &SelectActionTest::addAction);
    menuBar()->addAction(QStringLiteral("Remove an action"), this, &SelectActionTest::removeAction);

    QToolBar *toolBar = addToolBar(QStringLiteral("Test"));
    toolBar->addAction(m_comboSelect);
    toolBar->addAction(m_buttonSelect);
}

void SelectActionTest::actionTriggered(QAction *action)
{
    qDebug() << Q_FUNC_INFO << action;
}

void SelectActionTest::indexTriggered(int index)
{
    qDebug() << Q_FUNC_INFO << index;
}

void SelectActionTest::textTriggered(const QString &text)
{
    qDebug() << Q_FUNC_INFO << '"' << text << '"';
}

void SelectActionTest::addAction()
{
    QAction *action = m_comboSelect->addAction(QStringLiteral("Combo Action %1").arg(m_comboSelect->actions().count()));
    connect(action, &QAction::triggered, this, &SelectActionTest::slotActionTriggered);
    action = m_buttonSelect->addAction(QStringLiteral("Action %1").arg(m_buttonSelect->actions().count()));
    connect(action, &QAction::triggered, this, &SelectActionTest::slotActionTriggered);
}

void SelectActionTest::removeAction()
{
    if (!m_comboSelect->actions().isEmpty()) {
        m_comboSelect->removeAction(m_comboSelect->actions().last());
    }

    if (!m_buttonSelect->actions().isEmpty()) {
        m_buttonSelect->removeAction(m_buttonSelect->actions().last());
    }
}

void SelectActionTest::slotActionTriggered(bool state)
{
    qDebug() << sender() << " state " << state;
}

#include "moc_kselectactiontest.cpp"

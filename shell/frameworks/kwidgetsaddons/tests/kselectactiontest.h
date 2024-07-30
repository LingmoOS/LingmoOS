/*
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSELECTACTION_TEST_H
#define KSELECTACTION_TEST_H

#include <QMainWindow>

class KSelectAction;

class SelectActionTest : public QMainWindow
{
    Q_OBJECT

public:
    SelectActionTest(QWidget *parent = nullptr);

public Q_SLOTS:
    void actionTriggered(QAction *action);
    void indexTriggered(int index);
    void textTriggered(const QString &text);

    void slotActionTriggered(bool state);

    void addAction();
    void removeAction();

private:
    KSelectAction *m_comboSelect;
    KSelectAction *m_buttonSelect;
};

#endif

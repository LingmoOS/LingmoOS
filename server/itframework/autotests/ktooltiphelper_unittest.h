/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KTOOLTIPHELPER_UNITTEST_H
#define KTOOLTIPHELPER_UNITTEST_H

#include <QObject>

#include <QMainWindow>

class QFrame;
class QToolButton;
class QWidget;

class KToolTipHelper_UnitTest : public QObject
{
    Q_OBJECT

private:
    QString shownToolTip(QWidget *widget);

private Q_SLOTS:
    void initTestCase();
    void testGeneralWidget();
    void testInvokingWhatsThis();
    void testQToolButton();
    void testQMenu();
    void cleanupTestCase();

private:
    std::unique_ptr<QMainWindow> m_window;
    QWidget *m_centralWidget;
    QFrame *m_frame;
    QFrame *m_frameWithoutToolTip;
    QToolButton *m_toolButton;
};

#endif

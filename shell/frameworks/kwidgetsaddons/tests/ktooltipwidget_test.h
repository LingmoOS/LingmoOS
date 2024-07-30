/*
    SPDX-FileCopyrightText: 2017 Elvis Angelaccio <elvis.angelaccio@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTOOLTIPWIDGET_TEST_H
#define KTOOLTIPWIDGET_TEST_H

#include <KToolTipWidget>

#include <QDialog>
#include <QTreeWidget>

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    TestDialog(QWidget *parent = nullptr);

private:
    KToolTipWidget m_tooltipWidget;
    QTreeWidget m_view;
};

#endif

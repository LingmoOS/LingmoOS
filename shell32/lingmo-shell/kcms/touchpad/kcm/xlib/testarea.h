/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_testarea.h"

class TestArea : public QWidget
{
    Q_OBJECT
public:
    explicit TestArea(QWidget *);

Q_SIGNALS:
    void enter();
    void leave();

protected:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEvent *) override;
#else
    void enterEvent(QEnterEvent *) override;
#endif
    void leaveEvent(QEvent *) override;

private:
    Ui::TestArea m_ui;
};

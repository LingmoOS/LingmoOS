/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QPushButton>

class TestButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TestButton(QWidget *);

protected:
    void mousePressEvent(QMouseEvent *) override;

private Q_SLOTS:
    void resetText();

private:
    QString m_originalText;
    bool m_firstClick;
};

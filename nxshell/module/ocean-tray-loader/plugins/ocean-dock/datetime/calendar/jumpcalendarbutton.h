// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef JUMPCALENDARBUTTON_H
#define JUMPCALENDARBUTTON_H

#include "commoniconbutton.h"

#include <DLabel>

#include <QFrame>

class JumpCalendarButton : public QFrame
{
    Q_OBJECT
public:
    explicit JumpCalendarButton(QWidget *parent = nullptr);
    JumpCalendarButton(const QIcon& icon, const QString& description, QWidget *parent = nullptr);
    ~JumpCalendarButton();

    void setIcon(const QIcon &icon);
    void setDescription(const QString& description);

signals:
    void clicked();

protected:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void initUI();

private:
    bool m_hover;
    bool m_mousePress;
    CommonIconButton *m_iconButton;
    Dtk::Widget::DLabel *m_descriptionLabel;
};

#endif // JUMPCALENDARBUTTON_H

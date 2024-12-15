// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGPERIODBUTTON_H
#define LOGPERIODBUTTON_H
#include <DPushButton>
#include <QEvent>

class LogPeriodButton : public Dtk::Widget::DPushButton
{
public:
    explicit LogPeriodButton(const QString &text, QWidget *parent = nullptr);
    void setStandardSize(int iStahndardWidth);
    Qt::FocusReason getFocusReason();
protected:
    void enterEvent(QEvent *e)override;
    void leaveEvent(QEvent *e)override;
    void paintEvent(QPaintEvent *event)override;
    void focusInEvent(QFocusEvent *event) override;
private:
    bool isEnter {false};
    int m_stahndardWidth = -1;
    Qt::FocusReason m_reson = Qt::MouseFocusReason;
};

#endif  // LOGPERIODBUTTON_H

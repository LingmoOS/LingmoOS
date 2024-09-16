// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGCOMBOX_H
#define LOGCOMBOX_H
#include <DComboBox>
DWIDGET_USE_NAMESPACE
/**
 * @brief The LogCombox class 屏蔽本类绘制逻辑，因为dtk已经实现键盘focus效果
 */
class LogCombox: public DComboBox
{
public:
    explicit LogCombox(QWidget *parent = nullptr);
    void setFocusReason(Qt::FocusReason iReson);
    Qt::FocusReason getFocusReason();
protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event)override;
    void focusOutEvent(QFocusEvent *event)override;
private:
    Qt::FocusReason m_reson = Qt::MouseFocusReason;
};

#endif // LOGCOMBOX_H

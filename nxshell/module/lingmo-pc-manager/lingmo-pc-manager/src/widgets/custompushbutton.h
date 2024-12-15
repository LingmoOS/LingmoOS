// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMPUSHBUTTON_H
#define CUSTOMPUSHBUTTON_H

#include <DPushButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @brief 自定义按钮  正常没有背景悬停有背景
 */
class CustomPushbutton : public QWidget
{
    Q_OBJECT
public:
    explicit CustomPushbutton(QWidget *parent = nullptr);

    // 设置图标
    void setIcon(QIcon icon);
    void setText(QString string);
    QFont getButtonFont();

    // 鼠标事件
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

signals:
    void clicked();

private:
    DPushButton *m_button;
};

#endif // CUSTOMPUSHBUTTON_H

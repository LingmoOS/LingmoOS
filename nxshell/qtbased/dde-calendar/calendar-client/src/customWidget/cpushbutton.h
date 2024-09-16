// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CPUSHBUTTON_H
#define CPUSHBUTTON_H

#include <DIconButton>

#include <QLabel>

DWIDGET_USE_NAMESPACE

class CPushButton : public QWidget
{
    Q_OBJECT
public:
    explicit CPushButton(QWidget *parent = nullptr);

    //设置高亮状态
    void setHighlight(bool status);
    //返回高亮状态
    bool isHighlight();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_Highlighted = false; //记录当前是否处于高亮状态
    bool m_pressed = false; //记录鼠标是否按下
    QLabel *m_textLabel = nullptr;  //文字控件
    DIconButton *m_iconButton = nullptr; //icon控件
};

#endif // CPUSHBUTTON_H

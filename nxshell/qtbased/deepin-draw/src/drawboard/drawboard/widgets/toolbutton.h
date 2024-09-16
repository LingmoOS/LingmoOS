// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QWidget>
#include <QPushButton>

//#ifdef USE_DTK
//DWIDGET_USE_NAMESPACE
//#endif

class ToolButton : public QPushButton
{
    Q_OBJECT
public:

    /**
     * @description: ToolButton 构造函数
    */
    explicit ToolButton(QWidget *parent = nullptr);

//    /**
//     * @description: setText 设置控件显示的文字
//    */
//    void setText(const QString &text);

//    /**
//     * @description: setIcon 设置控件显示的图标
//    */
//    void setIcon(const QIcon &icon);

protected:
    /**
     * @description: paintEvent 重新绘制控件的样式
    */
    void paintEvent(QPaintEvent *e) override;


    QPen getPen(const QStyleOptionButton option);

private:
    //用于绘制的文字
    //QString m_text;

    //用于绘制的图片
    //QIcon   m_icon;
};

#endif // TOOLBUTTON_H

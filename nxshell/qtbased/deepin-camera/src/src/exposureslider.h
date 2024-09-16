// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPOSURESLIDER_H
#define EXPOSURESLIDER_H

#include <QObject>
#include <QWidget>
#include <QPainterPath>
#include <QWheelEvent>

#include <DSlider>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class ExposureSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int opacity READ getOpacity WRITE setOpacity)//自定义不透明度属性
public:
    ExposureSlider(QWidget *parent = nullptr);
    ~ExposureSlider();

    /**
    * @brief value 获取当前的曝光值
    */
    int value() {return m_curValue; };

    /**
    * @brief getOpacity 获取当前不透明度
    */
    int getOpacity() { return m_opacity; };

    /**
    * @brief setOpacity 设置不透明度 动画会通过这个函数输入插值，修改透明度。
    * @param  opacity 不透明度 0～255
    */
    void setOpacity(int opacity);

    /**
    * @brief showContent 显示滑动条内部内容，动画实现需要
    * @param  show 是否显示
    * @param  isShortCut 是否通过键盘进行触发
    */
    void showContent(bool show, bool isShortCut);

protected:
    /**
    * @brief paintEvent 绘制事件
    * @param  event 事件参数
    */
    void paintEvent(QPaintEvent *event) override;

    /**
    * @brief wheelEvent 鼠标滚轮事件
    * @param  event 事件参数
    */
    void wheelEvent(QWheelEvent *event) override;

    /**
    * @brief keyReleaseEvent 尺寸大小改变事件
    * @param  event 事件参数
    */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
     * @brief eventFilter 事件处理
     * @param e
     */
    bool eventFilter(QObject *obj, QEvent *e) override;


public slots:
    /**
    * @brief onValueChanged 曝光值改变槽函数
    * @param  value 曝光值
    */
    void onValueChanged(int value);

signals:
    /**
    * @brief valueChanged 曝光值改变信号
    * @param  value 曝光值
    */
    void valueChanged(int value);

    /**
    * @brief contentHided 滑动条内部内容隐藏动画结束信号
    */
    void contentHided();

    /**
    * @brief contentHided Enter键按下信号，用于键盘交互
    */
    void enterBtnClicked(bool isShortCut);

private:
    DLabel *m_pLabShowValue;    //曝光值显示
    DSlider *m_slider;          //滑动条
    int m_valueMax;             //最大值
    int m_valueMin;            //最小值
    int m_curValue;            //当前值

    int m_opacity;              //背景透明度

};

#endif // EXPOSURESLIDER_H

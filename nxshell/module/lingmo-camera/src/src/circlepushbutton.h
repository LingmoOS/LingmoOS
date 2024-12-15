// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CIRCLEPUSHBUTTON_H
#define CIRCLEPUSHBUTTON_H

#include <QPushButton>
#include <DApplicationHelper>

class QColor;
class QSvgRenderer;

//自定义圆形按钮
class circlePushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal iconopacity READ getIconOpacity WRITE setIconOpacity)//自定义图标不透明度属性
    Q_PROPERTY(int opacity READ getOpacity WRITE setOpacity)//自定义不透明度属性
    Q_PROPERTY(int rotate READ getRotate WRITE setRotate)//自定义旋转角度属性
public:
    explicit circlePushButton(QWidget *parent = nullptr);
    ~circlePushButton();
    /**
    * @brief setbackground 设置背景色
    * @param color 背景色
    */
    void setbackground(QColor color);
    /**
    * @brief setPixmap 设置正常显示图片、悬浮图片、点击图片
    * @param normalPath 正常显示图片
    * @param hoverPath 悬浮图片
    * @param pressPath 点击图片
    */
    void setPixmap(QString normalPath, QString hoverPath, QString pressPath);
    /**
    * @brief setSelected 设置选中
    * @param selected 是否选择
    */
    void setSelected(bool selected);
    /**
    * @brief getButtonState 获取按钮状态
    * @return  按钮是否选择
    */
    bool getButtonState();
    /**
    * @brief setButtonRadius 设置按钮半径
    * @param  radius 按钮半径
    */
    void setButtonRadius(int radius);
    /**
    * @brief copyPixmap 拷贝另一个按钮的图标
    * @param  other 另外一个按钮
    */
    void copyPixmap(const circlePushButton &other);
    /**
    * @brief copyPixmap 禁用选中
    * @param  disable 是否禁用选中
    */
    void setDisableSelect(bool disable);

    /**
    * @brief getOpacity 获取当前不透明度
    */
    int getOpacity() { return m_opacity; };

    /**
    * @brief getOpacity 获取当前图标不透明度
    */
    int getIconOpacity() { return m_iconOpacity; };

    /**
    * @brief getRotate 获取当前旋转角度
    */
    int getRotate() { return m_rotate; };

    /**
    * @brief setOpacity 设置不透明度 动画会通过这个函数输入插值，修改透明度。
    * @param  opacity 不透明度 0～255
    */
    void setOpacity(int opacity);

    /**
    * @brief setIconOpacity 设置图标不透明度 动画会通过这个函数输入插值，修改透明度。
    * @param  opacity 不透明度 0～100
    */
    void setIconOpacity(qreal opacity);

    /**
    * @brief setRotate 设置旋转角度
    * @param  rotate 角度值 0～360
    */
    void setRotate(int rotate);

    /**
    * @brief setNewNotification 设置是否显示小圆点
    * @param  set_new true显示  false不显示
    */
    void setNewNotification (const bool set_new);

signals:
    void clicked(bool isShortcut = false);

public slots:

protected:
    /**
    * @brief paintEvent 绘制事件
    * @param  event 事件参数
    */
    void paintEvent(QPaintEvent *event) override;

    /**
    * @brief enterEvent 鼠标进入事件
    * @param  event 事件参数
    */
    void enterEvent(QEvent *event) override;

    /**
    * @brief focusInEvent 焦点进入事件
    * @param  event 事件参数
    */
    void focusInEvent(QFocusEvent *event) override;

    /**
    * @brief focusOutEvent 焦点离开事件
    * @param  event 事件参数
    */
    void focusOutEvent(QFocusEvent *event) override;

    /**
    * @brief hideEvent 界面隐藏事件
    * @param  event 事件参数
    */
    void hideEvent(QHideEvent* event) override;

    bool focusNextPrevChild(bool next) override;

    /**
    * @brief leaveEvent 鼠标离开事件
    * @param  event 事件参数
    */
    void leaveEvent(QEvent *event) override;

    /**
    * @brief mousePressEvent 鼠标点击事件
    * @param  event 事件参数
    */
    void mousePressEvent(QMouseEvent *event) override;

    /**
    * @brief mouseMoveEvent 鼠标移动事件
    * @param  event 事件参数
    */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
    * @brief mouseReleaseEvent 鼠标释放事件
    * @param  event 事件参数
    */
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_hover;//是否悬浮
    bool m_mousePress;//是否点击
    bool m_isSelected;//是否选中

    QColor m_color;//背景色

    QSharedPointer<QSvgRenderer> m_normalSvg;//正常SVG
    QSharedPointer<QSvgRenderer> m_hoverSvg;//悬浮SVG
    QSharedPointer<QSvgRenderer> m_pressSvg;//点击SVG

    int           m_radius;//半径
    bool          m_disableSelect;//禁用选中
    int           m_opacity = 102;    //不透明度 默认值102
    qreal         m_iconOpacity = 1.f; //图标不透明度 默认值1
    int           m_rotate = 0; //旋转角度
    bool          m_isNewNotification = false;  //是否显示小圆点
};

typedef QList<circlePushButton*> circlePushBtnList;

#endif // CIRCLEPUSHBUTTON_H

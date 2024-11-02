/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KSLIDER_H
#define KSLIDER_H

#include <QSlider>
#include <QList>
#include "gui_g.h"

namespace kdk
{
/** @defgroup SliderModule
  * @{
  */

enum KSliderType
{
    SmoothSlider,
    StepSlider,
    NodeSlider,
    SingleSelectSlider
};

class KSliderPrivate;

/**
 * @brief 支持三种样式的滑动条
 * 非步数关系：步数为1，可以在任意位置点击和拖拽
 * 步数关系：步数为固定值，可根据步数值点击和拖拽
 * 节点关系：步数为节点间隔，可根据节点间隔点击和拖拽
 */
class KSlider:public QSlider
{
    Q_OBJECT
public:
    KSlider(QWidget*parent);
    KSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief 设置节点间隔
     * @param interval
     */
    void setTickInterval(int interval);

    /**
     * @brief 设置滑动条类型
     * @param type
     */
    void setSliderType(KSliderType type);

    /**
     * @brief 获取滑动条类型
     * @return
     */
    KSliderType sliderType();

    /**
     * @brief 获取节点间隔
     * @return
     */
    int	tickInterval() const;

    /**
     * @brief 设置值
     */
    void setValue(int);

    /**
     * @brief 设置是否显示节点
     * @return
     */
    void setNodeVisible(bool flag);

    /**
     * @brief 获取是否显示节点
     * @return
     */
    bool nodeVisible();

    /**
     * @brief 设置tooltip
     * @since 1.2
     */
    void setToolTip(const QString&);

    /**
     * @brief 获取toolTip
     * @since 1.2
     * @return
     */
    QString	toolTip() const;

    /**
     * @brief 设置是否启用半透明效果
     * @since 1.2
     * @param flag
     */
    void setTranslucent(bool flag);

    /**
     * @brief 获取是否启用半透明效果
     * @since 1.2
     * @return flag
     */
    bool isTranslucent();


protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    QSize sizeHint() const  override;
private:
    KSliderPrivate *const d_ptr;

    Q_DISABLE_COPY(KSlider)
    Q_DECLARE_PRIVATE(KSlider)

};

}
/**
  * @example testslider/widget.h
  * @example testslider/widget.cpp
  * @}
  */
#endif // KSLIDER_H

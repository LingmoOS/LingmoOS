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

#ifndef KPROGRESSCIRCLE_H
#define KPROGRESSCIRCLE_H
#include "gui_g.h"
#include <QWidget>
#include "kprogressbar.h"

/**
 * @defgroup BarModule
 */
namespace kdk
{
class KProgressCirclePrivate;

/**
 * @brief 以圆环样式显示进度，支持三种状态
 */
class GUI_EXPORT KProgressCircle : public QWidget
{
    Q_OBJECT

public:
    explicit KProgressCircle(QWidget *parent = nullptr);

    /**
     * @brief 获取KProgressCircle最小值
     * @return int
     */
    int minimum() const;
    /**
     * @brief 获取KProgressCircle最大值
     * @return int
     */
    int maximum() const;

    /**
     * @brief 获取当前值
     * @return int
     */
    int value() const;

    /**
     * @brief 获取文本
     * @return QString
     */
    QString text() const;

    /**
     * @brief 设置文本是否可见
     * @param visible
     */
    void setTextVisible(bool visible);

    /**
     * @brief 返回文本是否可见
     * @return bool
     */
    bool isTextVisible() const;
    /**
     * @brief 获取当前KProgressCircle状态
     * @return
     */
    ProgressBarState state();

    /**
     * @brief 设置当前KProgressCircle状态
     * @param state
     */
    void setState(ProgressBarState state);

Q_SIGNALS:
    void valueChanged(int value);

public Q_SLOTS:
    void reset();
    void setRange(int minimum, int maximum);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setValue(int value);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Q_DECLARE_PRIVATE(KProgressCircle)
    KProgressCirclePrivate* const d_ptr;
};
}


#endif // KPROGRESSCIRCLE_H

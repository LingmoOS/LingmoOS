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

#ifndef KPROGRESSBAR_H
#define KPROGRESSBAR_H

#include "gui_g.h"
#include <QProgressBar>

namespace kdk
{

/** @defgroup BarModule
  * @{
  */

/**
 * @brief 支持三种状态
 */
enum ProgressBarState
{
    NormalProgress,
    FailedProgress,
    SuccessProgress
};
class KProgressBarPrivate;

/**
 * @brief 进度条，支持三种状态显示
 */
class GUI_EXPORT KProgressBar:public QProgressBar
{
    Q_OBJECT

public:
    KProgressBar(QWidget*parent);

    /**
     * @brief 获取状态
     * @return
     */
    ProgressBarState state();

    /**
     * @brief 设置状态
     * @param state
     */
    void setState(ProgressBarState state);

    /**
     * @brief 获取文本
     * @return
     */
    QString text() const override;

    /**
     * @brief 设置方向
     */
    void setOrientation(Qt::Orientation);

    /**
     * @brief 设置进度条宽度
     * @param width
     * @since 1.2
     */
    void setBodyWidth(int width);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const;

private:
    Q_DECLARE_PRIVATE(KProgressBar)
    KProgressBarPrivate* const d_ptr;
};
}
/**
  * @example testprogressbar/widget.h
  * @example testprogressbar/widget.cpp
  * @}
  */
#endif // KPROGRESSBAR_H

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

#ifndef KBALLONTIP_H
#define KBALLONTIP_H

#include <QWidget>
#include "gui_g.h"

namespace  kdk
{
/** @defgroup MessageAlertModule
  * @{
  */

/**
 * @brief 支持五种样式
 */
enum TipType
{
    Nothing,
    Normal,
    Info,
    Warning,
    Error
};

class KBallonTipPrivate;
/**
 * @brief 消息提示框，支持五种样式
 */
class GUI_EXPORT KBallonTip : public QWidget
{
    Q_OBJECT

public:
    explicit KBallonTip(QWidget *parent = nullptr);
    explicit KBallonTip(const QString& content,const TipType& type,QWidget *parent = nullptr);

    /**
     * @brief 显示时间完毕后消失
     * @since 1.2
     */
    void showInfo();
    /**
     * @brief 设置类型
     * @param type
     */
    void setTipType(const TipType& type);

    /**
     * @brief 返回类型
     * @return
     */
    TipType tipType();

    /**
     * @brief 设置文本内容
     * @param text
     */
    void setText(const QString& text);

    /**
     * @brief text
     * @return
     */
    QString text();

    /**
     * @brief 设置内容边距
     * @param left
     * @param top
     * @param right
     * @param bottom
     */
    void setContentsMargins(int left, int top, int right, int bottom);

    /**
     * @brief 设置内容边距
     * @param margins
     */
    void setContentsMargins(const QMargins &margins);

    /**
      * @brief 设置持续时间
      * @since 1.2
      */
    void setTipTime(int my_time);

private Q_SLOTS:
    void onTimeupDestroy();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Q_DECLARE_PRIVATE(KBallonTip)
    KBallonTipPrivate* const d_ptr;
};
}
/**
  * @example testballontip/widget.h
  * @example testballontip/widget.cpp
  * @}
  */
#endif // KBALLONTIP_H

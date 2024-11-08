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

#ifndef KTAG_H
#define KTAG_H

#include "gui_g.h"
#include <QPushButton>

namespace kdk
{
/**
 * @defgroup TagModule
 */
class KTagPrivate;

/**
 * @brief 支持四种风格的tag
 */
enum TagStyle
{
    HighlightTag,
    BoderTag,
    BaseBoderTag,
    GrayTag
};

/**
 * @brief 提供四种样式的标签，响应系统主题色变化
 */
class GUI_EXPORT KTag : public QPushButton
{
    Q_OBJECT

public:
    explicit KTag(QWidget *parent = nullptr);
    /**
     * @brief 设置关闭按钮是否可见
     */
    void setClosable(bool);
    /**
     * @brief 返回关闭按钮是否可见
     * @return bool
     */
    bool closable();
    /**
     * @brief 设置文本
     */
    void setText(const QString&);
    /**
     * @brief 获取当前文本
     * @return QString
     */
    QString text();

    /**
     * @brief 设置tag样式
     */
    void setTagStyle(TagStyle);
    /**
     * @brief 获取当前tag样式
     * @return TagStyle
     */
    TagStyle tagStyle();

protected:
    void paintEvent(QPaintEvent* event);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KTag)
    KTagPrivate * const d_ptr;
};
}


#endif // KTAG_H

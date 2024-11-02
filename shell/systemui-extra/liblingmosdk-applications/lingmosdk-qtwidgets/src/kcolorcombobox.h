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

#ifndef KCOLORCOMBOBOX_H
#define KCOLORCOMBOBOX_H

#include <QComboBox>
#include <QList>
#include <QColor>
#include "gui_g.h"

namespace kdk {

/**
 *  @defgroup ContainerModules
 */
class KColorComboBoxPrivate;

/**
 * @brief 颜色选择框
 * @since 2.0
 */
class GUI_EXPORT KColorComboBox : public QComboBox
{
    Q_OBJECT

public:

    /**
     * @brief 指定显示样式，圆形或者圆角矩形
     */
    enum ComboType
    {
        Circle,
        RoundedRect
    };

    explicit KColorComboBox(QWidget *parent = nullptr);

    /**
     * @brief 以列表形式添加颜色选项
     * @param list
     */
    void setColorList(const QList<QColor>& list);

    /**
     * @brief 获取颜色列表
     * @return
     */
    QList<QColor> colorList();

    /**
     * @brief 添加颜色选项
     * @param color
     */
    void addColor(const QColor& color);

    /**
     * @brief 设置显示样式
     * @param type
     */
    void setComboType(const ComboType& type);

    /**
     * @brief 获取显示样式
     * @return
     */
    ComboType comboType();

    /**
     * @brief 设置item尺寸
     * @param size
     */
    void setPopupItemSize(const QSize&size);

    /**
     * @brief 返回item尺寸
     * @return
     */
    QSize popupItemSzie();


Q_SIGNALS:

    void activated(const QColor& color);
    void currentColorChanged(const QColor& color);
    void highlighted(const QColor& color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Q_DECLARE_PRIVATE(KColorComboBox)
    KColorComboBoxPrivate*const d_ptr;
};
}
#endif // KCOLORCOMBOBOX_H

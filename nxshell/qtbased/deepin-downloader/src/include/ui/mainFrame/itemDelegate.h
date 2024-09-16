// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file itemDelegate.h
 *
 * @brief 列表代理类
 *
 * @date 2020-06-09 10:00
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <DApplication>
#include <DGuiApplicationHelper>
#include <QStyledItemDelegate>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class QProgressBar;
/**
 * @class ItemDelegate
 * @brief 列表代理类
*/
class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ItemDelegate(QObject *parent = nullptr, int Flag = 0);
    ~ItemDelegate();

private:

    /**
     * @brief 绘图事件
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief 大小
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief 开始编辑前调用
     */
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

public slots:
    /**
      * @brief 鼠标悬停行改变
      */
    void onHoverchanged(const QModelIndex &index);

private slots:
    /**
     * @brief 主题改变
     * @param type 颜色类型
    */
    void onPalettetypechanged(DGuiApplicationHelper::ColorType type);

private:
    int m_tableFlag;
    //QProgressBar *progressbar;
    int m_hoverRow;
    QPixmap *m_bgImage;
    QPixmap *m_frontImage;
    bool m_isFirstInside;
    QString m_filePath;
};

#endif

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef PARTITIONTABLEERRORSINFODELEGATE_H
#define PARTITIONTABLEERRORSINFODELEGATE_H

#include <DDialog>
#include <QStyledItemDelegate>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

/**
 * @class PartitionTableErrorsInfoDelegate
 * @brief 列表代理类
*/

class PartitionTableErrorsInfoDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PartitionTableErrorsInfoDelegate(DDialog *dialog);

    /**
     * @brief 设置表格item字体颜色
     * @param color 颜色
     */
    void setTextColor(const QColor &color);

signals:

public slots:

protected:
    /**
     * @brief 重写绘画事件
     * @param painter 画笔
     * @param option 视图小部件中绘制项的参数
     * @param index 数据模型中的数据
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;      //绘制事件

private:
    DDialog *m_dialog;      //父类窗口指针
    QColor m_color; // 表格字体颜色
};

#endif // PARTITIONTABLEERRORSINFODELEGATE_H

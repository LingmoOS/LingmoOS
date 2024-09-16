// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERTABLEVIEW_H
#define DRIVERTABLEVIEW_H

#include "MacroDefinition.h"
#include "driveritem.h"

#include <DObject>
#include <DTreeView>
#include <QStyledItemDelegate>
#include <DCheckBox>
#include <DHeaderView>

#include <QStandardItemModel>

class DriverNameItem;
class DriverCheckItem;

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

/**
 * @brief The DriverItemDelegate class
 * 驱动安装界面单元格代理
 */
class DriverItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DriverItemDelegate(QAbstractItemView *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};




/**
 * @brief The DriverHeaderView class
 */
class DriverHeaderView : public DHeaderView
{
public:
    DriverHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    int sectionSizeHint(int logicalIndex) const;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

protected:
    void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
    int m_spacing {1};
};




/**
 * @brief The DriverTableView class
 * 驱动安装界面的表格
 */
class DriverTableView : public DTreeView
{
    Q_OBJECT
public:
    explicit DriverTableView(DWidget *parent = nullptr);

    /**
     * @brief initHeaderView 设置表头内容
     */
    void initHeaderView(const QStringList& headerList, bool firstCheckbox = false);

    /**
     * @brief appendRowItems 添加郑行item
     */
    void appendRowItems(int column);

    /**
     * @brief setIndexWidget 设置 item widget
     * @param row 行
     * @param column 列
     * @param widget widget
     */
    void setWidget(int row, int column, DWidget* widget);

    /**
     * @brief setHeaderCbStatus 该函数的作用是先判断是否所有item都被选中，如果都被选中则表头选中，如果有没有被选中则表头不选中
     * @param checked
     */
    void setHeaderCbStatus(bool checked);

    /**
     * @brief setHeaderCbEnable 设置是否置灰
     * @param checked
     */
    void setHeaderCbEnable(bool enable);

    /**
     * @brief setItemCbEnable 设置checkbox置灰色
     * @param enable
     */
    void setItemCbEnable(int row, bool enable);

    /**
     * @brief setAllItemCbEanble 统一设置checkbox置灰色
     * @param enable
     */
    void setAllItemCbEanble(bool enable);

    /**
     * @brief setCheckedCBDisable 将所有选中的勾选框置灰色
     */
    void setCheckedCBDisable();

    void setItemOperationEnable(int index, bool enable);

    /**
     * @brief getCheckedDriverIndex 获取选中的index
     * @param lstIndex
     */
   void getCheckedDriverIndex(QList<int> &lstIndex);

   /**
    * @brief setItemStatus 时时设置驱动状态
    * @param index
    * @param s
    */
   void setItemStatus(int index, Status s);

   /**
    * @brief setErrorMsg
    * @param index
    * @param msg
    */
   void setErrorMsg(int index, const QString& msg);

   /**
    * @brief hasItemDisabled 获取是否有item处于disabled的过程中
    * @return
    */
   bool hasItemDisabled();

   /**
    * @brief clear 清楚表格所有内容
    */
   void clear();

   /**
    * @brief removeItemAndWidget 移除指定行的item
    * @param row
    * @param column
    */
   void removeItemAndWidget(int row, int column);

   /**
    * @brief resizeColumn 触发一列的sectionresize事件
    * @param column
    */
   void resizeColumn(int column);

protected:
    /**
     * @brief 重写 paintEvent showEvent drawRow
     */
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent* event) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

signals:
    /**
     * @brief operatorClicked 点击安装或更新的信号
     * @param index
     */
    void operatorClicked(int index, int itemIndex, DriverOperationItem::Mode mode);

    /**
     * @brief itemChecked 选中的信号
     * @param index
     * @param checked
     */
    void itemChecked(int index, bool checked);

private slots:
    /**
     * @brief slotAllItemChecked 选中或不选中所有
     * @param checked 全选中或者全部选中
     */
    void slotAllItemChecked(bool checked);

    /**
     * @brief slotHeaderSectionResized 表头的宽度发生变化
     * @param logicalIndex
     * @param oldSize
     * @param newSize
     */
    void slotHeaderSectionResized(int logicalIndex, int oldSize, int newSize);

private:
    /**
     * @brief getTopRadiusPath 获取圆角矩形
     * @param rect
     * @return
     */
    QPainterPath getTopRadiusPath(const QRect& rect);

private:
    DriverItemDelegate* mp_Delegate;
    DriverHeaderView*   mp_HeadView;
    QStandardItemModel* mp_Model;
    DriverCheckItem *   mp_HeaderCb;
};

#endif // DRIVERTABLEVIEW_H

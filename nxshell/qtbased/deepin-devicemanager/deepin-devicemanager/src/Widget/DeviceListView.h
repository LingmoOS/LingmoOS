// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICELISTVIEW_H
#define DEVICELISTVIEW_H

#include <QObject>
#include <QWidget>

#include <DListView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;


/**
 * @brief The DeviceListviewDelegate class
 * listView代理类
 */
class DeviceListviewDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DeviceListviewDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const;
};


/**
 * @brief The DeviceListView class
 * listView的控件
 */
class DeviceListView : public DListView
{
    Q_OBJECT
public:
    explicit DeviceListView(QWidget *parent = nullptr);
    ~DeviceListView()override;

    /**
     * @brief:添加item
     * @param[in] name : item的文本
     * @param[in] icon : item的图标文件
     */
    void addItem(const QString &name, const QString &iconFile);

    /**
     * @brief:设置当前是否启用
     * @param[enable] : 是否启用
     */
    void setCurItemEnable(bool enable);

    /**
     * @brief setCurItem : 根据str设置当前的item
     * @param str
     */
    void setCurItem(const QString &str);

    /**
     * @brief getConcatenateStrings
     * @return
     */
    QString getConcatenateStrings(const QModelIndex &index);

    /**
     * @brief clearItem : 清楚列表
     */
    void clearItem();

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief keyPressEvent:相应键盘按键事件
     * @param keyEvent:按键：上，下键
     */
    void keyPressEvent(QKeyEvent *keyEvent) override;

private:
    QStandardItemModel        *mp_ItemModel;                   // Model View 模式里面的 Model
};

#endif // DEVICELISTVIEW_H

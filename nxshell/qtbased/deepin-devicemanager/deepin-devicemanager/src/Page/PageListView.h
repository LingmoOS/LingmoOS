// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTVIEWWIDGET_H
#define LISTVIEWWIDGET_H

#include <DWidget>
#include <QAction>
#include <QMenu>
#include <QList>

class DeviceListView;

using namespace Dtk::Widget;

/**
 * @brief The PageListView class
 * 设备列表的展示类
 */

class PageListView : public DWidget
{
    Q_OBJECT
public:
    explicit PageListView(DWidget *parent = nullptr);
    ~PageListView() override;

    void updateListItems(const QList<QPair<QString, QString> > &lst);

    /**
     * @brief currentIndex 当前Item类别
     * @return  类别
     */
    QString currentIndex();

    /**
     * @brief currentType:当前展示设备
     * @return 设备
     */
    QString currentType();

    /**
     * @brief clear:清除数据
     */
    void clear();

    void setCurType(QString type);

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

signals:
    /**
     * @brief itemClicked:点击item发出信号
     * @param txt:item显示的字符串
     */
    void itemClicked(const QString &txt);

    void refreshActionTrigger();

    void exportActionTrigger();

private slots:
    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     * @param point:鼠标位置
     */
    void slotShowMenu(const QPoint &point);

    /**
     * @brief slotListViewItemClicked:ListView Item 点击槽函数
     * @param index:点击Item的索引
     */
    void slotListViewItemClicked(const QModelIndex &index);

private:
    DeviceListView            *mp_ListView;
    QAction                   *mp_Refresh;
    QAction                   *mp_Export;
    QMenu                     *mp_Menu;
    QString                   m_CurType;        // 当前显示的设备类型
};

#endif // LISTVIEWWIDGET_H

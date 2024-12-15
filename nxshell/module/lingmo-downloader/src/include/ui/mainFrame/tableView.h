// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file tableView.h
 *
 * @brief 下载条目列表
 *
 * @date 2020-06-09 09:56
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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class Settings;
class TableModel;
class ItemDelegate;
class TopButton;
class TableDataControl;
class DownloadHeaderView;
/**
 * @class TableView
 * @brief 下载条目列表
*/
class TableView : public QTableView
{
    Q_OBJECT
public:
    TableView(int Flag);
    ~TableView();
    void reset(bool switched = false);
    /**
     * @brief 获取model
     * @return model
    */
    TableModel *getTableModel();

    /**
     * @brief 获取control
     * @return Control
    */
    TableDataControl *getTableControl();

    /**
     * @brief 获取header
     * @return header
    */
    DownloadHeaderView *getTableHeader();

    /**
     * @brief 刷新列表
    */
    bool refreshTableView(const int &index);

public slots:
    /**
     * @brief 切换列表
    */
    void onListchanged();

private:
    /**
     * @brief 界面初始化
    */
    void initUI();

    /**
     * @brief 信号槽连接初始化
    */
    void initConnections();

private slots:
    /**
     * @brief model数据改变
     */
    void onModellayoutChanged();

signals:

    //void getDatachanged();
    /**
     * @brief 表头全选按键状态改变信号
     */
    void HeaderStatechanged(bool checked);

    /**
     * @brief 清除表头选中状态信号
     */
    //  void ClearHeaderCheck();

    /**
     * @brief 表头全选按键选中
     */
    void isCheckHeader(bool checked);

    /**
     * @brief 鼠标悬停行改变
     */
    void Hoverchanged(const QModelIndex &index);

protected:
    /**
     * @brief 鼠标按下事件
    */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件
    */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件
    */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief 离开事件
    */
    void leaveEvent(QEvent *event) override;

    /**
     * @brief 键盘按下事件
    */
    void keyPressEvent(QKeyEvent *event) override;

    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

private:
    int m_TableFlag;
    TableModel *m_TableModel;
    TableDataControl *m_TableDataControl;
    DownloadHeaderView *m_HeaderView;
    ItemDelegate *m_Itemdegegate;
    Settings *m_Setting;
    TopButton *m_ToolBar;
    QModelIndex m_PreviousIndex;
};

#include <DListView>
DWIDGET_USE_NAMESPACE
class LeftListView : public DListView
{
    Q_OBJECT
public:
    explicit LeftListView();

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void paintEvent(QPaintEvent *e);
signals:
    void currentIndexChanged(const QModelIndex &current);
};

#endif // TABLEVIEW_H

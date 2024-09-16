// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESLISTVIEW_H
#define PACKAGESLISTVIEW_H

#include <QMouseEvent>
#include <QKeyEvent>
#include <DMenu>
#include <QWidget>
#include <DListView>

DWIDGET_USE_NAMESPACE

class PackagesListView : public DListView
{
    Q_OBJECT
public:
    explicit PackagesListView(QWidget *parent = nullptr);

    /**
     * @brief setRightMenuShowStatus 根据安装进程来设置是否显示右键菜单
     * @param isShow 是否显示右键菜单
     */
    void setRightMenuShowStatus(bool isShow);

signals:

    /**
     * @brief signalChangeItemHeight    设置item高度
     * @param height item高度
     */
    void signalChangeItemHeight(int height);

    /**
     * @brief signalShowContextMenu 显示右键菜单
     * @param index             显示右键菜单的index
     */
    void signalShowContextMenu(QModelIndex index);

    /**
     * @brief signalRemoveItemClicked      删除该item
     * @param index   要删除的item的index
     */
    void signalRemoveItemClicked(QModelIndex index);

    /**
     * @brief signalCurrentIndexRow 当前选中行
     * @param row 行号
     */
    void signalCurrentIndexRow(int row);

public slots:

    /**
     * @brief slotGetPos 获取到的当前Item的位置和row
     * @param rect   当前item的rect
     * @param index  当前item的下标
     */
    void slotGetPos(QRect rect, int index);  // 获取到的当前Item的位置和row
protected:
    /**
     * @brief mousePressEvent 鼠标按下事件
     * 如果是左键按下，如果右键菜单出现，则取消右键菜单
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief mouseReleaseEvent 鼠标释放事件
     * @param event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief setSelection 选中事件
     * @param rect
     * @param command
     * 选中时，允许跳出右键菜单。置标志位为true
     */
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;

    /**
     * @brief keyPressEvent 键盘按下事件
     * @param event
     * 添加alt+m 快捷键触发右键菜单
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief paintEvent   绘制事件
     * @param event
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief focusInEvent 焦点进入事件
     * @param event
     * 当焦点切换到当前控件时，默认选中第一项。
     */
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief event 事件
     */
    virtual bool event(QEvent *event) override;

private:
    /**
     * @brief initUI
     * 初始化 listview的各个属性参数
     */
    void initUI();

    /**
     * @brief initConnections
     * 链接listView的信号和槽
     */
    void initConnections();

    /**
     * @brief initRightContextMenu
     * 初始化右键菜单
     */
    void initRightContextMenu();

    /**
     * @brief initShortcuts
     * 初始化快捷键
     */
    void initShortcuts();

private slots:

    /**
     * @brief slotListViewShowContextMenu 弹出右键菜单
     * @param index 点击右键菜单的index的位置
     */
    void slotListViewShowContextMenu(QModelIndex index);

    /**
     * @brief slotRightMenuDeleteAction   删除某一个包
     */
    void slotRightMenuDeleteAction();

    /**
     * @brief slotShortcutDeleteAction  使用快捷键删除
     */
    void slotShortcutDeleteAction();

private:
    bool m_bLeftMouse = false;       // 左键按下，取消右键菜单标识
    bool m_bShortcutDelete = false;  // 快捷删除标识
    DMenu *m_rightMenu = nullptr;    // 右键菜单

    QModelIndex m_currModelIndex;  // 当前选中的index
    QModelIndex m_highlightIndex;  // 当前选中的index

    QPoint m_rightMenuPos;  // 确定的右键菜单出现的位置

    int m_currentIndex = -1;  // 当前选中的index.row

    /**
     * @brief m_bIsRightMenuShow 当前是否能够调出右键菜单标识，由MultiPage工作状态决定，并传入
     */
    bool m_bIsRightMenuShow = false;  // 当前是否允许右键菜单显示
};

#endif  // PACKAGESLISTVIEW_H

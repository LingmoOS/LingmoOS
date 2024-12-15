// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEFTVIEW_H
#define LEFTVIEW_H

#include "widgets/vnoterightmenu.h"

#include <DTreeView>
#include <DMenu>

#include <QStandardItemModel>
#include <QDateTime>

DWIDGET_USE_NAMESPACE

class LeftViewDelegate;
class LeftViewSortFilter;
class FolderSelectDialog;
class MoveView;

struct VNoteFolder;
//记事本列表
class LeftView : public DTreeView
{
    Q_OBJECT
public:
    //触摸屏事件
    enum TouchState {
        TouchNormal,
        TouchMoving,
        TouchDraging,
        TouchPressing,
        TouchOutVisibleRegion
    };
    //菜单状态
    enum MenuStatus {
        InitialState,
        ReleaseFromMenu,
        Normal
    };
    explicit LeftView(QWidget *parent = nullptr);
    //获取记事本项父节点
    QStandardItem *getNotepadRoot();
    //获取记事本项父节点索引
    QModelIndex getNotepadRootIndex();
    //选中默认记事本项
    QModelIndex setDefaultNotepadItem();
    //清除选中后，恢复选中
    QModelIndex restoreNotepadItem();
    //只有选中项有右键菜单
    void setOnlyCurItemMenuEnable(bool enable);
    //添加记事本
    void addFolder(VNoteFolder *folder);
    //从列表后追加记事本
    void appendFolder(VNoteFolder *folder);
    //记事本项重命名
    void editFolder();
    //记事本排序
    void sort();
    //关闭右键菜单
    void closeMenu();
    //获取记事本个数
    int folderCount();
    //删除选中的记事本
    VNoteFolder *removeFolder();
    //移动笔记
    bool doNoteMove(const QModelIndexList &src, const QModelIndex &dst);
    QModelIndex selectMoveFolder(const QModelIndexList &src);
    //获取记事本顺序
    QString getFolderSort();
    //获取第一个记事本
    VNoteFolder *getFirstFolder();
    //处理触摸屏slide事件
    void handleTouchSlideEvent(qint64 timeParam, double distY, QPoint point);
    //更新触摸屏一指状态
    void setTouchState(const TouchState &touchState);
    //当前记事本笔记个数
    void setNumberOfNotes(int numberOfNotes);
    //弹出右键菜单
    void popupMenu();
    //重命名笔记本
    void renameVNote(QString text);
signals:
    //拖拽到当前记事本
    void dropNotesEnd(bool dropCancel);
    void renameVNoteFolder(QString noteFolderName);

protected:
    //鼠标事件
    //单击
    void mousePressEvent(QMouseEvent *event) override;
    //单击释放
    void mouseReleaseEvent(QMouseEvent *event) override;
    //双击
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    //移动
    void mouseMoveEvent(QMouseEvent *event) override;
    //按键事件
    void keyPressEvent(QKeyEvent *e) override;
    //关闭重命名编辑框触发
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;

    // 拖拽进入视图事件
    void dragEnterEvent(QDragEnterEvent *event) override;
    // 拖拽移动事件
    void dragMoveEvent(QDragMoveEvent *event) override;
    // 拖拽离开视图事件
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    // 拖拽放下事件
    void dropEvent(QDropEvent *event) override;
    //处理触摸移动事件
    void doTouchMoveEvent(QMouseEvent *event);
    //处理触摸屏拖拽事件
    void handleDragEvent(bool isTouch = true);
    //事件过滤
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    //初始化代理模块
    void initDelegate();
    //初始化数据管理模块
    void initModel();
    //初始化右键菜单
    void initMenu();
    //初始化连接
    void initConnections();
    //触发拖动操作
    void triggerDragFolder();
    //设置记事本默认顺序
    bool setFolderSort();
    //拖拽移动
    void doDragMove(const QModelIndex &src, const QModelIndex &dst);

    //记事本列表右键菜单
    VNoteRightMenu *m_notepadMenu {nullptr};
    QStandardItemModel *m_pDataModel {nullptr};
    LeftViewDelegate *m_pItemDelegate {nullptr};
    LeftViewSortFilter *m_pSortViewFilter {nullptr};
    bool m_onlyCurItemMenuEnable {false};
    FolderSelectDialog *m_folderSelectDialog {nullptr};
    MoveView *m_MoveView {nullptr};
    bool m_folderDraing {false};

    //以下为实现触摸屏功能声明参数
    bool m_isTouchSliding {false};
    qint64 m_touchPressStartMs = 0;
    QPoint m_touchPressPoint;
    //是否鼠标拖拽
    bool m_isDraging {false};
    QModelIndex m_index;
    MenuStatus m_menuState {InitialState};
    QTimer *m_selectCurrentTimer {nullptr};
    QTimer *m_popMenuTimer {nullptr};
    TouchState m_touchState {TouchNormal};
    int m_notesNumberOfCurrentFolder = 0;
};

#endif // LEFTVIEW_H

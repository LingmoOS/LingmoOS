// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIDDLEVIEW_H
#define MIDDLEVIEW_H

#include "widgets/vnoterightmenu.h"

#include <DListView>
#include <DMenu>
#include <DLabel>

#include <QDateTime>

DWIDGET_USE_NAMESPACE
class MiddleViewDelegate;
class MiddleViewSortFilter;
class MoveView;

struct VNoteItem;
//记事项列表
class MiddleView : public DListView
{
    Q_OBJECT
public:
    //触摸屏事件状态
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
    //辅助键状态
    enum ModifierState {
        noModifier = 0,
        ctrlModifier = 1,
        shiftAndUpOrDownModifier = 2,
        shiftAndMouseModifier = 3,
        shiftAndHomeOrEndKeyModifier = 4
    };
    //鼠标状态
    enum MouseState {
        normal = 0,
        pressing = 1,
        moving = 2
    };
    //另存为类型
    enum SaveAsType {
        Note = 0, //保存笔记
        Text, //txt类型
        Html, //html类型
        Voice, //语音类型
    };

    explicit MiddleView(QWidget *parent = nullptr);
    //设置搜索关键字
    void setSearchKey(const QString &key);
    //设置记事本id
    void setCurrentId(qint64 id);
    //设置搜索无结果窗口是否可见
    void setVisibleEmptySearch(bool visible);
    //只有选中项有右键菜单
    void setOnlyCurItemMenuEnable(bool enable);
    //头部添加记事项
    void addRowAtHead(VNoteItem *note);
    //尾部追加记事项
    void appendRow(VNoteItem *note);
    //清除记事项
    void clearAll();
    //根据索引选中记事本
    void setCurrentIndex(int index);
    //记事项重命名
    void editNote();
    //另存为
    void saveAs(SaveAsType);
    //关闭右键菜单
    void closeMenu();
    //获取记事项id
    qint64 getCurrentId();
    //获取记事项数目
    qint32 rowCount() const;
    //删除当前选中笔记
    QList<VNoteItem *> deleteCurrentRow();
    //获取当前选中项数据
    VNoteItem *getCurrVNotedata() const;
    //当前选中数据项列表
    QList<VNoteItem *> getCurrVNotedataList() const;
    //置顶/取消置顶
    void noteStickOnTop();
    //排序
    void sortView(bool adjustCurrentItemBar = true);
    //获取选中的笔记列表
    QModelIndexList getAllSelectNote();
    //删除笔记列表
    void deleteModelIndexs(const QModelIndexList &indexs);
    //处理触摸屏slide事件
    void handleTouchSlideEvent(qint64 timeParam, double distY, QPoint point);
    //更新触摸屏一指状态
    void setTouchState(const TouchState &touchState);
    //当前是否为多选
    bool isMultipleSelected();
    //当前选中笔记是否有文本
    bool haveText();
    //当前选中笔记是否有语音
    bool haveVoice();
    //记录笔记项移除前位置
    void setNextSelection();
    //笔记项移除后选中
    void selectAfterRemoved();
    //获取笔记数量
    int getSelectedCount();
    //更新拖拽状态
    void setDragSuccess(bool dragSuccess = false);
    //弹出右键菜单
    void popupMenu();
    //是否是搜索无结果界面
    bool searchEmpty();
signals:
    //请求刷新详情页
    void requestChangeRightView(bool isMultipleOption);
    //刷新窗口
    void requestRefresh();

public slots:
    //更新记事项
    void onNoteChanged();
    //刷新窗口
    void onRefresh();
    //文件导出完成
    void onExportFinished(int err);

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
    //按键释放事件
    void keyReleaseEvent(QKeyEvent *event) override;
    //事件过滤器
    bool eventFilter(QObject *o, QEvent *e) override;
    //按键事件
    void keyPressEvent(QKeyEvent *e) override;
    //关闭重命名编辑框触发
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;
    //处理触摸屏move操作
    void doTouchMoveEvent(QMouseEvent *event);
    //处理拖拽事件
    void handleDragEvent(bool isTouch = true);
    //处理shift+鼠标press选中操作
    void handleShiftAndPress(QModelIndex &index);
    void setModifierState(const ModifierState &modifierState);
    ModifierState getModifierState() const;
    void setMouseState(const MouseState &mouseState);

private:
    //初始化代理模块
    void initDelegate();
    //初始化数据模块
    void initModel();
    //初始化右键菜单
    void initMenu();
    //初始化UI布局
    void initUI();
    //初始化连接
    void initConnections();
    //触发拖动操作
    void triggerDragNote();
    //请求刷新详情页
    void changeRightView(bool isMultipleDetailPage = true);
    //初始化位置状态
    void initPositionStatus(int row);

    bool m_onlyCurItemMenuEnable {false};
    qint64 m_currentId {-1};
    QString m_searchKey;
    DLabel *m_emptySearch {nullptr};
    //笔记列表右键菜单
    VNoteRightMenu *m_noteMenu {nullptr};

    QStandardItemModel *m_pDataModel {nullptr};
    MiddleViewDelegate *m_pItemDelegate {nullptr};
    MiddleViewSortFilter *m_pSortViewFilter {nullptr};
    MoveView *m_MoveView {nullptr};

    //以下为实现触摸屏功能声明参数
    bool m_isTouchSliding {false};
    qint64 m_touchPressStartMs = 0;
    QPoint m_touchPressPoint;
    //正在鼠标拖拽
    bool m_isDraging {false};
    QModelIndex m_index;
    QTimer *m_selectCurrentTimer {nullptr};
    QTimer *m_popMenuTimer {nullptr};
    MenuStatus m_menuState {InitialState};
    TouchState m_touchState {TouchNormal};

    int m_currentRow = -1;
    ModifierState m_modifierState {ModifierState::noModifier};
    MouseState m_mouseState {normal};
    //用于shift多选判断位置
    int m_shiftSelection = -1;
    int m_nextSelection = -1;
    //拖拽完成标志
    bool m_dragSuccess {false};
    QTimer *m_refreshTimer {nullptr};
};

#endif // MIDDLEVIEW_H

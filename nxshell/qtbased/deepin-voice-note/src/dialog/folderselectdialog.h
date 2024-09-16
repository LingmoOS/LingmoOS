// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERSELECTDIALOG_H
#define FOLDERSELECTDIALOG_H

#include <DPushButton>
#include <DWarningButton>
#include <DVerticalLine>
#include <DTreeView>
#include <DStyledItemDelegate>
#include <DWindowCloseButton>
#include <DSuggestButton>
#include <DLabel>
#include <DAbstractDialog>

#include <QStandardItemModel>
#include <QList>
#include <QDateTime>

DWIDGET_USE_NAMESPACE

class LeftViewDelegate;
class LeftViewSortFilter;
struct VNoteFolder;

class FolderSelectView : public DTreeView
{
    Q_OBJECT
public:
    explicit FolderSelectView(QWidget *parent = nullptr);

protected:
    //鼠标move
    void mouseMoveEvent(QMouseEvent *event) override;
    //鼠标press
    void mousePressEvent(QMouseEvent *event) override;
    //鼠标release
    void mouseReleaseEvent(QMouseEvent *event) override;
    //键盘press
    void keyPressEvent(QKeyEvent *event) override;
    //获取焦点
    void focusInEvent(QFocusEvent *e) override;
    //失去焦点
    void focusOutEvent(QFocusEvent *e) override;

protected:
    //处理触摸屏鼠标滑动
    void doTouchMoveEvent(QMouseEvent *eve);
    //处理触摸屏滑动事件
    void handleTouchSlideEvent(qint64 timeInterval, double distY, QPoint point);

private:
    bool m_isTouchSliding {false};
    qint64 m_touchPressStartMs = 0;
    int m_touchPressPointY = 0;
};

class FolderSelectDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit FolderSelectDialog(QStandardItemModel *model, QWidget *parent = nullptr);
    //设置移动笔记信息
    void setNoteContextInfo(const QString &text, int notesNumber);
    //设置不需要显示的记事本
    void setFolderBlack(const QList<VNoteFolder *> &folders);
    //清除选中
    void clearSelection();
    //获取选中的记事本
    QModelIndex getSelectIndex();
    //选中变化处理
    void onVNoteFolderSelectChange(const QItemSelection &selected, const QItemSelection &deselected);
    //字体切换长度适应
    void onFontChanged();

protected:

    void keyPressEvent(QKeyEvent *event) override;
    //初始化布局
    void initUI();
    //连接槽函数
    void initConnections();
    //隐藏事件
    void hideEvent(QHideEvent *event) override;
    //主题切换刷新文本颜色
    void refreshTextColor(bool dark);

    /**
     * @author liuxinping  ut002764
     * @brief  focusInEvent 重写焦点进入事件
     * @param
     * @return
     */
    void focusInEvent(QFocusEvent *event) override;

private:
    DLabel *m_noteInfo {nullptr};
    QString m_notesName = "";
    int m_notesNumber = 0;
    DLabel *m_labMove {nullptr};
    FolderSelectView *m_view {nullptr};
    DWindowCloseButton *m_closeButton {nullptr};
    LeftViewSortFilter *m_model {nullptr};
    LeftViewDelegate *m_delegate {nullptr};
    DPushButton *m_cancelBtn {nullptr};
    DSuggestButton *m_confirmBtn {nullptr};
    DVerticalLine *m_buttonSpliter {nullptr};
};

#endif // FOLDERSELECTDIALOG_H

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LeftViewDelegate_H
#define LeftViewDelegate_H

#include <DStyledItemDelegate>
#include <DPalette>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

//记事本列表代理
class LeftViewDelegate : public DStyledItemDelegate
{
public:
    explicit LeftViewDelegate(QAbstractItemView *parent = nullptr);
    //主题切换处理
    void handleChangeTheme();
    //设置所有列表项是否可用
    void setEnableItem(bool enable);
    //设置是否绘制笔记数目
    void setDrawNotesNum(bool enable);
    //设置是否拖动效果
    void setDragState(bool state);
    //设置是否绘制hover效果
    void setDrawHover(bool enable);
    const int MAX_FOLDER_NAME_LEN = 64;
    //更新判断标志
    void setSelectView(bool selectView);
    //设置tab焦点标志
    void setTabFocus(bool focus);
    //判断是否tab焦点状态
    bool isTabFocus();

protected:
    //绘制列表项
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
    //返回列表项大小
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;
    //创建编辑框，用于重命名
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    //编辑框设置初始内容
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    //编辑完成，数据同步
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    //调整编辑框位置
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    //初始化
    void init();
    //绘制记事本父节点
    void paintNoteRoot(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    //绘制记事本项
    void paintNoteItem(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    //绘制三角
    void paintTriangle(QPainter *painter, const QRect &rc, const QBrush &brush, bool left) const;

    void setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const;

    void paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const;

    QAbstractItemView *m_treeView {nullptr};
    DPalette m_parentPb;
    bool m_enableItem {true};
    bool m_drawNotesNum {true};
    bool m_draging {false};
    bool m_drawHover {true};
    bool m_tabFocus {false};

    //是否为待移动记事本列表
    bool m_isPendingList {false};
};

#endif // LeftViewDelegate_H

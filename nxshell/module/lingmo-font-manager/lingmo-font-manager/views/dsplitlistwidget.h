// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSPLITLISTWIDGET_H
#define DSPLITLISTWIDGET_H

#include "signalmanager.h"

#include <DListView>
#include <DStyledItemDelegate>

#include <QPainterPath>

DWIDGET_USE_NAMESPACE

struct FocusStatus {
    bool m_IsMouseClicked {false};
    bool m_IsFirstFocus{true};
    bool m_IsHalfWayFocus{false};
};

class DSplitListWidget;

class DNoFocusDelegate : public DStyledItemDelegate
{
public:
    explicit DNoFocusDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    //绘制菜单项背景
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option,
                         const QRect &backgroundRect, const QPalette::ColorGroup cg)const;

    void paintTabBackground(QPainter *painter, const QStyleOptionViewItem &option,
                            const QRect &backgroundRect, const QPalette::ColorGroup cg, const bool isHover)const;

    //获取需要绘制区域的路径
    void setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius)const;

    QString adjustLength(QString &titleName, QFont &font) const;
protected:
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)override;

private:
    DSplitListWidget *m_parentView;
    void hideTooltipImmediately();

};

class DSplitListWidget : public DListView
{
    Q_OBJECT
public:
    enum FontGroup {
        AllFont,        //所有字体
        SysFont,        //系统字体
        UserFont,       //用户字体
        CollectFont,    //收藏
        ActiveFont,     //已激活
        ChineseFont,    //中文
        EqualWidthFont  //等宽
    };

    explicit DSplitListWidget(QWidget *parent = nullptr);
    ~DSplitListWidget() override;

    void initListData();

    SignalManager *m_signalManager = SignalManager::instance();
    QStringList m_titleStringList;
    QMap<QString, int> m_titleStringIndexMap;
    QStandardItemModel *m_categoryItemModell;

    void setIsHalfWayFocus(bool IsHalfWayFocus);

    bool IsTabFocus() const;

    FocusStatus &getStatus();

    void setCurrentStatus(const FocusStatus &currentStatus);
    void setCurrentPage();

    //设置m_LastPageNumber接口
    void setLastPageNumber(int LastPageNumber);

    void setIsIstalling(bool isIstalling);

private:
    bool m_refreshFinished = true;
    bool m_isFocusFromFontListView{false};
    //判断鼠标有没有点击
    bool m_IsMouseClicked = false;

    //判断是否通过tab获取的焦点
    bool m_IsTabFocus = false;

    //判断是否是通过键盘左键获取到的焦点，这种情况下选中效果与tab效果一致
    bool m_IsLeftFocus = false;

    //判断是否为第一次打开应用设置的焦点
    bool m_IsFirstFocus = true;

    //判断是否为删除等过程中设置的焦点
    bool m_IsHalfWayFocus = false;
    //用于判断是否弹出提示信息的鼠标状态标志位
    bool m_isMouseMoved{false};

    bool m_isIstalling = false;
    //记录上一个界面对应的数字，用于之后的处理
    int m_LastPageNumber = -1;

    //记录鼠标左键按下时的点用于之后判断
    QPoint lastTouchBeginPos;

    //记录鼠标移动的趋势
    bool m_IsPositive = false;

    FocusStatus m_currentStatus;

signals:
    void onListWidgetItemClicked(int index, bool needClearSelect = true);

public slots:
    void setRefreshFinished(bool isInstalling);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event)override;
    bool eventFilter(QObject *obj, QEvent *event)override;
};

Q_DECLARE_METATYPE(DSplitListWidget::FontGroup)
#endif  // DSPLITLISTWIDGET_H

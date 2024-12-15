// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIPAGESVIEW_H
#define MULTIPAGESVIEW_H

#include <QListView>
#include <QList>
#include <QScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QWheelEvent>

#include "../widgets/applistarea.h"
#include "appgridview.h"
#include "../model/appslistmodel.h"
#include "../model/appsmanager.h"
#include "../global_util/calculate_util.h"
#include "../widgets/gradientlabel.h"
#include "../boxframe/boxframe.h"
#include "pagecontrol.h"

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

class EditLabel;

class MultiPagesView : public QWidget, public DragPageDelegate
{
    Q_OBJECT

public:
    enum Direction {
        Left,
        Right
    };

    explicit MultiPagesView(AppsListModel::AppCategory categoryModel = AppsListModel::FullscreenAll, QWidget *parent = nullptr);
    ~MultiPagesView();

    void refreshTitle(const QString &title, int maxWidth);

    void updatePageCount(AppsListModel::AppCategory category = AppsListModel::FullscreenAll);
    void showCurrentPage(int currentPage);
    QModelIndex selectApp(const int key);
    QModelIndex getAppItem(int index);
    AppGridView *pageView(int pageIndex);
    AppsListModel *pageModel(int pageIndex);

    int currentPage(){return m_pageIndex;}
    int pageCount() { return m_pageCount;}

    void setDataDelegate(QAbstractItemDelegate *delegate);
    void setModel(AppsListModel::AppCategory category);
    void updatePosition(int mode = 0);
    void ShowPageView(AppsListModel::AppCategory category);
    void updateAppDrawerTitle(const QModelIndex &index);

    void mousePress(QMouseEvent *e) override;
    void mouseMove(QMouseEvent *e) override;
    void mouseRelease(QMouseEvent *e) override;

    void setGradientVisible(bool visible);
    void updateGradient();
    void updateGradient(QPixmap& pixmap, QPoint startPoint, QPoint topRightImg);

    QPropertyAnimation::State getPageSwitchAnimationState();
    QWidget *getParentWidget();
    QPoint calculPadding(MultiPagesView::Direction dir);
    AppListArea *getListArea();
    AppGridViewList getAppGridViewList();
    AppsListModel::AppCategory getCategory();
    QSize calculateWidgetSize();
    bool isScrolling();
    EditLabel *getEditLabel();

signals:
    void connectViewEvent(AppGridView* pView);
    void titleChanged();

public slots:
    void resetCurPageIndex();

private slots:
    void dragToLeft(const QModelIndex &index);
    void dragToRight(const QModelIndex &index);
    void dragStop();

private:
    void initUi();
    void initConnection();

protected:
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

private:
    GradientLabel *m_pLeftGradient;
    GradientLabel *m_pRightGradient;

    AppsManager *m_appsManager;                         // 应用管理类
    CalculateUtil *m_calcUtil;                          // 界面布局计算类
    AppListArea *m_appListArea;                         // 滑动区域控件
    AppGridViewList m_appGridViewList;                  // 多视图列表
    PageAppsModelist m_pageAppsModelList;               // 多视图模型列表

    DHBoxWidget *m_viewBox;

    QPropertyAnimation *m_pageSwitchAnimation;          // 分页切换动画

    QAbstractItemDelegate *m_delegate;                  // 视图代理基类
    EditLabel *m_titleLabel;
    PageControl *m_pageControl;                         // 分页控件

    AppsListModel::AppCategory m_category;
    int m_pageCount;
    int m_pageIndex;

    bool m_bDragStart;
    bool m_bMousePress;
    int m_nMousePos;
    int m_scrollValue;
    int m_scrollStart;
    QElapsedTimer *m_changePageDelayTime;                      // 滚动延时，设定时间内只允许滚动一次

    QMap<AppsListModel::AppCategory, QString> m_typeAndTitleMap;
};

#endif // MULTIPAGESVIEW_H

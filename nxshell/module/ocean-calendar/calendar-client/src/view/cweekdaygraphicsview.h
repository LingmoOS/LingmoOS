// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CWEEKDAYGRAPHICSVIEW_H
#define CWEEKDAYGRAPHICSVIEW_H

#include "draginfographicsview.h"
#include "graphicsItem/cweekdaybackgrounditem.h"

/**
 * @brief The CWeekDayGraphicsview class
 *  周/日全天和非全天视图
 */
class CScheduleCoorManage;
class CWeekDayGraphicsview : public DragInfoGraphicsView
{
    Q_OBJECT
public:
    enum ViewPosition {
        WeekPos //周视图
        ,
        DayPos //日视图
    };
    enum ViewType {
        ALLDayView //全天
        ,
        PartTimeView //非全天
    };

public:
    explicit CWeekDayGraphicsview(QWidget *parent = nullptr, ViewPosition viewPos = WeekPos, ViewType viewtype = ALLDayView);
    ~CWeekDayGraphicsview() override;
    void setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin);
    void setRange(QDate begin, QDate end);
    void setTheMe(int type = 0) override;
    CScheduleCoorManage *getCoorManage() const;
    //设置当前焦点背景item
    void setCurrentFocusItem(const QDate &focusDate, bool setItemFocus = false);

protected:
    //设置场景的矩阵
    void setSceneRect(qreal x, qreal y, qreal w, qreal h);
    //创建背景显示项
    void createBackgroundItem();
    void setSceneCurrentItemFocus(const QDate &focusDate) override;
    //更新背景上显示的item
    void updateBackgroundShowItem() override;

private:
    //设置背景时间
    void setBackgroundDate();
signals:
    void signaleSwitchToView(const QDate &focusDate, ViewType type, bool setItemFocus);
    void signalViewFocusInit();
public slots:
    void slotSwitchView(const QDate &focusDate, bool setItemFocus);
    void slotViewInit();
    void slotPosOnView(const qreal y);

protected:
    ViewPosition m_viewPos;
    ViewType m_viewType;
    QDate m_beginDate;
    QDate m_endDate;
    CScheduleCoorManage *m_coorManage;
    int m_rightmagin;
    QVector<CWeekDayBackgroundItem *> m_backgroundItem;
};

#endif // CWEEKDAYGRAPHICSVIEW_H

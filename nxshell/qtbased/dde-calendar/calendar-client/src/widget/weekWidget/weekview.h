// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WEEKVIEW_H
#define WEEKVIEW_H

#include "../widget/touchgestureoperation.h"

#include <DIconButton>

#include <QObject>
#include <QWidget>
#include <QDate>

DWIDGET_USE_NAMESPACE

typedef int(*GetWeekNumOfYear)(const QDate &);

/**
 * @brief The CWeekView class
 * 周数显示类
 */
class CWeekNumWidget;
class CWeekView : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekView(const GetWeekNumOfYear &getWeekNumOfYear, QWidget *parent = nullptr);
    ~CWeekView() override;
    //设置选择时间，并更新
    void setSelectDate(const QDate date);
    //设置当前时间
    void setCurrent(const QDateTime &dateTime);
    //根据系统主题类型设置颜色
    void setTheMe(int type = 0);
signals:
    /**
     * @brief signalsSelectDate     选择日期的信号
     * @param date                  选择的日期
     */
    void signalsSelectDate(const QDate &date);
    /**
     * @brief signalIsDragging 判断是否是拖拽状态
     * @param isDragging 是否是拖拽状态
     */
    void signalIsDragging(bool &isDragging);
    /**
     * @brief signalBtnPrev 前一周按钮信号
     */
    void signalBtnPrev();
    /**
     * @brief signalBtnNext 后一周按钮信号
     */
    void signalBtnNext();
protected:
    /**
     * @brief wheelEvent 鼠标滚轮切换上一周下一周
     * @param event 鼠标滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;
    bool event(QEvent *e) override;

private:
    //上一周按钮
    DIconButton *m_prevButton = nullptr;
    //下一周按钮
    DIconButton *m_nextButton = nullptr;

    //触摸手势处理
    touchGestureOperation m_touchGesture;

    CWeekNumWidget *m_weekNumWidget;
};

class CWeekNumWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekNumWidget(const GetWeekNumOfYear &getWeekNumOfYear, QWidget *parent);
    ~CWeekNumWidget() override;
    //设置选择时间，并更新
    void setSelectDate(const QDate date);
    //设置当前时间
    void setCurrent(const QDateTime &dateTime);
    //根据系统主题类型设置颜色
    void setTheMe(int type = 0);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool event(QEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    //绘制周数
    void paintCell(QWidget *cell);
    //设置被选择的周数
    void setSelectedCell(int index);
    //更新数据
    void updateDate();
signals:
    //选择日期的信号
    void signalsSelectDate(const QDate &date);
    //前一周按钮信号
    void signalBtnPrev();
    //后一周按钮信号
    void signalBtnNext();
public slots:
    //选择点击的日期
    void cellClicked(QWidget *cell);

private:
    QList<QWidget *> m_cellList;
    //选择的日期
    QDate m_selectDate;
    QDate m_days[10];
    int m_selectedCell = 0;
    QFont m_dayNumFont;

    QDateTime m_currentDate;

    GetWeekNumOfYear  m_getWeekNumOfYear;
    QColor m_defaultTextColor = Qt::black;
    QColor m_backgrounddefaultColor = Qt::white;
    QColor m_currentDayTextColor = Qt::white;
    QColor m_backgroundcurrentDayColor = "#0081FF";
    QColor m_fillColor = Qt::white;
    bool m_isFocus;
};

#endif // MONTDAYVIEW_H

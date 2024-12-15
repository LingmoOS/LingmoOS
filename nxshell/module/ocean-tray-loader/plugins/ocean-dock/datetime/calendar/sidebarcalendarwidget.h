// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SIDEBARCALENDARWIDGET_H
#define SIDEBARCALENDARWIDGET_H

#include "cweekwidget.h"
#include "datetitlewidget.h"
#include "calendarmanager.h"

#include <DLabel>
#include <DPushButton>
#include <DFontSizeManager>
#include <DCommandLinkButton>

#include <QWidget>
#include <QGridLayout>
#include <QDate>

DWIDGET_USE_NAMESPACE

class CalendarManager;
class SidebarCalendarKeyButton;
class JumpCalendarButton;
class RegionFormat;
//小日历类
class SidebarCalendarWidget : public QWidget
{
    enum WeekDay{
        Monday = 1,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday
    };
    enum Month{
        Jan = 1,
        Feb,
        Mar,
        Apr,
        May,
        Jun,
        Jul,
        Aug,
        Sept,
        Oct,
        Nov,
        Dec
    };

    Q_OBJECT
public:
    explicit SidebarCalendarWidget(RegionFormat *regionFormat, QWidget *parent = nullptr);
    //设置选中的日期
    void setSelectedDate(const QDate &date);

signals:
    void jumpButtonClicked();

private slots:
    //日期按键点击事件
    void onKeyButtonClicked(const QDate &date);
    //下一页按钮点击事件
    void onNextPageClicked();
    //上一页按钮点击事件
    void onPreviousPageClicked();
    //一周首日更新
    void onFirstDayChanged(int value);
    void onDateFormatChanged(CalendarManager::DateFormat format);
    void onWeekDayFormatChanged(CalendarManager::WeekDayFormat format);
    void backToday();
    void onShortDateFormatChanged();

protected:
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool event(QEvent *event) override;

private:
    void initView();
    void initConnection();
    void initData();
    void setDate(const QDate &date);
    void updateBackground();
    QString formatedWeekDay(WeekDay weekDay, CalendarManager::WeekDayFormat format);
    QString formatedMonth(Month month);
    QString formatedDateType(CalendarManager::DateFormat format);
    QString formatedWeekType(CalendarManager::WeekDayFormat format);

private:
    RegionFormat *m_regionFormat;
    CalendarManager *m_manager;
    QWidget* m_headWidget;                              //头部控件
    CWeekWidget* m_weekWidget;                          //周显示区域控件
    QWidget* m_keyWidget;                               //日期按钮区域控件
    QGridLayout* m_keyLayout;                           //按钮布局
    QLabel* m_dateLabel;                                //头部日期显示label
    DPushButton* m_nextPage;                            //下一页切换按键
    DPushButton* m_previousPage;                        //上一页切换按键
    QList<SidebarCalendarKeyButton*> m_keyButtonList;   //所有的日期按钮控件
    int m_firstday;
    DateTitleWidget *m_dateTitleWidget;
    DLabel *m_weekLabel;
    DLabel *m_leftdateLabel;
    DLabel *m_lunarLabel;
    DLabel *m_lunarDetailLabel;
    JumpCalendarButton *m_jumpCalendarButton;
    DCommandLinkButton *m_bakTodayBtn;
    QWidget *m_leftWidget;
    QWidget *m_rightWidget;
    QDate m_selectedData;
    QDate m_displayedMonth;
    CalendarManager::WeekDayFormat m_weekdayFormat;
    int m_deltaSum;
    QTimer *m_timer;
};


//日历按键类
class SidebarCalendarKeyButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SidebarCalendarKeyButton(QWidget *parent = nullptr);

    inline void setDate(const QDate& date) { m_date = date; update(); }
    inline QDate date() {return m_date; }
    inline bool isToday() { return m_isToday; }
    inline void setIsToday(bool isToday) { m_isToday = isToday; update(); }
    inline bool isThisMonth() { return m_isThisMonth; }
    inline void setIsThisMonth(bool isThisMonth) { m_isThisMonth = isThisMonth; update(); }
    inline bool isSelected() { return m_isSelected;}
    inline void setIsSelected(bool isSelected) { m_isSelected = isSelected; update(); }

signals:
    void clicked(const QDate &date);

public slots:
    void onClick();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    bool m_pressed;
    QDate m_date;
    bool m_isToday;
    bool m_isThisMonth;
    bool m_isSelected;
};

#endif // SIDEBARCALENDARWIDGET_H

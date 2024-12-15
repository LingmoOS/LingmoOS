// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAYMONTHVIEW_H
#define DAYMONTHVIEW_H
#include "customframe.h"
#include "huangliData/dbusdatastruct.h"
#include "cweekwidget.h"

#include <DWidget>
#include <DImageButton>
#include <DPushButton>
#include <DFrame>
#include <DIconButton>

#include <QList>
#include <QDate>
#include <QStyleOption>
#include <QGridLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class CDayHuangLiLabel;
class CTodayButton;
class CDayMonthWidget;
class CDayMonthView: public CustomFrame
{
    Q_OBJECT

public:
    explicit CDayMonthView(QWidget *parent = nullptr);
    ~CDayMonthView() override;
    //设置显示时间,选择时间和当前时间
    void setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate);
    void setLunarVisible(bool visible);
    void setTheMe(int type = 0);
    void setSearchFlag(bool flag);
    //设置黄历信息
    void setHuangLiInfo(const CaHuangLiDayInfo &huangLiInfo);
    void setHasScheduleFlag(const QVector<bool> &hasScheduleFlag);
signals:
    //判断是否为拖拽
    void signalIsDragging(bool &isDragging);
    //选择时间改变信号
    void signalChangeSelectDate(const QDate &date);
public slots:
    //上一个月
    void slotprev();
    //下一个月
    void slotnext();
    //返回当前时间
    void slottoday();
private:
    void initUI();
    void initConnection();
    void updateDateShow();
    void updateDateLunarDay();
private:
    void changeSelectDate(const QDate &date);
protected:
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *e) override;

private:
    DIconButton      *m_prevButton = nullptr;
    DIconButton      *m_nextButton = nullptr;
    CTodayButton *m_today = nullptr;
    CustomFrame *m_currentMouth = nullptr;
    CustomFrame *m_currentDay = nullptr;
    CustomFrame *m_currentYear = nullptr;
    CustomFrame *m_currentWeek = nullptr;
    CustomFrame *m_currentLuna = nullptr;
    CDayHuangLiLabel *m_yiLabel = nullptr;
    CDayHuangLiLabel *m_jiLabel = nullptr;
    QDate m_selectDate;
    QDate m_currentDate;
    int m_selectedCell = 0;
    QColor m_topBorderColor = Qt::red;
    QColor m_backgroundCircleColor = "#2ca7f8";
    QColor m_weekendsTextColor = Qt::black;
    QColor m_festivalTextColor = Qt::black;
    int m_firstWeekDay;
    bool m_huanglistate = true;
    QVBoxLayout *m_hhLayout = nullptr;
    QVBoxLayout *m_upLayout = nullptr;
    DHorizontalLine *m_splitline = nullptr;
    QVBoxLayout *m_yiDownLayout = nullptr;
    QVBoxLayout *m_jiDownLayout = nullptr;
    QStringList m_weeklist;
    CaHuangLiDayInfo m_huangLiInfo;
    CDayMonthWidget *m_dayMonthWidget;
    const int m_radius = 8;
    bool m_searchflag = false;

    CWeekWidget *m_weekWidget = nullptr; //周视图显示区域
};

class CDayMonthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CDayMonthWidget(QWidget *parent = nullptr);
    ~CDayMonthWidget() override;
    void setTheMe(int type = 0);
    //设置显示时间,选择时间和当前时间
    void setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate);
    void setHasScheduleFlag(const QVector<bool> &hasScheduleFlag);

private:
    const QString getCellDayNum(int pos);
    const QDate getCellDate(int pos);
    void paintCell(QWidget *cell);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
signals:
    //选择时间改变信号
    void signalChangeSelectDate(const QDate &date);
public slots:
    //点击月显示时间修改选择时间
    void cellClicked(QWidget *cell);
    //设置选择项修改选择时间
    void setSelectedCell(int index);

private:
    QList<QWidget *> m_cellList;
    QVector<QDate> m_showDays;
    int m_selectedCell = 0;
    QDate m_selectDate;
    QDate m_currentDate;
    QGridLayout *m_gridLayout = nullptr;
    QVector<bool> m_vlineflag; //节假日和日程标识

    QColor m_selectedTextColor = Qt::white;
    QColor m_currentDayTextColor = "#2ca7f8";
    QColor m_defaultTextColor = Qt::black;
    QColor m_notCurrentTextColor = "#b2b2b2";
    QColor m_ceventColor = "#FF5D00";
    QFont m_dayNumFont;
    bool m_isFocus;
    QPoint m_startPos;


    //拖拽新建日程相关处理流程
    int m_dayMouseState = 0;    //鼠标状态 0:原始状态，1:点击状态  2:移动状态
    int m_dayCreateState = 0;   //创建日程状态 0:不创建  1：创建   2: 超出界面范围


};
#endif // YEARVIEW_H

// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthwindow.h"
#include "monthview.h"
#include "monthdayview.h"
#include "constants.h"
#include "schedulesearchview.h"
#include "todaybutton.h"
#include "scheduledatamanage.h"

#include <DPalette>

#include <QMessageBox>
#include <QPainter>

DGUI_USE_NAMESPACE
CMonthWindow::CMonthWindow(QWidget *parent)
    : CScheduleBaseWidget(parent)
{
    initUI();
    initConnection();
    setLunarVisible(m_calendarManager->getShowLunar());
}

CMonthWindow::~CMonthWindow()
{
    disconnect(m_today, &CTodayButton::clicked, this, &CMonthWindow::slottoday);
    delete  m_monthDayView;
    m_monthDayView = nullptr;
    delete  m_monthView;
    m_monthView = nullptr;
}

/**
 * @brief setLunarVisible   设置是否显示阴历信息
 * @param state             是否显示阴历信息
 */
void CMonthWindow::setLunarVisible(bool state)
{
    m_monthView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
}

/**
 * @brief setTheMe  根据系统主题类型设置颜色
 * @param type      系统主题类型
 */
void CMonthWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {


        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);

        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    } else if (type == 2) {


        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#252525");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    }
    m_monthDayView->setTheMe(type);
    m_monthView->setTheMe(type);
}

void CMonthWindow::setYearData()
{
    if (getSelectDate() == getCurrendDateTime().date()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    if (getShowLunar()) {
        m_YearLabel->setText(QString::number(getSelectDate().year()) + tr("Y"));
    } else {
        m_YearLabel->setText(QString::number(getSelectDate().year()));
    }
}

/**
 * @brief CMonthWindow::updateShowDate  更新显示时间
 * @param isUpdateBar
 */
void CMonthWindow::updateShowDate(const bool isUpdateBar)
{
    setYearData();
    const QDate _selectDate = m_calendarManager->getSelectDate();
    Qt::DayOfWeek _firstWeek = m_calendarManager->getFirstDayOfWeek();
    m_monthView->setFirstWeekday(_firstWeek);
    QVector<QDate> _monthShowData = m_calendarManager->getMonthDate(_selectDate.year(), _selectDate.month());
    m_startDate = _monthShowData.first();
    m_stopDate = _monthShowData.last();
    m_monthView->setShowDate(_monthShowData);
    m_monthView->setRemindWidgetTimeFormat((m_calendarManager->getTimeShowType() ? "AP " : "") + m_calendarManager->getTimeFormat());
    if (isUpdateBar)
        m_monthDayView->setSelectDate(_selectDate); //设置12个月份显示
    //如果为中文环境则显示班休和农历信息
    if (getShowLunar()) {
        updateShowLunar();
    }
    //切换月份须更新显示日程
    updateShowSchedule();
}

/**
 * @brief CMonthWindow::setCurrentDateTime      设置当前时间
 * @param currentDate
 */
void CMonthWindow::setCurrentDateTime(const QDateTime &currentDate)
{
    CScheduleBaseWidget::setCurrentDateTime(currentDate);
    //更新当前时间
    m_monthView->setCurrentDate(currentDate.date());
}

/**
 * @brief CMonthWindow::updateShowSchedule  更新日程显示
 */
void CMonthWindow::updateShowSchedule()
{
    QMap<QDate, DSchedule::List> map = gScheduleManager->getScheduleMap(m_startDate, m_stopDate);
    //因获取的日程中只有有日程的项，数量不等于开始时间到结束时间的天数，
    //但是视图显示要求数量为开始时间到结束时间的天数，所以在没有日程的时间中添加空日期列表
    QDate date = m_startDate;
    QDate eDate = m_stopDate.addDays(1);
    while (date != eDate) {
        if (!map.contains(date)) {
            map[date] = DSchedule::List();
        }
        date = date.addDays(1);
    }
    m_monthView->setScheduleInfo(map);
}

/**
 * @brief CMonthWindow::updateShowLunar     更新显示农历信息
 */
void CMonthWindow::updateShowLunar()
{
    getLunarInfo();
    m_YearLunarLabel->setText(m_lunarYear);
    QMap<QDate, int> _monthFestivalInfo = gLunarManager->getFestivalInfoDateMap(m_startDate, m_stopDate);
    m_monthView->setFestival(_monthFestivalInfo);
    QMap<QDate, CaHuangLiDayInfo> _monthHuangLiInfo = gLunarManager->getHuangLiDayMap(m_startDate, m_stopDate);
    m_monthView->setHuangLiInfo(_monthHuangLiInfo);
}

/**
 * @brief CMonthWindow::updateSearchScheduleInfo        更新搜索日程信息
 */
void CMonthWindow::updateSearchScheduleInfo()
{
    //获取搜索日程信息
    m_monthView->setSearchScheduleInfo(gScheduleManager->getAllSearchedScheduleList());
}

/**
 * @brief CMonthWindow::setSelectSearchScheduleInfo     设置选中日程
 * @param info
 */
void CMonthWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
{
    m_monthView->setSelectSchedule(info);
}

/**
 * @brief CMonthWindow::setSearchWFlag      设置是否显示搜索界面
 * @param flag
 */
void CMonthWindow::setSearchWFlag(bool flag)
{
    m_searchFlag = flag;
    m_monthDayView->setSearchflag(flag);
}

/**
 * @brief CMonthWindow::deleteselectSchedule 快捷键删除日程
 */
void CMonthWindow::deleteselectSchedule()
{
    m_monthView->deleteSelectSchedule();
}

/**
 * @brief previousMonth 选择上一个月份
 */
void CMonthWindow::previousMonth()
{
    slideMonth(false);
}

/**
 * @brief nextMonth     选择下一个月份
 */
void CMonthWindow::nextMonth()
{
    slideMonth(true);
}

/**
 * @brief initUI 初始化界面
 */
void CMonthWindow::initUI()
{
    m_today = new CTodayButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEMonthCalendar::MTodayWindth, DDEMonthCalendar::MTodayHeight);

    QFont todayfont;
    todayfont.setWeight(QFont::Medium);
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEMonthCalendar::M_YLabelHeight);
    m_YearLunarLabel = new QLabel();
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunaLabelWindth, DDEMonthCalendar::M_YLunaLabelHeight);

    QFont yLabelF;
    yLabelF.setWeight(QFont::Medium);
    yLabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(yLabelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);

    yLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_YearLunarLabel->setFont(yLabelF);
    DPalette Lunarpa = m_YearLunarLabel->palette();
    Lunarpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_YearLunarLabel->setPalette(Lunarpa);

    m_monthDayView = new CMonthDayView(this);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_YearLabel);
    yeartitleLayout->addWidget(m_dialogIconButton);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);

    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout->addSpacing(6);
    yeartitleLayout->addLayout(yeartitleLayout1);

    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_monthDayView, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    m_monthView = new CMonthView(this);
    m_monthView->setObjectName("monthViewWidget");
    m_monthView->setAccessibleName("monthViewWidget");
    m_monthView->setCurrentDate(getCurrendDateTime().date());
    QVBoxLayout *mhLayout = new QVBoxLayout;
    mhLayout->setMargin(0);
    mhLayout->setSpacing(0);
    mhLayout->addWidget(m_monthView);

    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setSpacing(0);
    hhLayout->setMargin(0);

    //头部控件统一高度为 M_YTopHeight
    QWidget *top = new QWidget(this);
    top->setFixedHeight(DDEMonthCalendar::M_YTopHeight);
    top->setLayout(yeartitleLayout);
    hhLayout->addWidget(top);
    m_gridWidget = new DWidget();
    m_gridWidget->setContentsMargins(0, 0, 0, 0);
    m_gridWidget->setAutoFillBackground(true);
    m_gridWidget->setLayout(mhLayout);

    hhLayout->addWidget(m_gridWidget);

    m_tMainLayout = new QHBoxLayout;
    m_tMainLayout->setMargin(0);
    m_tMainLayout->setSpacing(0);
    m_tMainLayout->addLayout(hhLayout);

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    m_tMainLayout->addLayout(ssLayout);

    this->setLayout(m_tMainLayout);
}

/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CMonthWindow::initConnection()
{
    connect(m_today, &DPushButton::clicked, this, &CMonthWindow::slottoday);
    connect(m_monthDayView, &CMonthDayView::signalsSelectDate, this, &CMonthWindow::slotSetSelectDate);
    //双击时间修改选择时间和切换到日视图
    connect(m_monthView, &CMonthView::signalsViewSelectDate, this, &CMonthWindow::slotViewSelectDate);
    connect(m_monthView, &CMonthView::signalAngleDelta, this, &CMonthWindow::slotAngleDelta);
    //月份控件区域左右滚动信号关联
    connect(m_monthDayView, &CMonthDayView::signalAngleDelta, this, &CMonthWindow::slotAngleDelta);
    connect(m_monthView, &CMonthView::signalSwitchPrePage, this, &CMonthWindow::slotSwitchPrePage);
    connect(m_monthView, &CMonthView::signalSwitchNextPage, this, &CMonthWindow::slotSwitchNextPage);
}

/**
 * @brief slideMonth    切换月份，并更新信息
 * @param next          是否切换到下一个月
 */
void CMonthWindow::slideMonth(bool next)
{
    slotScheduleHide();

    if (m_isSwitchStatus)
        return;

    m_isSwitchStatus = true;

    QTimer::singleShot(5, [this, next]() {
        if (next) {
            setSelectDate(getSelectDate().addMonths(-1), true);
        } else {
            setSelectDate(getSelectDate().addMonths(1), true);
        }
        //更新日程
        updateShowDate();

        m_isSwitchStatus = false;
    });
}

/**
 * @brief slotScheduleHide 隐藏日程浮框
 */
void CMonthWindow::slotScheduleHide()
{
    m_monthView->slotScheduleRemindWidget(false);
}

/**
 * @brief slotAngleDelta    接受滚动事件滚动相对量
 * @param delta             滚动相对量
 */
void CMonthWindow::slotAngleDelta(int delta)
{
    //拖拽时禁用
    if (!m_monthView->isDragging()) {
        if (delta > 0) {
            //下一个月
            nextMonth();
        } else {
            //上一个月
            previousMonth();
        }
    }
}

/**
 * @brief CMonthWindow::slotViewSelectDate          设置选择时间切换日视图
 * @param date
 */
void CMonthWindow::slotViewSelectDate(const QDate &date)
{
    slotScheduleHide();
    if (setSelectDate(date, true)) {
        //更新界面
        updateData();
        emit signalSwitchView(3);
    }
}

void CMonthWindow::slotSwitchPrePage()
{
    nextMonth();
}

void CMonthWindow::slotSwitchNextPage()
{
    previousMonth();
}

/**
 * @brief resizeEvent                   窗口大小调整
 * @param event                         窗口大小调整事件
 */
void CMonthWindow::resizeEvent(QResizeEvent *event)
{
    qreal dw = width() * 0.5023 + 0.5;
    int dh = 36;
    m_monthDayView->setFixedSize(qRound(dw), dh);
    QWidget::resizeEvent(event);
}

/**
 * @brief CMonthWindow::slottoday               返回当前时间
 */
void CMonthWindow::slottoday()
{
    //隐藏提示框
    slotScheduleHide();
    setSelectDate(getCurrendDateTime().date(), true);
    updateData();
}

/**
 * @brief CMonthWindow::slotSetSelectDate       设置选择时间
 * @param date
 */
void CMonthWindow::slotSetSelectDate(const QDate &date)
{
    slotScheduleHide();
    if (setSelectDate(date, true)) {
        updateShowDate(false);
    }
}

// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daywindow.h"
#include "constants.h"
#include "customframe.h"
#include "daymonthview.h"
#include "schedulesearchview.h"
#include "scheduleview.h"

#include <DPalette>

#include <QHBoxLayout>
#include <QMessageBox>
#include <QDrag>
#include <QMimeData>

DGUI_USE_NAMESPACE
CDayWindow::CDayWindow(QWidget *parent)
    : CScheduleBaseWidget(parent)
{
    initUI();
    initConnection();
    setLunarVisible(m_calendarManager->getShowLunar());
}

CDayWindow::~CDayWindow() {}
void CDayWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_leftground->setBColor("#FFFFFF");

        DPalette ypa = m_YearLabel->palette();
        ypa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette lpa = m_LunarLabel->palette();
        lpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette spa = m_SolarDay->palette();
        spa.setColor(DPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        m_leftground->setBColor("#282828");

        DPalette ypa = m_YearLabel->palette();
        ypa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette lpa = m_LunarLabel->palette();
        lpa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette spa = m_SolarDay->palette();
        spa.setColor(DPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(DPalette::WindowText);
    }
    m_daymonthView->setTheMe(type);
    m_scheduleView->setTheMe(type);
}

/**
 * @brief CDayWindow::setTime           设置显示时间位置
 * @param time
 */
void CDayWindow::setTime(const QTime time)
{
    if (time.isValid()) {
        //如果时间有效
        m_scheduleView->setTime(time);
    } else {
        if (getSelectDate() == getCurrendDateTime().date()) {
            m_scheduleView->setTime(getCurrendDateTime().time());
        } else {
            m_scheduleView->setTime(m_makeTime);
        }
    }
}

/**
 * @brief CDayWindow::updateHeight            更新全天和非全天高度
 */
void CDayWindow::updateHeight()
{
    m_scheduleView->updateHeight();
}

/**
 * @brief CDayWindow::setCurrentDateTime    设置当前时间
 * @param currentDate
 */
void CDayWindow::setCurrentDateTime(const QDateTime &currentDate)
{
    //设置当前时间
    CScheduleBaseWidget::setCurrentDateTime(currentDate);
    //更新当前时间
    m_scheduleView->setCurrentDate(getCurrendDateTime());
}

/**
 * @brief CDayWindow::setYearData   设置选择时间年信息显示
 */
void CDayWindow::setYearData()
{
    QLocale locale;
    //判断是否为中文环境
    if (getShowLunar()) {
        m_YearLabel->setText(QString::number(getSelectDate().year()) + tr("Y") +
                             QString::number(getSelectDate().month()) + tr("M") +
                             QString::number(getSelectDate().day()) + tr("D"));
    } else {
        m_YearLabel->setText(locale.toString(getSelectDate(), "yyyy/M/d"));
    }
}

/**
 * @brief CDayWindow::updateShowDate        更新显示时间
 * @param isUpdateBar
 */
void CDayWindow::updateShowDate(const bool isUpdateBar)
{
    Q_UNUSED(isUpdateBar)
    setYearData();
    int w = m_scheduleView->width() - 72;
    m_scheduleView->setRange(w, 1032, getSelectDate(), getSelectDate());
    //设置时间显示格式
    m_scheduleView->setTimeFormat((m_calendarManager->getTimeShowType() ? "AP " : "") + m_calendarManager->getTimeFormat());
    //获取需要显示的时间
    QVector<QDate> _monthDate = m_calendarManager->getMonthDate(getSelectDate().year(), getSelectDate().month());
    m_daymonthView->setShowDate(_monthDate, getSelectDate(), getCurrendDateTime().date());
    //如果为中文环境则显示农历信息
    if (getShowLunar())
        updateShowLunar();
    updateShowSchedule();
}

/**
 * @brief CDayWindow::updateShowSchedule    更新日程显示
 */
void CDayWindow::updateShowSchedule()
{
    //获取一天的日程信息
    QMap<QDate, DSchedule::List> _weekScheduleInfo = gScheduleManager->getScheduleMap(getSelectDate(), getSelectDate());
    //设置显示日程信息
    m_scheduleView->setShowScheduleInfo(_weekScheduleInfo);
    //获取界面显示定位时间位置
    setMakeTime(_weekScheduleInfo);

    QSet<QDate> scheduleDate = gScheduleManager->getAllScheduleDate();
    QVector<bool> monthFlag{};
    QVector<QDate> monthDate = m_calendarManager->getMonthDate(getSelectDate().year(), getSelectDate().month());

    for (QDate date : monthDate) {
        monthFlag.push_back((scheduleDate.find(date) != scheduleDate.end()));
    }
    //设置日视图右侧月显示日期是否有日程
    m_daymonthView->setHasScheduleFlag(monthFlag);
}

/**
 * @brief CDayWindow::updateShowLunar       更新显示农历信息
 */
void CDayWindow::updateShowLunar()
{
    CaHuangLiDayInfo _huangLiInfo = getLunarInfo();
    m_LunarLabel->setText(tr("Lunar") + _huangLiInfo.mLunarMonthName + _huangLiInfo.mLunarDayName);
    m_daymonthView->setHuangLiInfo(_huangLiInfo);
}

void CDayWindow::updateSearchScheduleInfo()
{
    m_scheduleView->slotUpdateScene();
}

/**
 * @brief CDayWindow::setSelectSearchScheduleInfo       设置选中日程
 * @param info
 */
void CDayWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
{
    if (info->allDay()) {
        setTime();
    } else {
        m_scheduleView->setTime(info->dtStart().time());
    }

    m_scheduleView->setSelectSchedule(info);
}

/**
 * @brief CDayWindow::deleteselectSchedule 快捷键删除日程
 */
void CDayWindow::deleteselectSchedule()
{
    m_scheduleView->slotDeleteitem();
}

void CDayWindow::setSearchWFlag(bool flag)
{
    m_searchFlag = flag;
    m_daymonthView->setSearchFlag(flag);
}

void CDayWindow::setLunarVisible(bool state)
{
    m_LunarLabel->setVisible(state);
    m_SolarDay->setVisible(state);
    m_scheduleView->setLunarVisible(state);
    m_daymonthView->setLunarVisible(state);
}

void CDayWindow::initUI()
{
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 9, 0, 3);

    m_YearLabel = new QLabel();
    m_YearLabel->setMinimumHeight(DDEDayCalendar::D_YLabelHeight);
    QFont labelF;
    labelF.setWeight(QFont::Medium);
    labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(labelF);
    DPalette ypa = m_YearLabel->palette();
    ypa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(ypa);
    titleLayout->addWidget(m_YearLabel);
    m_dialogIconButton->setFocusPolicy(Qt::NoFocus);
    titleLayout->addWidget(m_dialogIconButton);
    m_LunarLabel = new QLabel();
    titleLayout->addSpacing(15);
    m_LunarLabel->setFixedHeight(DDEDayCalendar::D_YLabelHeight);
    labelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_LunarLabel->setFont(labelF);
    m_LunarLabel->setAlignment(Qt::AlignCenter);
    DPalette lpa = m_LunarLabel->palette();
    lpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_LunarLabel->setPalette(lpa);
    titleLayout->addWidget(m_LunarLabel);
    m_SolarDay = new QLabel();
    labelF.setPixelSize(DDECalendar::FontSizeTen);
    m_SolarDay->setFixedHeight(DDEDayCalendar::D_YLabelHeight);
    m_SolarDay->setFont(labelF);
    m_SolarDay->setAlignment(Qt::AlignCenter);
    DPalette spa = m_SolarDay->palette();
    spa.setColor(DPalette::WindowText, Qt::red);
    m_SolarDay->setPalette(spa);
    titleLayout->addWidget(m_SolarDay);
    titleLayout->addStretch();

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    m_scheduleView = new CScheduleView(this, ScheduleViewPos::DayPos);
    m_scheduleView->setviewMargin(72, 109, 20, 0);
    m_scheduleView->setCurrentDate(getCurrendDateTime());
    leftLayout->addLayout(titleLayout);
    leftLayout->addWidget(m_scheduleView);

    m_verline = new DVerticalLine;
    m_verline->setFixedWidth(2);

    m_daymonthView = new CDayMonthView(this);

    QHBoxLayout *leftMainLayout = new QHBoxLayout;
    leftMainLayout->setMargin(0);
    leftMainLayout->setSpacing(1);
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->addLayout(leftLayout);
    leftMainLayout->addWidget(m_verline);
    leftMainLayout->addWidget(m_daymonthView);

    leftMainLayout->setStretchFactor(leftLayout, 3);
    leftMainLayout->setStretchFactor(m_verline, 1);
    leftMainLayout->setStretchFactor(m_daymonthView, 2);

    m_leftground = new CustomFrame();
    m_leftground->setRoundState(true, true, true, true);
    m_leftground->setLayout(leftMainLayout);
    m_leftground->setBColor("#FFFFFF");

    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_leftground);

    this->setLayout(m_mainLayout);
}

void CDayWindow::initConnection()
{
    connect(m_daymonthView, &CDayMonthView::signalIsDragging, this, &CDayWindow::slotIsDragging);
    connect(m_daymonthView, &CDayMonthView::signalChangeSelectDate, this, &CDayWindow::slotChangeSelectDate);
    connect(m_scheduleView, &CScheduleView::signalSwitchPrePage, this, &CDayWindow::slotSwitchPrePage);
    connect(m_scheduleView, &CScheduleView::signalSwitchNextPage, this, &CDayWindow::slotSwitchNextPage);
}

/**
 * @brief CDayWindow::setMakeTime       界面显示定位时间位置
 * @param info
 */
void CDayWindow::setMakeTime(QMap<QDate, DSchedule::List> &info)
{
    if (info.contains(getSelectDate())) {
        DSchedule::List _scheduleVector = info[getSelectDate()];
        //设置当前第一个非全天默认时间
        QDateTime firstscheduleBeginTime(getSelectDate().addDays(1), QTime(0, 0, 0));
        //获取非全天日程
        for (int i = 0 ; i < _scheduleVector.size(); ++i) {
            if (!_scheduleVector.at(i)->allDay()) {
                if (firstscheduleBeginTime > _scheduleVector.at(i)->dtStart()) {
                    firstscheduleBeginTime = _scheduleVector.at(i)->dtStart();
                }
            }
        }
        //如果为默认时间则表示当天没有非全天日程
        if (firstscheduleBeginTime.date() > getSelectDate()) {
            m_makeTime.setHMS(12, 30, 0, 0);
        } else if (firstscheduleBeginTime.date() == getSelectDate()) {
            m_makeTime = firstscheduleBeginTime.time();
            //设置定位的时间位置,原始时间太靠下,现向上偏移2小时
            QTime onTime = m_makeTime;
            if (onTime.hour() + 4 >= 24) {
                onTime = QTime(20, 0);
            } else {
                onTime = onTime.addSecs(14400);
            }
            m_makeTime = onTime;
        } else {
            m_makeTime.setHMS(0, 0, 0, 0);
        }
    } else {
        //将定位的默认时间显示出8点
        m_makeTime.setHMS(12, 30, 0, 0);
    }
}

void CDayWindow::slotScheduleHide()
{
    m_scheduleView->slotScheduleShow(false);
}

/**
 * @brief CDayWindow::slotChangeSelectDate          更新选择时间
 * @param date
 */
void CDayWindow::slotChangeSelectDate(const QDate &date)
{
    if (setSelectDate(date, true)) {
        //隐藏日程悬浮框
        slotScheduleHide();
        updateShowDate();
        setTime();
    }
}

void CDayWindow::slotIsDragging(bool &isDragging)
{
    isDragging = m_scheduleView->IsDragging();
}

/**
 * @brief CDayWindow::slotSwitchPrePage     切换选择时间
 */
void CDayWindow::slotSwitchPrePage()
{
    slotChangeSelectDate(getSelectDate().addDays(-1));
}

/**
 * @brief CDayWindow::slotSwitchNextPage    切换选择时间
 */
void CDayWindow::slotSwitchNextPage()
{
    slotChangeSelectDate(getSelectDate().addDays(1));
}

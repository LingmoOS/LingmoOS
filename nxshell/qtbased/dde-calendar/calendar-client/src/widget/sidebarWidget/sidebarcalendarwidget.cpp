// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sidebarcalendarwidget.h"
#include "cschedulebasewidget.h"
#include "constants.h"
#include "units.h"

#include <DApplicationHelper>
#include <QMouseEvent>

QDate SidebarCalendarKeyButton::m_selectedData = QDate();
QDate SidebarCalendarKeyButton::m_displayedMonth = QDate();


SidebarCalendarWidget::SidebarCalendarWidget(QWidget *parent) : QWidget(parent)
{
    initView();
    initConnection();
    initData();
}

void SidebarCalendarWidget::initView()
{
    m_dateLabel = new QLabel();
    QFont font = m_dateLabel->font();
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    m_dateLabel->setFont(font);
    m_nextPage = new QPushButton();
    m_previousPage = new QPushButton();
    m_nextPage->setIcon(DStyle().standardIcon(QStyle::SP_ArrowRight));
    m_previousPage->setIcon(DStyle().standardIcon(QStyle::SP_ArrowLeft));
    m_nextPage->setFixedSize(20, 20);
    m_previousPage->setFixedSize(20, 20);
    m_nextPage->setIconSize(QSize(10,10));
    m_previousPage->setIconSize(QSize(10,10));
    m_nextPage->setFlat(true);
    m_previousPage->setFlat(true);

    m_nextPage->setFocusPolicy(Qt::NoFocus);
    m_previousPage->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->setMargin(0);
    headLayout->setAlignment(Qt::AlignCenter);
    headLayout->addWidget(m_previousPage);
    headLayout->addWidget(m_dateLabel);
    headLayout->addWidget(m_nextPage);

    m_headWidget = new QWidget(this);
    m_headWidget->setLayout(headLayout);
    m_headWidget->setMinimumHeight(30);

    m_weekWidget = new CWeekWidget(this);
    m_weekWidget->setAutoFontSizeByWindow(false);
    m_weekWidget->setFirstDay(Qt::Sunday);

    m_keyWidget = new QWidget(this);
    m_keyLayout = new QGridLayout();
    m_keyLayout->setMargin(0);
    m_keyLayout->setSpacing(0);
    m_keyWidget->setLayout(m_keyLayout);
    //循坏实例化6*7个日期按键
    for (int i = 0; i < 42; ++i) {
        SidebarCalendarKeyButton *button = new SidebarCalendarKeyButton();
        button->setFocusPolicy(Qt::NoFocus);
        m_keyButtonList.push_back(button);
        m_keyLayout->addWidget(button, i/7, i%7, 1, 1, Qt::AlignCenter);
        connect(button, &SidebarCalendarKeyButton::signaClicked, this, &SidebarCalendarWidget::slotKeyButtonClicked);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    this->setLayout(mainLayout);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(m_headWidget);
    mainLayout->addWidget(m_weekWidget, 1);
    mainLayout->addWidget(m_keyWidget, 6);
}

/**
 * @brief SidebarCalendarWidget::initConnection
 * 初始化信号链接
 */
void SidebarCalendarWidget::initConnection()
{
    connect(m_nextPage, &QPushButton::clicked, this, &SidebarCalendarWidget::slotNextPageClicked);
    connect(m_previousPage, &QPushButton::clicked, this, &SidebarCalendarWidget::slotPreviousPageClicked);
    connect(CalendarManager::getInstance(), &CalendarManager::sigNotifySidebarFirstDayChanged, this, &SidebarCalendarWidget::slotFirstDayChanged);
}

/**
 * @brief SidebarCalendarWidget::initData
 * 初始化数据
 */
void SidebarCalendarWidget::initData()
{
    m_firstday = CalendarManager::getInstance()->getFirstDayOfWeek();
    QDate date = QDate::currentDate();
    setDate(date);
}

/**
 * @brief SidebarCalendarWidget::setDate
 * 设置显示的日期
 * @param date 将被选择显示的日期
 */
void SidebarCalendarWidget::setDate(QDate& date)
{
    SidebarCalendarKeyButton::setSelectedDate(date);
    setKeyDate(date);
}

/**
 * @brief SidebarCalendarWidget::setKeyDate
 * 根据选中的日期设置按钮显示的日期范围
 * @param date 待显示的月日期
 */
void SidebarCalendarWidget::setKeyDate(QDate date)
{
    if(!withinTimeFrame(date)){
        return;
    }
    QString fd = "";
    fd.append("yyyy").append(tr("Y")).append(" ").append("MM").append(tr("M"));
    m_dateLabel->setText(date.toString(fd));
    SidebarCalendarKeyButton::setDisplayedMonth(date);

    //获取当月第一天
    date = QDate(date.year(), date.month(), 1);
//    int firstday = CalendarManager::getInstance()->getFirstDayOfWeek();
    int day = date.dayOfWeek();
    //计算当前月日历第一天该显示的时间
    date = date.addDays(-(day - m_firstday + 7) % 7);
    for (SidebarCalendarKeyButton* btn : m_keyButtonList) {
        btn->setDate(date);
        date = date.addDays(1);
    }
    update();
}

/**
 * @brief SidebarCalendarWidget::slotKeyButtonClicked
 * 日期按键点击事件
 * @param keyButton 事件发送者
 */
void SidebarCalendarWidget::slotKeyButtonClicked(SidebarCalendarKeyButton* keyButton)
{
    QDate date = keyButton->getSelectedDate();
    if(!withinTimeFrame(date)){
        return;
    }
    if (date.year() ==keyButton->getDisplayedMonth().year() && date.month() == keyButton->getDisplayedMonth().month()) {
        //未切换月份，只刷新界面显示
        update();
    } else {
        //已切换月份，重新设置选中的日期
        setDate(date);
    }
    CScheduleBaseWidget::setSelectDate(date);
}

/**
 * @brief SidebarCalendarWidget::slotNextPageClicked
 * 下一页切换事件
 */
void SidebarCalendarWidget::slotNextPageClicked()
{
    //设置显示月份日期
    QDate date = SidebarCalendarKeyButton::getDisplayedMonth().addMonths(+1);
    //设置显示的日期范围
    setKeyDate(date);
}

/**
 * @brief SidebarCalendarWidget::slotPreviousPageClicked
 * 上一页切换事件
 */
void SidebarCalendarWidget::slotPreviousPageClicked()
{
    //设置显示月份日期
    QDate date = SidebarCalendarKeyButton::getDisplayedMonth().addMonths(-1);
    //设置显示的日期范围
    setKeyDate(date);
}

void SidebarCalendarWidget::slotFirstDayChanged(int value)
{
    m_firstday = value;
    setKeyDate(SidebarCalendarKeyButton::getSelectedDate());
}

SidebarCalendarKeyButton::SidebarCalendarKeyButton(QWidget *parent)
    : QPushButton(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(QSize(10, 10));
}

/**
 * @brief SidebarCalendarKeyButton::setSelectedDate
 * 设置被选中的日期
 * @param date 将被选中的日期
 */
void SidebarCalendarKeyButton::setSelectedDate(const QDate& date)
{
    m_selectedData = date;
}

/**
 * @brief SidebarCalendarKeyButton::getSelectedDate
 * @return 当前被选中的日期
 */
QDate SidebarCalendarKeyButton::getSelectedDate()
{
    return m_selectedData;
}

/**
 * @brief SidebarCalendarKeyButton::setDisplayedMonth
 * 设置显示的月份
 * @param date 将显示的月份日期
 */
void SidebarCalendarKeyButton::setDisplayedMonth(const QDate& date)
{
    m_displayedMonth = date;
}

/**
 * @brief SidebarCalendarKeyButton::getDisplayedMonth
 * @return 当前显示的月份日期
 */
QDate SidebarCalendarKeyButton::getDisplayedMonth()
{
    return m_displayedMonth;
}

/**
 * @brief SidebarCalendarKeyButton::setDate
 * 设置被显示的日期
 * @param date 将被显示的日期
 */
void SidebarCalendarKeyButton::setDate(const QDate& date)
{
    this->m_displayedDate = date;
    update();
}

/**
 * @brief SidebarCalendarKeyButton::getDate
 * @return 当前显示的日期
 */
QDate SidebarCalendarKeyButton::getDate()
{
    return m_displayedDate;
}

/**
 * @brief SidebarCalendarKeyButton::click
 * 点击事件
 */
void SidebarCalendarKeyButton::click()
{
    if(!withinTimeFrame(m_displayedDate)){
        return;
    }
    m_selectedData = m_displayedDate;
    emit signaClicked(this);
}

void SidebarCalendarKeyButton::mousePressEvent(QMouseEvent *event)
{
    //判断鼠标左键按下事件
    if (event->button() == Qt::LeftButton) {
        this->pressed = true;
    }
    QWidget::mousePressEvent(event);
}

void SidebarCalendarKeyButton::mouseReleaseEvent(QMouseEvent *event)
{
    //判断鼠标左键释放事件
    if (pressed && event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        click();
        pressed = false;
    } else if (event->button() == Qt::LeftButton) {
        pressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void SidebarCalendarKeyButton::mouseMoveEvent(QMouseEvent *event)
{
    //判断鼠标左键移动事件
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        pressed = true;
    } else if (event->button() == Qt::LeftButton) {
        pressed = false;
    }
}

void SidebarCalendarKeyButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);


    qreal w = this->width();
    qreal h = this->height();
    const qreal r = w > h ? h: w;
    QRectF rectf(qRound((w-r)/2), qRound((h-r)/2), r, r);   //绘制区域

    QFont font;
    font.setPixelSize(DDECalendar::FontSizeTwelve);
    painter.setFont(font);

    if (m_displayedDate == m_selectedData && withinTimeFrame(m_displayedDate)) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(DApplicationHelper::instance()->palette(this).highlight());
        //绘制高亮背景
        painter.drawEllipse(rectf);
        //设置高亮下的字体颜色
        painter.setPen(DApplicationHelper::instance()->palette(this).highlightedText().color());
    } else if (!(m_displayedDate.year() == m_displayedMonth.year() && m_displayedDate.month() == m_displayedMonth.month())){
        //设置正常显示状态下的字体颜色
        painter.setOpacity(0.3);
    }
    //获取待绘制的文字
    QString text = QString::number(m_displayedDate.day());

    //绘制文字
    painter.drawText(rectf, text, QTextOption(Qt::AlignCenter));
    QWidget::paintEvent(event);
}


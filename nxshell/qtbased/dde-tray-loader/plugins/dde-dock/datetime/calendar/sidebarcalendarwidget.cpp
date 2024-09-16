// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sidebarcalendarwidget.h"
#include "constants.h"
#include "units.h"
#include "jumpcalendarbutton.h"
#include "regionFormat.h"

#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DRegionMonitor>
#include <DToolTip>

#include <QMouseEvent>
#include <QScreen>
#include <QJsonObject>

SidebarCalendarWidget::SidebarCalendarWidget(RegionFormat *regionFormat, QWidget* parent)
    : QWidget(parent)
    , m_regionFormat(regionFormat)
    , m_manager(CalendarManager::instance())
    , m_headWidget(new QWidget(this))
    , m_weekWidget(new CWeekWidget(this))
    , m_keyWidget(new QWidget(this))
    , m_keyLayout(new QGridLayout)
    , m_dateLabel(new QLabel(this))
    , m_nextPage(new DPushButton(this))
    , m_previousPage(new DPushButton(this))
    , m_firstday(Qt::Sunday)
    , m_dateTitleWidget(new DateTitleWidget(this))
    , m_weekLabel(new DLabel(this))
    , m_leftdateLabel(new DLabel(this))
    , m_lunarLabel(new DLabel(this))
    , m_lunarDetailLabel(new DLabel(this))
    , m_jumpCalendarButton(new JumpCalendarButton(this))
    , m_bakTodayBtn(new DCommandLinkButton(tr("Today"), this))
    , m_leftWidget(new QWidget(this))
    , m_rightWidget(new QWidget(this))
    , m_selectedData(QDate())
    , m_weekdayFormat(CalendarManager::WeekDayFormat_1)
    , m_deltaSum(0)
    , m_timer(new QTimer(this))
{
    initView();
    initConnection();
    initData();
}

void SidebarCalendarWidget::initView()
{
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(500, 300);
    QVBoxLayout *leftLayout = new QVBoxLayout;
    QVBoxLayout *rightLayout = new QVBoxLayout;

    DFontSizeManager::instance()->bind(m_weekLabel, DFontSizeManager::T5, 70);
    DFontSizeManager::instance()->bind(m_leftdateLabel, DFontSizeManager::T5, 70);
    DFontSizeManager::instance()->bind(m_lunarLabel, DFontSizeManager::T6, 62);
    DFontSizeManager::instance()->bind(m_lunarDetailLabel, DFontSizeManager::T6, 62);

    m_jumpCalendarButton->setIcon(QIcon::fromTheme("open-calendar"));
    m_jumpCalendarButton->setDescription(tr("Open the calendar"));
    m_jumpCalendarButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_jumpCalendarButton->setFixedWidth(m_jumpCalendarButton->sizeHint().width());
    connect(qApp, &QApplication::fontChanged, this, [this]() {
        m_jumpCalendarButton->setFixedWidth(m_jumpCalendarButton->sizeHint().width());
        update();
    });

    m_lunarDetailLabel->setElideMode(Qt::TextElideMode::ElideRight);
    DToolTip::setToolTipShowMode(m_lunarDetailLabel, DToolTip::ShowWhenElided);

    leftLayout->setSpacing(0);
    leftLayout->addStretch(1);
    leftLayout->addWidget(m_dateTitleWidget);
    leftLayout->addStretch(2);
    leftLayout->addWidget(m_weekLabel);
    leftLayout->addWidget(m_leftdateLabel);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(m_lunarLabel);
    leftLayout->addWidget(m_lunarDetailLabel);
    leftLayout->addSpacing(5);
    leftLayout->addWidget(m_jumpCalendarButton);
    leftLayout->setContentsMargins(20, 0, 10, 20);

    DFontSizeManager::instance()->bind(m_dateLabel, DFontSizeManager::T5, 65);
    m_nextPage->setIcon(DStyle().standardIcon(QStyle::SP_ArrowRight));
    m_previousPage->setIcon(DStyle().standardIcon(QStyle::SP_ArrowLeft));
    m_nextPage->setIconSize(QSize(15, 15));
    m_previousPage->setIconSize(QSize(15, 15));
    m_nextPage->setMinimumSize(30, 30);
    m_previousPage->setMinimumSize(30, 30);
    m_nextPage->setFlat(true);
    m_previousPage->setFlat(true);

    m_nextPage->setFocusPolicy(Qt::NoFocus);
    m_previousPage->setFocusPolicy(Qt::NoFocus);
    m_bakTodayBtn->setFocusPolicy(Qt::NoFocus);

    DFontSizeManager::instance()->bind(m_bakTodayBtn, DFontSizeManager::T6);

    QHBoxLayout* headLayout = new QHBoxLayout;
    headLayout->setContentsMargins(8, 0, 0, 0);
    headLayout->setAlignment(Qt::AlignCenter);
    headLayout->addWidget(m_dateLabel);
    headLayout->addStretch();
    headLayout->addWidget(m_bakTodayBtn);
    headLayout->addWidget(m_previousPage);
    headLayout->addWidget(m_nextPage);

    m_headWidget->setLayout(headLayout);
    m_headWidget->setMinimumHeight(30);

    m_weekWidget->setAutoFontSizeByWindow(false);
    m_weekWidget->setFirstDay(Qt::Sunday);
    m_weekWidget->setMinimumHeight(20);
    DFontSizeManager::instance()->bind(m_weekWidget, DFontSizeManager::T6, 60);

    m_keyLayout->setMargin(0);
    m_keyLayout->setSpacing(0);
    m_keyWidget->setLayout(m_keyLayout);
    //循坏实例化6*7个日期按键
    for (int i = 0; i < MONTH_METRIC_NUM; ++i) {
        SidebarCalendarKeyButton* button = new SidebarCalendarKeyButton();
        button->setFocusPolicy(Qt::NoFocus);
        m_keyButtonList.push_back(button);
        m_keyLayout->addWidget(button, i / DAYS_INWEEK, i % DAYS_INWEEK, 1, 1, Qt::AlignCenter);
        connect(button, &SidebarCalendarKeyButton::clicked, this, &SidebarCalendarWidget::onKeyButtonClicked);
    }

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);

    rightLayout->addSpacing(6);
    rightLayout->addWidget(m_headWidget);
    rightLayout->addWidget(m_weekWidget, 1);
    rightLayout->addWidget(m_keyWidget, 6);
    rightLayout->setContentsMargins(5, 5, 15, 15);

    // 为设置左右比例和固定大小，使用widget包起来
    m_leftWidget->setLayout(leftLayout);
    m_rightWidget->setLayout(rightLayout);
    m_leftWidget->setMinimumWidth(230);
    updateBackground();

    mainLayout->addWidget(m_leftWidget);
    mainLayout->addWidget(m_rightWidget);
    setLayout(mainLayout);

    m_timer->setInterval(1000);
    m_timer->start();
}

/**
 * @brief SidebarCalendarWidget::initConnection
 * 初始化信号链接
 */
void SidebarCalendarWidget::initConnection()
{
    connect(m_nextPage, &QPushButton::clicked, this, &SidebarCalendarWidget::onNextPageClicked);
    connect(m_previousPage, &QPushButton::clicked, this, &SidebarCalendarWidget::onPreviousPageClicked);
    connect(m_manager, &CalendarManager::sidebarFirstDayChanged, this, &SidebarCalendarWidget::onFirstDayChanged);
    // connect(m_manager, &CalendarManager::dateFormatChanged, this, &SidebarCalendarWidget::onDateFormatChanged);
    connect(m_regionFormat, &RegionFormat::shortDateFormatChanged, this, &SidebarCalendarWidget::onShortDateFormatChanged);
    connect(m_manager, &CalendarManager::weekDayFormatChanged, this, &SidebarCalendarWidget::onWeekDayFormatChanged);
    connect(m_bakTodayBtn, &DCommandLinkButton::clicked, this, &SidebarCalendarWidget::backToday);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &SidebarCalendarWidget::updateBackground);
    connect(m_jumpCalendarButton, &JumpCalendarButton::clicked, this, &SidebarCalendarWidget::jumpButtonClicked);
    connect(m_timer, &QTimer::timeout, this, [this] {
        if (QTime::currentTime().hour() != QTime(0, 0, 0).hour()
                || QTime::currentTime().minute() != QTime(0, 0, 0).minute()
                || QTime::currentTime().second() != QTime(0, 0, 0).second())
            return;

        backToday();
    });
}

/**
 * @brief SidebarCalendarWidget::initData
 * 初始化数据
 */
void SidebarCalendarWidget::initData()
{
    m_firstday = m_manager->firstDayOfWeek();
    m_weekdayFormat = m_manager->weekDayFormat();
    QDate date = QDate::currentDate();
    setSelectedDate(date);
}

/**
 * @brief SidebarCalendarWidget::setDate
 * 设置显示的日期
 * @param date 将被选择显示的日期
 */
void SidebarCalendarWidget::setSelectedDate(const QDate &date)
{
    if(m_selectedData == date)
        return;

    m_selectedData = date;
    setDate(date);
}

/**
 * @brief SidebarCalendarWidget::setKeyDate
 * 根据选中的日期设置按钮显示的日期范围
 * @param date 待显示的月日期
 */
void SidebarCalendarWidget::setDate(const QDate &date)
{
    if (!withinTimeFrame(date)) {
        return;
    }

    m_bakTodayBtn->setVisible(date.month() != QDate::currentDate().month() || date.year() != QDate::currentDate().year());

    auto instance = LunarManager::instace()->huangLiDay(date);
    m_dateTitleWidget->setDateLabelText("/ " + formatedMonth(static_cast<Month>(date.month())), date.day());
    m_weekLabel->setText(formatedWeekDay(static_cast<WeekDay>(date.dayOfWeek()), m_weekdayFormat));
    m_leftdateLabel->setText(date.toString(m_regionFormat->originShortDateFormat()));
    m_lunarLabel->setText(tr("Lunar") + instance.mLunarMonthName + instance.mLunarDayName);
    m_lunarDetailLabel->setText(instance.mGanZhiYear + tr("y") + "【"
                                + instance.mZodiac + tr("y") + "】"
                                + instance.mGanZhiMonth + tr("m") + " "
                                + instance.mGanZhiDay + tr("d"));
    m_dateLabel->setText(formatedMonth(static_cast<Month>(date.month())));

    if (m_displayedMonth != date) {
        m_displayedMonth = date;
    }

    //获取当月第一天
    QDate firstDateInMonth = QDate(date.year(), date.month(), 1);
    int day = firstDateInMonth.dayOfWeek();

    //计算当前月日历第一天该显示的时间
    QDate dateInMonth = firstDateInMonth.addDays(-(day - m_firstday + DAYS_INWEEK) % DAYS_INWEEK);
    for (SidebarCalendarKeyButton* btn : m_keyButtonList) {
        btn->setDate(dateInMonth);

        btn->setIsToday(dateInMonth == QDate::currentDate());
        btn->setIsThisMonth(dateInMonth.month() == m_displayedMonth.month());
        btn->setIsSelected(dateInMonth == m_selectedData);

        dateInMonth = dateInMonth.addDays(1);
    }
    update();
}

void SidebarCalendarWidget::updateBackground()
{
    auto pa = palette();
    auto backgroundColor = pa.window().color();
    backgroundColor.setAlphaF(0.02);
    pa.setColor(QPalette::Window, backgroundColor);
    m_leftWidget->setPalette(pa);
    m_leftWidget->setAutoFillBackground(true);
}

QString SidebarCalendarWidget::formatedDateType(CalendarManager::DateFormat format)
{
    switch (format) {
        case CalendarManager::DateFormat_1: return "yyyy/M/d";
        case CalendarManager::DateFormat_2: return "yyyy-M-d";
        case CalendarManager::DateFormat_3: return "yyyy.M.d";
        case CalendarManager::DateFormat_4: return "yyyy/MM/dd";
        case CalendarManager::DateFormat_5: return "yyyy-MM-dd";
        case CalendarManager::DateFormat_6: return "yyyy.MM.dd";
        case CalendarManager::DateFormat_7: return "MM.dd.yyyy";
        case CalendarManager::DateFormat_8: return "dd.MM.yyyy";
        case CalendarManager::DateFormat_9: return "yy/M/d";
        case CalendarManager::DateFormat_10: return "yy-M-d";
        case CalendarManager::DateFormat_11: return "yy.M.d";
        default: return "yyyy-MM-dd";
    }
}

QString SidebarCalendarWidget::formatedWeekType(CalendarManager::WeekDayFormat format)
{
    switch (format) {
        case CalendarManager::WeekDayFormat_1: return "dddd";
        default: return "ddd";
    }
}

/**
 * @brief SidebarCalendarWidget::onKeyButtonClicked
 * 日期按键点击事件
 * @param date 事件发送者的日期
 */
void SidebarCalendarWidget::onKeyButtonClicked(const QDate &date)
{
    const QDate &selectedDate = date;
    if (!withinTimeFrame(selectedDate)) {
        return;
    }
    setSelectedDate(selectedDate);
    if (selectedDate.year() == m_displayedMonth.year() && selectedDate.month() == m_displayedMonth.month()) {
        //未切换月份，只刷新界面显示
        update();
    } else {
        //已切换月份，重新设置选中的日期
        setSelectedDate(selectedDate);
    }
}

/**
 * @brief SidebarCalendarWidget::onNextPageClicked
 * 下一页切换事件
 */
void SidebarCalendarWidget::onNextPageClicked()
{
    //设置显示月份日期
    const QDate &date = m_displayedMonth.addMonths(+1);
    //设置显示的日期范围
    m_selectedData = date;
    setDate(date);
}

/**
 * @brief SidebarCalendarWidget::onPreviousPageClicked
 * 上一页切换事件
 */
void SidebarCalendarWidget::onPreviousPageClicked()
{
    //设置显示月份日期
    QDate date = m_displayedMonth.addMonths(-1);
    //设置显示的日期范围
    m_selectedData = date;
    setDate(date);
}

void SidebarCalendarWidget::onFirstDayChanged(int value)
{
    m_firstday = value;
    setDate(m_selectedData);
}

void SidebarCalendarWidget::onDateFormatChanged(CalendarManager::DateFormat format)
{
    m_leftdateLabel->setText(QDate::currentDate().toString(formatedDateType(format)));
}

void SidebarCalendarWidget::onWeekDayFormatChanged(CalendarManager::WeekDayFormat format)
{
    if (m_weekdayFormat == format)
        return;

    m_weekdayFormat = format;
    m_weekLabel->setText(formatedWeekDay(static_cast<WeekDay>(QDate::currentDate().dayOfWeek()), format));
}

void SidebarCalendarWidget::backToday()
{
    const QDate &date = QDate::currentDate();
    setSelectedDate(date);
}

void SidebarCalendarWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void SidebarCalendarWidget::showEvent(QShowEvent *event)
{
    bool showLunar = QLocale::system().language() == QLocale::Language::Chinese;
    m_lunarLabel->setVisible(showLunar);
    m_lunarDetailLabel->setVisible(showLunar);
    backToday();
    QWidget::showEvent(event);
}

void SidebarCalendarWidget::wheelEvent(QWheelEvent *event)
{
    m_deltaSum += event->delta();
    if (m_deltaSum >= TURNPAGE_DELTA) {
        Q_EMIT m_previousPage->clicked();
        m_deltaSum = 0;
    } else if (m_deltaSum <= -TURNPAGE_DELTA) {
        Q_EMIT m_nextPage->clicked();
        m_deltaSum = 0;
    }
    QWidget::wheelEvent(event);
}

QString SidebarCalendarWidget::formatedWeekDay(WeekDay weekDay, CalendarManager::WeekDayFormat format)
{
    if (QLocale::system().language() == QLocale::English)
        format = CalendarManager::WeekDayFormat_1;

    if (format == CalendarManager::WeekDayFormat_1)
        switch (weekDay) {
            case Monday: return tr("Monday");
            case Tuesday: return tr("Tuesday");
            case Wednesday: return tr("Wednesday");
            case Thursday: return tr("Thursday");
            case Friday: return tr("Friday");
            case Saturday: return tr("Saturday");
            case Sunday: return tr("Sunday");
            default: return QString();
        }
    else
        switch (weekDay) {
            case Monday: return tr("monday");
            case Tuesday: return tr("tuesday");
            case Wednesday: return tr("wednesday");
            case Thursday: return tr("thursday");
            case Friday: return tr("friday");
            case Saturday: return tr("saturday");
            case Sunday: return tr("sunday");
            default: return QString();
        }
}

QString SidebarCalendarWidget::formatedMonth(Month month)
{
    switch (month) {
        case Jan: return tr("Jan");
        case Feb: return tr("Feb");
        case Mar: return tr("Mar");
        case Apr: return tr("Apr");
        case May: return tr("May");
        case Jun: return tr("Jun");
        case Jul: return tr("Jul");
        case Aug: return tr("Aug");
        case Sept: return tr("Sept");
        case Oct: return tr("Oct");
        case Nov: return tr("Nov");
        case Dec: return tr("Dec");
        default:return QString();
    }
}

void SidebarCalendarWidget::onShortDateFormatChanged()
{
    m_leftdateLabel->setText(QDate::currentDate().toString(m_regionFormat->originShortDateFormat()));
}

SidebarCalendarKeyButton::SidebarCalendarKeyButton(QWidget* parent)
    : QPushButton(parent)
    , m_pressed(false)
    , m_date(QDate())
    , m_isToday(false)
    , m_isThisMonth(false)
    , m_isSelected(false)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(35);
}

/**
 * @brief SidebarCalendarKeyButton::onClick
 * 点击事件
 */
void SidebarCalendarKeyButton::onClick()
{
    emit clicked(date());
}

void SidebarCalendarKeyButton::mousePressEvent(QMouseEvent* event)
{
    //判断鼠标左键按下事件
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
    }
    QWidget::mousePressEvent(event);
}

void SidebarCalendarKeyButton::mouseReleaseEvent(QMouseEvent* event)
{
    //判断鼠标左键释放事件
    if (m_pressed && event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        onClick();
        m_pressed = false;
    } else if (event->button() == Qt::LeftButton) {
        m_pressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void SidebarCalendarKeyButton::mouseMoveEvent(QMouseEvent* event)
{
    //判断鼠标左键移动事件
    if (event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        m_pressed = true;
    } else if (event->button() == Qt::LeftButton) {
        m_pressed = false;
    }
}

void SidebarCalendarKeyButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal w = width();
    qreal h = height();
    const qreal r = w > h ? h : w;
    QRectF rectf(qRound((w - r) / 2), qRound((h - r) / 2), r, r); //绘制区域

    QFont font;
    font.setPixelSize(DDECalendar::FontSizeFourteen);
    painter.setFont(font);

    QStyleOption opt;
    opt.initFrom(this);
    auto pa = DApplicationHelper::instance()->palette(this);

    if (opt.state & QStyle::StateFlag::State_MouseOver) {
        painter.setPen(Qt::NoPen);
        auto color = pa.windowText().color();
        color.setAlphaF(0.1);
        painter.setBrush(color);
        painter.drawRoundedRect(rectf.marginsRemoved(
                                    QMargins(RECT_MARGINS,
                                             RECT_MARGINS,
                                             RECT_MARGINS,
                                             RECT_MARGINS)
                                    ), ROUND_CORNER, ROUND_CORNER);
        painter.setPen(pa.windowText().color());
    }

    const QString &text = QString::number(m_date.day());

    if (m_isToday) {
       painter.setPen(Qt::NoPen);
       painter.setBrush(pa.highlight());
       //绘制高亮背景
       painter.drawRoundedRect(rectf.marginsRemoved(
                                   QMargins(RECT_MARGINS,
                                            RECT_MARGINS,
                                            RECT_MARGINS,
                                            RECT_MARGINS)
                                   ), ROUND_CORNER, ROUND_CORNER);
       //设置高亮下的字体颜色
       painter.setPen(pa.highlightedText().color());
    } else if (m_isSelected) {
        QPen pen;
        pen.setColor(pa.highlight().color());
        pen.setWidth(RECT_PENWIDTH);
        painter.setPen(pen);
        //绘制高亮背景
        painter.drawRoundedRect(rectf.adjusted(RECT_PENWIDTH,
                                               RECT_PENWIDTH,
                                               -RECT_PENWIDTH,
                                               -RECT_PENWIDTH)
                                , ROUND_CORNER, ROUND_CORNER);
        //绘制选中框内圈白色高光
        pen.setColor(Qt::white);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawRoundedRect(rectf.adjusted(RECT_PENWIDTH + RECT_MARGINS + 1,
                                               RECT_PENWIDTH + RECT_MARGINS + 1,
                                               -RECT_PENWIDTH - RECT_MARGINS - 1,
                                               -RECT_PENWIDTH - RECT_MARGINS - 1)
                                , ROUND_CORNER - RECT_PENWIDTH, ROUND_CORNER - RECT_PENWIDTH);

        pen.setColor(pa.windowText().color());
        painter.setPen(pen);
    } else if (!m_isThisMonth) {
        //设置正常显示状态下的字体颜色
        painter.setOpacity(0.3);
    } else {
        painter.setPen(pa.windowText().color());
    }
    painter.drawText(rectf, text, QTextOption(Qt::AlignCenter));

    QWidget::paintEvent(event);
}

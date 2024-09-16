// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthbrefwidget.h"
#include "constants.h"
#include "scheduledatamanage.h"
#include "calendarglobalenv.h"

#include <DApplicationHelper>
#include <QMouseEvent>
#include <QPainter>
#include <QGridLayout>
#include <QPainterPath>
#include <QDebug>

QDate MonthBrefWidget::GlobalData::m_selectedMonth = QDate();
QDate MonthBrefWidget::GlobalData::m_selectedDate = QDate();
MonthBrefWidget::MonthBrefWidget(QWidget *parent)
    : QWidget(parent)
    , m_globalData(new GlobalData)
{
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);

    for (int i = 0; i < DDEYearCalendar::RectSizeOfEveryMonth; ++i) {
        CMonthDayRectWidget *item = new CMonthDayRectWidget(m_globalData, this);
        m_DayItem.append(item);
        gridLayout->addWidget(item, i/7, i%7, 1, 1, Qt::AlignCenter);

        connect(item, &CMonthDayRectWidget::signalClicked, this, &MonthBrefWidget::signalPressDate);
        connect(item, &CMonthDayRectWidget::signalDoubleClick, this, &MonthBrefWidget::signalDoubleClickDate);
    }

    this->setLayout(gridLayout);
}

MonthBrefWidget::~MonthBrefWidget()
{
    delete m_globalData;
}

/**
 * @brief MonthBrefWidget::setDate
 * 设置显示的月
 * @param monthDate 待显示的月日期
 */
void MonthBrefWidget::setShowMonthDate(const QDate& monthDate)
{
    //获取当月第一天
    QDate date = QDate(monthDate.year(), monthDate.month(), 1);
    int firstday = CalendarManager::getInstance()->getFirstDayOfWeek();
    int day = date.dayOfWeek();
    //计算当前月日历第一天该显示的时间
    date = date.addDays(-(day-firstday+7)%7);
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(date);
        date = date.addDays(1);
    }
    //设置显示的月
    m_globalData->m_showMonthDate = monthDate;
    update();
}

/**
 * @brief MonthBrefWidget::setHasScheduleDateSet
 * 设置含有日程的日期集合
 * @param hasScheduleSet
 */
void MonthBrefWidget::setHasScheduleDateSet(const QSet<QDate> &hasScheduleSet)
{
    //清空原有标识
    for (int i = 0; i < 32; ++i) {
        m_globalData->m_scheduleDateFlag[i]  = false;
    }
    //根据日程所在天设置标识
    for (QDate date:hasScheduleSet) {
        if (m_globalData->isBelongMonth(date)) {
            m_globalData->m_scheduleDateFlag[date.day()] = true;
        }
    }
    update();
}

/**
 * @brief MonthBrefWidget::setHasSearchScheduleSet
 * 设置含有搜索日程的日期集合
 * @param hasScheduleSet
 */
void MonthBrefWidget::setHasSearchScheduleSet(const QSet<QDate> &hasScheduleSet)
{
    //清空原有标识
    for (int i = 0; i < 32; ++i) {
        m_globalData->m_searchedDateFlag[i]  = false;
    }
    //根据日程所在天设置标识
    for (QDate date:hasScheduleSet) {
        if (m_globalData->isBelongMonth(date)) {
            m_globalData->m_searchedDateFlag[date.day()] = true;
        }
    }
    update();
}

/**
 * @brief MonthBrefWidget::mousePressEvent
 * 鼠标按下事件
 * @param event
 */
void MonthBrefWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸转换则设置触摸状态和触摸开始坐标
        m_touchState = 1;
        m_touchBeginPoint = event->pos();
        QWidget::mousePressEvent(event);
    }
}

/**
 * @brief MonthBrefWidget::mouseReleaseEvent        鼠标释放事件
 * @param event
 */
void MonthBrefWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        m_touchState = 0;
        QWidget::mouseReleaseEvent(event);
    }
    mouseMoveEvent(event);
}

/**
 * @brief MonthBrefWidget::mouseMoveEvent       鼠标移动事件，设置hover状态
 * @param event
 */
void MonthBrefWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        QPoint currentPoint = event->pos();
        //如果移动距离大与5则为触摸移动状态
        if (QLineF(m_touchBeginPoint, currentPoint).length() > 5) {
            m_touchState = 2;
        }
        QWidget::mouseMoveEvent(event);
    }
}

CMonthDayRectWidget::CMonthDayRectWidget(MonthBrefWidget::GlobalData* globalData, QWidget *parent) : QPushButton(parent)
  , m_globaldata(globalData)
{
    setMinimumSize(10, 10);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::NoFocus);
    setTheMe(DGuiApplicationHelper::instance()->themeType());
    //系统主题切换信号
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
                     this,&CMonthDayRectWidget::setTheMe);
}

/**
 * @brief CMonthDayRectWidget::setTheMe
 * 系统主题变化
 * @param type
 */
void CMonthDayRectWidget::setTheMe(int type)
{
    CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    if (type == 0 || type == 1) {
        m_ceventColor = QColor(255, 93, 0);
    } else if (type == 2) {
        m_ceventColor = QColor(204, 77, 3);
    }
    update();
}

/**
 * @brief CMonthDayRect::setDate
 * 设置当天的时间
 * @param date
 */
void CMonthDayRectWidget::setDate(const QDate &date)
{
    m_date = date;
}

/**
 * @brief CMonthDayRect::getDate
 * 获取当天的时间
 * @return
 */
QDate CMonthDayRectWidget::getDate() const
{
    return  m_date;
}

/**
 * @brief CMonthDayRectWidget::mousePressEvent
 * 鼠标按下事件
 * @param event
 */
void CMonthDayRectWidget::mousePressEvent(QMouseEvent *event)
{
    //因双击会产生两次按下事件，第二次按下事件触发在双击事件触发后，双击后要跳转页面，因此在按下时判断本控件是否出现显示状态再进行处理
    if (event->button() == Qt::LeftButton && isVisible()) {
        m_pressed = true;
        //保存日程悬浮框显示位置
        CalendarGlobalEnv::getGlobalEnv()->reviseValue(DDECalendar::CursorPointKey, mapToGlobal(QPoint(width()/2,height()/2)));
        m_globaldata->setSelectedDate(m_date);
        emit signalClicked(m_date);
    }
    QWidget::mousePressEvent(event);
}

/**
 * @brief CMonthDayRectWidget::mouseDoubleClickEvent
 * 鼠标双击事件
 * @param event
 */
void CMonthDayRectWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit signalDoubleClick(m_date);
    }
    QWidget::mouseDoubleClickEvent(event);
}

/**
 * @brief CMonthDayRectWidget::mouseReleaseEvent
 * 鼠标释放事件
 * @param event
 */
void CMonthDayRectWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressed = false;
    }
    update();
    QWidget::mouseReleaseEvent(event);
}

/**
 * @brief CMonthDayRectWidget::paintEvent
 * 绘制日期以及当天状态
 * @param event
 */
void CMonthDayRectWidget::paintEvent(QPaintEvent *event)
{
    //获取待绘制的文字
    QString text = QString::number(m_date.day());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //字体跟随界面大小
    qreal w = this->width();
    qreal h = this->height();
    const qreal r = w > h ? h: w;
    QRectF rectf(qRound((w-r)/2), qRound((h-r)/2), r, r);
    //根据高度和宽度设置时间字体的大小
    QFont font;
    font.setPixelSize(int(r/20.0*12));
    painter.setFont(font);

    QColor highColor = DApplicationHelper::instance()->palette(this).highlight().color();

    if (m_date == QDate::currentDate() && m_globaldata->isBelongMonth(m_date)) {
        //当天日期
        painter.setBrush(highColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rectf);
        painter.setPen(DApplicationHelper::instance()->palette(this).highlightedText().color());
        painter.drawText(rectf, Qt::AlignCenter, text);
    } else if (m_globaldata->isSelectedDate(m_date)) {
        //被选中的日期
        painter.setBrush(highColor);
        painter.setPen(Qt::NoPen);
        painter.setOpacity(0.3);
        painter.drawEllipse(rectf);
        painter.setPen(highColor);
        painter.setOpacity(1);
        painter.drawText(rectf, Qt::AlignCenter, text);
    } else if (m_globaldata->isHasSearchedByDate(m_date)) {
        //有被搜索日程，绘制文字
        painter.setPen(highColor);
        painter.setOpacity(1);
        painter.drawText(rectf, Qt::AlignCenter, text);
    } else {
        //绘制默认文字
        if (!m_globaldata->isBelongMonth(m_date)) {
            //置灰日期
            painter.setOpacity(0.3);
        }
        painter.drawText(rectf, Qt::AlignCenter, text);
    }

    if (m_globaldata->isHasSearchedByDate(m_date)) {
        //有被搜索日程，绘制圆圈
        QPainterPath path;
        path.addEllipse(rectf);
        path.addEllipse(rectf.x()+1, rectf.y()+1, rectf.width()-2, rectf.height()-2);
        painter.setBrush(highColor);
        painter.setPen(Qt::NoPen);
        painter.drawPath(path);
    }

    if (m_globaldata->isHasScheduleByDate(m_date)) {
        //有日程，绘制日程圆点
        painter.setBrush(QBrush(m_ceventColor));
        painter.setPen(Qt::NoPen);
        painter.setOpacity(1);
        qreal ellipse_r = r * (4.0 / 25);
        if (ellipse_r < 4) {
            ellipse_r = 4;
        } else if (ellipse_r > 7) {
            ellipse_r = 7;
        }
        painter.drawEllipse(QRectF(rectf.x() + rectf.width() - ellipse_r, rectf.y() + rect().y(), ellipse_r, ellipse_r));
    }

    if (m_pressed) {
        //按下状态，绘制点击效果
        painter.setBrush(DApplicationHelper::instance()->palette(this).text());
        painter.setPen(Qt::NoPen);
        painter.setOpacity(0.3);
        painter.drawEllipse(rectf);
    } else if (m_hovered) {
        //悬浮状态，绘制悬浮效果
        painter.setBrush(DApplicationHelper::instance()->palette(this).text());
        painter.setOpacity(0.2);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rectf);
    }
    QWidget::paintEvent(event);
}

/**
 * @brief CMonthDayRectWidget::enterEvent
 * 鼠标进入事件
 * @param event
 */
void CMonthDayRectWidget::enterEvent(QEvent *event)
{
    m_hovered = true;
    QWidget::enterEvent(event);
}

/**
 * 离开事件，设置当前选中的日期为空
 *
 */
void CMonthDayRectWidget::leaveEvent(QEvent *event)
{
    m_hovered = false;
    QWidget::leaveEvent(event);
}

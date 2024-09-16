// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "weekheadview.h"
#include "customframe.h"
#include "scheduledatamanage.h"

#include <DPalette>
#include <DHiDPIHelper>

#include <QHBoxLayout>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include <QMenu>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPushButton>

DGUI_USE_NAMESPACE
CWeekHeadView::CWeekHeadView(QWidget *parent)
    : DWidget(parent)
    , m_touchGesture(this)
{
    setContentsMargins(0, 0, 0, 0);

    m_dayNumFont.setWeight(QFont::Medium);
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_monthFont.setWeight(QFont::Medium);
    m_monthFont.setPixelSize(DDECalendar::FontSizeTwenty);

    // cells grid
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setMargin(0);
    hBoxLayout->setSpacing(0);

    m_monthLabel = new CustomFrame(this);
    m_monthLabel->setFixedWidth(DDEWeekCalendar::WMCellHeaderWidth - 5);
    m_monthLabel->setRoundState(true, false, false, false);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);
    mlabelF.setPixelSize(DDECalendar::FontSizeTwenty);
    m_monthLabel->setTextFont(mlabelF);
    hBoxLayout->addWidget(m_monthLabel);
    hBoxLayout->setStretch(0, 0);
    hBoxLayout->setSpacing(0);

    for (int c = 0; c != DDEWeekCalendar::AFewDaysofWeek; ++c) {
        QWidget *cell = new QWidget(this);
        cell->installEventFilter(this);
        cell->setFocusPolicy(Qt::ClickFocus);
        hBoxLayout->addWidget(cell);
        m_cellList.append(cell);
    }

    setLayout(hBoxLayout);
}

CWeekHeadView::~CWeekHeadView()
{

}

/**
 * @brief setTheMe      根据系统主题类型设置颜色
 * @param type          系统主题类型
 */
void CWeekHeadView::setTheMe(int type)
{
    m_themetype = type;

    if (type == 0 || type == 1) {
        QColor textC = "#000000";
        QColor textBC(230, 238, 242);

        m_monthLabel->setBColor(textBC);
        m_monthLabel->setTextColor(textC);

        m_backgroundCircleColor = "#0081FF";
        m_backgroundShowColor = "#2CA7F8";
        m_backgroundShowColor.setAlphaF(0.4);
        m_Background_Weekend_Color = "#DAE4ED";

        m_defaultTextColor = "#6F6F6F";
        m_currentDayTextColor = "#FFFFFF";
        m_defaultLunarColor = "#898989";
        m_currentMonthColor = "#000000";
        m_backgroundColor = "#E6EEF2";
        m_solofestivalLunarColor = "#4DFF7272";
    } else if (type == 2) {
        QColor textBC = "#82AEC1";
        textBC.setAlphaF(0.1);
        m_monthLabel->setBColor(textBC);
        m_monthLabel->setTextColor("#BF1D63");
        m_backgroundCircleColor = "#0059D2";
        m_backgroundShowColor = "#002AAF";
        m_backgroundShowColor.setAlphaF(0.4);
        m_Background_Weekend_Color = "#333D4A";

        m_defaultTextColor = "#C0C6D4";
        m_currentDayTextColor = "#C0C6D4";
        m_defaultLunarColor = "#6886BA";

        m_currentMonthColor = "#000000";
        m_backgroundColor = "#82AEC1";
        m_backgroundColor.setAlphaF(0.1);
        m_solofestivalLunarColor = "#4DFF7272";
    }
    m_weekendsTextColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    update();
}

/**
 * @brief CWeekHeadView::setWeekDay         设置一周显示时间
 * @param vDays
 * @param selectDate
 */
void CWeekHeadView::setWeekDay(QVector<QDate> vDays, const QDate &selectDate)
{
    if (vDays.count() != DDEWeekCalendar::AFewDaysofWeek)
        return;
    m_days = vDays;
    QLocale locale;
    m_monthLabel->setTextStr(locale.monthName(selectDate.month(), QLocale::ShortFormat));
}

/**
 * @brief CWeekHeadView::setHunagliInfo     设置黄历信息
 * @param huangLiInfo
 */
void CWeekHeadView::setHunagLiInfo(const QMap<QDate, CaHuangLiDayInfo> &huangLiInfo)
{
    m_huangLiInfo = huangLiInfo;
    update();
}

/**
 * @brief setLunarVisible   设置是否显示阴历信息
 * @param visible           是否显示阴历信息
 */
void CWeekHeadView::setLunarVisible(bool visible)
{
    int state = int(m_showState);

    if (visible)
        state |= ShowLunar;
    else
        state &= ~ShowLunar;

    m_showState = ShowState(state);
    update();
}

/**
 * @brief eventFilter 过滤器
 * @param o 事件对象
 * @param e 时间类型
 * @return false
 */
bool CWeekHeadView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);

    if (cell && m_cellList.contains(cell)) {
        if (e->type() == QEvent::Paint) {
            paintCell(cell);
        }  else if (e->type() == QEvent::MouseButtonDblClick) {
            const int pos = m_cellList.indexOf(cell);
            emit signalsViewSelectDate(m_days[pos]);
        }
    }
    return false;
}

/**
 * @brief getCellDayNum 根据索引值获取当天是在一个月中的第几天
 * @param pos 索引
 * @return 日期的字符串
 */
const QString CWeekHeadView::getCellDayNum(int pos)
{
    return QString::number(m_days[pos].day());
}

/**
 * @brief getCellDate 根据索引获取当天的日期
 * @param pos 索引值
 * @return 日期
 */
const QDate CWeekHeadView::getCellDate(int pos)
{
    return m_days[pos];
}

/**
 * @brief getLunar 根据索引值获取当天的阴历信息
 * @param pos 索引值
 * @return 阴历信息字符串
 */
const QString CWeekHeadView::getLunar(int pos)
{
    CaHuangLiDayInfo info ;
    if (pos >= 0 && pos < m_days.size()) {
        info = m_huangLiInfo[m_days[pos]];
        if (info.mLunarDayName == "初一") {
            info.mLunarDayName =  info.mLunarMonthName;
        }
        if (info.mTerm.isEmpty())
            return info.mLunarDayName;
    }
    return info.mTerm;
}

/**
 * @brief paintCell 绘制周信息
 * @param cell 每天所在的widget
 */
void CWeekHeadView::paintCell(QWidget *cell)
{
    m_weekendsTextColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    const QRect rect(0, 0, cell->width(), cell->height());
    const int pos = m_cellList.indexOf(cell);
    const bool isCurrentDay = getCellDate(pos) == QDate::currentDate();
    const bool isSelectedCell = isCurrentDay;
    int d = m_days[pos].dayOfWeek();

    QPainter painter(cell);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    //根据周几设置不一样的背景色
    if (d == DDEWeekCalendar::FirstDayOfWeekend || d == DDEWeekCalendar::AFewDaysofWeek) {
        painter.setBrush(QBrush(m_Background_Weekend_Color));
    } else {
        painter.setBrush(QBrush(m_backgroundColor));
    }
    if (m_cellList.last() != cell) {
        painter.drawRect(rect); //画矩形
        //绘制分割线
        QPoint point_begin(cell->width(), 0);
        QPoint point_end(cell->width(), cell->height());
        painter.save();
        QColor m_linecolor = "#000000";
        m_linecolor.setAlphaF(0.1);
        painter.setPen(m_linecolor);
        painter.drawLine(point_begin, point_end);
        painter.restore();
    } else {
        int labelwidth = cell->width();
        int labelheight = cell->height();
        QPainterPath painterPath;
        painterPath.moveTo(0, 0);
        painterPath.lineTo(0, labelheight);
        painterPath.lineTo(labelwidth, labelheight);
        painterPath.lineTo(labelwidth, labelheight - m_radius);
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
        painterPath.lineTo(0, 0);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
    }
    int bw = (cell->width() - 104) / 2;
    int bh = (cell->height() - 26) / 2;

    if (bw < 0) {
        bw = 2;
    }
    if (bh < 0) {
        bh = 2;
    }
    if (isSelectedCell) {
        if (m_showState & ShowLunar) {
            QRect fillRect(bw - 2, bh, 26, 26);
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(fillRect);
            painter.restore();
        } else {
            QRect fillRect(cell->width() - (cell->width() / 2) + 1, bh - 1, 26, 26);
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(fillRect);
            painter.restore();
        }
    }
    QLocale locale;
    const QString dayNum = getCellDayNum(pos);
    const QString dayLunar = getLunar(pos);
    QString dayWeek = locale.dayName(d ? d : DDEWeekCalendar::AFewDaysofWeek, QLocale::ShortFormat);

    painter.save();
    painter.setPen(Qt::SolidLine);
    // draw text of day
    if (isSelectedCell) {
        painter.setPen(m_currentDayTextColor);
    } else if (isCurrentDay) {
        painter.setPen(m_weekendsTextColor);
    } else {
        if (d == DDEWeekCalendar::FirstDayOfWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
            painter.setPen(m_weekendsTextColor);
        else
            painter.setPen(m_defaultTextColor);
    }

    painter.setFont(m_dayNumFont);

    if (m_showState & ShowLunar) {
        painter.drawText(QRect(bw - 1, bh, 24, 24), Qt::AlignCenter, dayNum);
        if (d == DDEWeekCalendar::FirstDayOfWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
            painter.setPen(m_weekendsTextColor);
        else
            painter.setPen(m_defaultTextColor);
        painter.drawText(QRect(bw + 24, bh, 30, 25), Qt::AlignCenter, dayWeek);
    } else {
        QFontMetrics fm1 = painter.fontMetrics();
        painter.drawText(QRect(cell->width() - (cell->width() / 2) - 4, bh - 1, 36, 26), Qt::AlignCenter, dayNum);
        if (d == DDEWeekCalendar::FirstDayOfWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
            painter.setPen(m_weekendsTextColor);
        else
            painter.setPen(m_defaultTextColor);

        QFontMetrics fm = painter.fontMetrics();

        while (fm.width(dayWeek) > cell->width() / 2)
            dayWeek.chop(1);
        //水平右对齐，上下居中
        painter.drawText(QRect(0, bh, (cell->width() / 2), 26), Qt::AlignRight | Qt::AlignVCenter, dayWeek);
    }
    // draw text of day type
    if (m_showState & ShowLunar) {
        if (cell->width() > 100) {
            if (d == DDEWeekCalendar::FirstDayOfWeekend || d == DDEWeekCalendar::AFewDaysofWeek)
                painter.setPen(m_weekendsTextColor);
            else
                painter.setPen(m_defaultLunarColor);

            if (cell->width() < 132) {
                QString str_dayLunar = nullptr;
                if (dayLunar.count() > 2) {
                    for (int i = 0; i < 2; i++) {
                        str_dayLunar.append(dayLunar.at(i));
                    }
                    str_dayLunar.append("...");
                } else {
                    str_dayLunar = dayLunar;
                }
                //水平左对齐，上下居中
                painter.drawText(QRect(bw + 52 + 10, bh, 50, 25), Qt::AlignLeft | Qt::AlignVCenter, str_dayLunar);
            } else {
                painter.drawText(QRect(bw + 52 + 10, bh, 50, 25), Qt::AlignLeft | Qt::AlignVCenter, dayLunar);
            }
        }
    }
    painter.restore();
    painter.end();
}

void CWeekHeadView::wheelEvent(QWheelEvent *e)
{
    //如果滚轮为左右方向则触发信号
    if (e->orientation() == Qt::Orientation::Horizontal) {
        emit signalAngleDelta(e->angleDelta().x());
    }
}

bool CWeekHeadView::event(QEvent *e)
{
    if (m_touchGesture.event(e)) {
        //获取触摸状态
        switch (m_touchGesture.getTouchState()) {
        case touchGestureOperation::T_SLIDE: {
            //在滑动状态如果可以更新数据则切换月份
            if (m_touchGesture.isUpdate()) {
                m_touchGesture.setUpdate(false);
                switch (m_touchGesture.getMovingDir()) {
                case touchGestureOperation::T_LEFT:
                    emit signalAngleDelta(-1);
                    break;
                case touchGestureOperation::T_RIGHT:
                    emit signalAngleDelta(1);
                    break;
                default:
                    break;
                }
            }
            break;
        }
        default:
            break;
        }
        return true;
    } else {
        return DWidget::event(e);
    }
}

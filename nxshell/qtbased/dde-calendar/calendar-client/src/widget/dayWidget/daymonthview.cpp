// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "todaybutton.h"
#include "scheduledatamanage.h"
#include "daymonthview.h"
#include "constants.h"
#include "dayhuangliview.h"
#include "configsettings.h"
#include "calendarglobalenv.h"
#include "scheduledlg.h"

#include <DPalette>
#include <DHorizontalLine>
#include <DHiDPIHelper>

#include <QGridLayout>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>


DGUI_USE_NAMESPACE
CDayMonthView::CDayMonthView(QWidget *parent)
    : CustomFrame(parent)
{
    m_weeklist.append(tr("Monday"));
    m_weeklist.append(tr("Tuesday"));
    m_weeklist.append(tr("Wednesday"));
    m_weeklist.append(tr("Thursday"));
    m_weeklist.append(tr("Friday"));
    m_weeklist.append(tr("Saturday"));
    m_weeklist.append(tr("Sunday"));
    initUI();
    initConnection();
}

CDayMonthView::~CDayMonthView()
{
}

void CDayMonthView::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
{
    m_selectDate = selectDate;
    m_currentDate = currentDate;
    m_dayMonthWidget->setShowDate(showDate, selectDate, currentDate);
    updateDateShow();
    update();
}

void CDayMonthView::setLunarVisible(bool visible)
{
    m_huanglistate = visible;
    m_yiLabel->setVisible(visible);
    m_jiLabel->setVisible(visible);
    m_currentLuna->setVisible(visible);
    m_splitline->setVisible(visible);
    update();
}

void CDayMonthView::setTheMe(int type)
{
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    m_dayMonthWidget->setTheMe(type);
    if (type == 0 || type == 1) {
        DPalette aniPa = this->palette();
        QColor tbColor = "#FFFFFF";
        aniPa.setColor(DPalette::Background, tbColor);
        setPalette(aniPa);
        setBackgroundRole(DPalette::Background);
        setBColor(tbColor);

        QColor todayhover = "#000000";
        todayhover.setAlphaF(0.1);
        QColor todaypress = "#000000";
        todaypress.setAlphaF(0.2);
        DPalette prevPalette = m_prevButton->palette();
        prevPalette.setColor(DPalette::Dark, QColor("#E6E6E6"));
        prevPalette.setColor(DPalette::Light, QColor("#E3E3E3"));

        DPalette nextPa = m_nextButton->palette();
        nextPa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        nextPa.setColor(DPalette::Light, QColor("#E3E3E3"));

        m_currentMouth->setTextColor(QColor("#3B3B3B"));

        m_currentDay->setTextColor(todayColor);

        m_currentWeek->setTextColor(QColor("#414D68"));

        m_currentLuna->setTextColor(QColor("#414D68"));

        m_currentYear->setTextColor(QColor("#414D68"));

        QFont hLabelF;
        hLabelF.setPixelSize(DDECalendar::FontSizeFourteen);

        QColor yiColor = QColor("#75C18E");
        yiColor.setAlphaF(0.1);
        m_yiLabel->setbackgroundColor(yiColor);
        m_yiLabel->setTextInfo(QColor("#7B7B7B"), hLabelF);

        QColor jiColor = QColor("#C17575");
        jiColor.setAlphaF(0.1);
        m_jiLabel->setbackgroundColor(jiColor);
        m_jiLabel->setTextInfo(QColor("#7B7B7B"), hLabelF);

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0081FF";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;

    } else if (type == 2) {
        DPalette aniPa = this->palette();
        QColor tbColor = "#282828";
        aniPa.setColor(DPalette::Background, tbColor);
        setPalette(aniPa);
        setBackgroundRole(DPalette::Background);
        setBColor(tbColor);

        DPalette prevPalette = m_prevButton->palette();
        prevPalette.setColor(DPalette::Dark, QColor("#484848"));
        prevPalette.setColor(DPalette::Light, QColor("#414141"));
        DPalette nextPa = m_nextButton->palette();
        nextPa.setColor(DPalette::Dark, QColor("#484848"));
        nextPa.setColor(DPalette::Light, QColor("#414141"));

        m_currentMouth->setTextColor(QColor("#C0C6D4"));

        m_currentDay->setTextColor(todayColor);

        m_currentWeek->setTextColor(QColor("#C0C6D4"));

        m_currentLuna->setTextColor(QColor("#C0C6D4"));

        m_currentYear->setTextColor(QColor("#C0C6D4"));
        QFont hLabelF;
        hLabelF.setPixelSize(DDECalendar::FontSizeFourteen);

        QColor yiColor = QColor("#2F8C4D");
        yiColor.setAlphaF(0.2);
        m_yiLabel->setbackgroundColor(yiColor);
        m_yiLabel->setTextInfo(QColor("#C0C6D4"), hLabelF);

        QColor jiColor = QColor("#A43B3B");
        jiColor.setAlphaF(0.2);
        m_jiLabel->setbackgroundColor(jiColor);
        m_jiLabel->setTextInfo(QColor("#C0C6D4"), hLabelF);

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0059D2";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;
    }
    update();
}

void CDayMonthView::setSearchFlag(bool flag)
{
    m_searchflag = flag;
    update();
}

/**
 * @brief CDayMonthView::setHuangLiInfo     设置黄历信息
 * @param huangLiInfo
 */
void CDayMonthView::setHuangLiInfo(const CaHuangLiDayInfo &huangLiInfo)
{
    m_huangLiInfo = huangLiInfo;
    updateDateLunarDay();
}

void CDayMonthView::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
{
    m_dayMonthWidget->setHasScheduleFlag(hasScheduleFlag);
}

void CDayMonthView::initUI()
{
    m_today = new CTodayButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(80, DDEDayCalendar::D_MLabelHeight);
    QFont todayfont;
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    m_prevButton->setFixedSize(36, 36);

    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    m_nextButton->setFixedSize(36, 36);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 3);
    //add separator line
    m_currentMouth = new CustomFrame(this);
    m_currentMouth->setFixedSize(74, DDEDayCalendar::D_MLabelHeight);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);
    mlabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_currentMouth->setTextFont(mlabelF);
    m_currentMouth->setTextAlign(Qt::AlignCenter);
    titleLayout->addWidget(m_prevButton);
    titleLayout->addWidget(m_currentMouth);
    titleLayout->addWidget(m_nextButton);
    titleLayout->addStretch();
    titleLayout->addWidget(m_today, 0, Qt::AlignRight);

    //上半部分
    m_upLayout = new QVBoxLayout;
    m_upLayout->setMargin(0);
    m_upLayout->setSpacing(0);
    m_upLayout->setContentsMargins(22, 9, 0, 7);
    m_upLayout->addLayout(titleLayout);

    m_weekWidget = new CWeekWidget();
    m_weekWidget->setMaximumHeight(40);
    m_dayMonthWidget = new CDayMonthWidget();
    m_upLayout->addWidget(m_weekWidget, 1);
    m_upLayout->addWidget(m_dayMonthWidget, 6);

    //中间部分
    QVBoxLayout *midLayout = new QVBoxLayout;
    midLayout->setMargin(0);
    midLayout->setSpacing(0);
    midLayout->setContentsMargins(0, 0, 0, 20);
    m_currentDay = new CustomFrame(this);
    m_currentDay->setFixedHeight(DDEDayCalendar::DDLabelHeight);
    m_currentDay->setMinimumWidth(width());
    m_currentDay->setTextAlign(Qt::AlignCenter);
    QFont dayLabelF;
    dayLabelF.setWeight(QFont::Medium);
    dayLabelF.setPixelSize(DDECalendar::FontSizeOneHundred);
    m_currentDay->setTextFont(dayLabelF);
    midLayout->addWidget(m_currentDay);

    m_currentWeek = new CustomFrame(this);
    m_currentWeek->setFixedHeight(DDEDayCalendar::DWLabelHeight);
    m_currentWeek->setTextAlign(Qt::AlignCenter);
    QFont wLabelF;
    wLabelF.setPixelSize(DDECalendar::FontSizeSixteen);
    m_currentWeek->setTextFont(wLabelF);
    midLayout->addWidget(m_currentWeek);
    midLayout->addSpacing(2);

    m_currentYear = new CustomFrame(this);
    m_currentYear->setFixedHeight(DDEDayCalendar::DWLabelHeight);
    m_currentYear->setTextAlign(Qt::AlignCenter);
    m_currentYear->setTextFont(wLabelF);
    midLayout->addWidget(m_currentYear);
    midLayout->addSpacing(2);

    m_currentLuna = new CustomFrame(this);
    m_currentLuna->setFixedHeight(DDEDayCalendar::DHuangLiInfoLabelHeight);
    m_currentLuna->setTextAlign(Qt::AlignCenter);
    QFont hLabelF;
    hLabelF.setPixelSize(DDECalendar::FontSizeTwelve);
    m_currentLuna->setTextFont(hLabelF);
    midLayout->addWidget(m_currentLuna);

    m_yiDownLayout = new QVBoxLayout;
    m_yiDownLayout->setSpacing(0);
    m_yiDownLayout->setContentsMargins(10, 5, 10, 0);
    hLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_yiLabel = new CDayHuangLiLabel(this);
    m_yiLabel->setbackgroundColor(QColor("#75C18E"));
    m_yiLabel->setTextInfo(QColor("#7B7B7B "), hLabelF);
    m_yiLabel->setMinimumHeight(DDEDayCalendar::DHuangLiLabelHeight);
    m_yiLabel->setMaximumHeight(DDEDayCalendar::DHuangLiLabelMaxHeight);
    m_yiLabel->setHuangLiText(QStringList());
    m_yiDownLayout->addWidget(m_yiLabel, 1);

    m_jiDownLayout = new QVBoxLayout;
    m_jiDownLayout->setSpacing(0);
    m_jiDownLayout->setContentsMargins(10, 10, 10, 10);

    m_jiLabel = new CDayHuangLiLabel(this);
    m_jiLabel->setbackgroundColor(QColor("#C17575"));
    m_jiLabel->setTextInfo(QColor("#7B7B7B "), hLabelF);
    m_jiLabel->setMinimumHeight(DDEDayCalendar::DHuangLiLabelHeight);
    m_jiLabel->setMaximumHeight(DDEDayCalendar::DHuangLiLabelMaxHeight);
    m_jiLabel->setHuangLiText(QStringList(), 1);
    m_jiDownLayout->addWidget(m_jiLabel, 1);

    m_hhLayout = new QVBoxLayout;
    m_hhLayout->setMargin(0);
    m_hhLayout->setSpacing(0);
    m_hhLayout->addLayout(m_upLayout, 6);
    m_hhLayout->addLayout(midLayout);

    m_splitline = new DHorizontalLine;
    m_splitline->setMinimumHeight(2);

    QHBoxLayout *hlineLayout = new QHBoxLayout;
    hlineLayout->setSpacing(0);
    hlineLayout->setContentsMargins(50, 0, 50, 5);
    hlineLayout->addWidget(m_splitline);

    m_hhLayout->addLayout(hlineLayout);
    m_hhLayout->addLayout(m_yiDownLayout);
    m_hhLayout->addLayout(m_jiDownLayout);
    m_hhLayout->addStretch();

    setLayout(m_hhLayout);
}

void CDayMonthView::initConnection()
{
    connect(m_prevButton, &DIconButton::clicked, this, &CDayMonthView::slotprev);
    connect(m_today, &CTodayButton::clicked, this, &CDayMonthView::slottoday);
    connect(m_nextButton, &DIconButton::clicked, this, &CDayMonthView::slotnext);
    connect(m_dayMonthWidget, &CDayMonthWidget::signalChangeSelectDate, this, &CDayMonthView::signalChangeSelectDate);
}

/**
 * @brief CDayMonthView::updateDateShow     更新月/天界面显示
 */
void CDayMonthView::updateDateShow()
{
    QLocale locale;
    m_currentMouth->setTextStr(locale.monthName(m_selectDate.month(), QLocale::ShortFormat));
    m_currentDay->setTextStr(QString::number(m_selectDate.day()));

    if (m_selectDate.dayOfWeek() > 0)
        m_currentWeek->setTextStr(m_weeklist.at(m_selectDate.dayOfWeek() - 1));
    m_currentYear->setTextStr(m_selectDate.toString("yyyy/M"));

    if (m_selectDate == m_currentDate) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
}

/**
 * @brief CDayMonthView::updateDateLunarDay     更新显示黄历信息
 */
void CDayMonthView::updateDateLunarDay()
{
    if (!m_huanglistate) return;
    m_currentLuna->setTextStr(m_huangLiInfo.mGanZhiYear + "年 " + "【" + m_huangLiInfo.mZodiac + "年】" + m_huangLiInfo.mGanZhiMonth + "月 " + m_huangLiInfo.mGanZhiDay + "日 ");
    QStringList yiList = m_huangLiInfo.mSuit.split(".", QString::SkipEmptyParts);
    QStringList jiList = m_huangLiInfo.mAvoid.split(".", QString::SkipEmptyParts);
    m_yiLabel->setHuangLiText(yiList);
    m_jiLabel->setHuangLiText(jiList, 1);
}

void CDayMonthView::changeSelectDate(const QDate &date)
{
    emit signalChangeSelectDate(date);
}

void CDayMonthView::wheelEvent(QWheelEvent *event)
{
    //如果是拖拽则退出
    bool isDragging = false;
    emit signalIsDragging(isDragging);
    if (isDragging)
        return;
    if (event->delta() < 0) {
        //切换前一天
        changeSelectDate(m_selectDate.addDays(1));
    } else {
        //切换后一天
        changeSelectDate(m_selectDate.addDays(-1));
    }
}

void CDayMonthView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();
    DPalette aniPa = this->palette();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.save();
    painter.setBrush(aniPa.background());
    painter.setPen(Qt::NoPen);
    QPainterPath painterPath;
    painterPath.moveTo(0, 0);
    painterPath.lineTo(0, labelheight);
    //  如果有搜索界面则为右边为直角否则为圆角
    if (!m_searchflag) {
        painterPath.lineTo(labelwidth - m_radius, labelheight);
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
        painterPath.lineTo(labelwidth, m_radius);
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);
    } else {
        painterPath.lineTo(labelwidth, labelheight);
        painterPath.lineTo(labelwidth, 0);
    }
    painterPath.lineTo(0, 0);
    painterPath.closeSubpath();
    painter.drawPath(painterPath);
    painter.restore();
}

void CDayMonthView::slotprev()
{
    changeSelectDate(m_selectDate.addMonths(-1));
}

void CDayMonthView::slotnext()
{
    changeSelectDate(m_selectDate.addMonths(1));
}

void CDayMonthView::slottoday()
{
    changeSelectDate(m_currentDate);
}

CDayMonthWidget::CDayMonthWidget(QWidget *parent)
    : QWidget(parent)
    , m_isFocus(false)
{
    m_gridLayout = new QGridLayout;
    m_gridLayout->setMargin(0);
    m_gridLayout->setSpacing(0);
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwelve);
    for (int r = 0; r < 6; ++r) {
        for (int c = 0; c < 7; ++c) {
            QWidget *cell = new QWidget;
            cell->installEventFilter(this);
            m_gridLayout->addWidget(cell, r, c, 1, 1);
            m_cellList.append(cell);
        }
    }
    this->setLayout(m_gridLayout);
    setFocusPolicy(Qt::StrongFocus);
    //设置最大高度
    setMaximumHeight(250);
}

CDayMonthWidget::~CDayMonthWidget()
{
}

void CDayMonthWidget::setTheMe(int type)
{
    m_currentDayTextColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    if (type == 0 || type == 1) {
        m_defaultTextColor = Qt::black;
        m_selectedTextColor = Qt::white;
        m_notCurrentTextColor = "#b2b2b2";
        m_ceventColor = QColor(255, 93, 0);
    } else if (type == 2) {
        m_defaultTextColor = "#C0C6D4";
        m_selectedTextColor = "#B8D3FF";
        m_notCurrentTextColor = "#C0C6D4";
        m_notCurrentTextColor.setAlphaF(0.5);
        m_ceventColor = QColor(204, 77, 3);
    }
    update();
}

void CDayMonthWidget::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
{
    m_showDays = showDate;
    m_selectDate = selectDate;
    m_currentDate = currentDate;
    //当前选择index
    m_selectedCell = m_showDays.indexOf(m_selectDate);
    update();
}

void CDayMonthWidget::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
{
    m_vlineflag = hasScheduleFlag;
    update();
}

const QString CDayMonthWidget::getCellDayNum(int pos)
{
    return QString::number(m_showDays[pos].day());
}

const QDate CDayMonthWidget::getCellDate(int pos)
{
    return m_showDays[pos];
}

void CDayMonthWidget::paintCell(QWidget *cell)
{
    const QRect rect = cell->rect();
    const int pos = m_cellList.indexOf(cell);
    const bool isSelectedCell = pos == m_selectedCell;
    const bool isCurrentDay = getCellDate(pos) == m_currentDate;

    QPainter painter(cell);
    painter.setRenderHint(QPainter::Antialiasing);

    // draw selected cell background circle
    if (isSelectedCell) {
        const qreal r = rect.width() > rect.height() ? rect.height() * 0.9 : rect.width() * 0.9;
        const qreal x = rect.x() + (rect.width() - r) / 2;
        const qreal y = rect.y() + (rect.height() - r) / 2;
        QRectF fillRect = QRectF(x, y, r, r).marginsRemoved(QMarginsF(1.5, 2.5, 1.5, 1.5));
        painter.save();
        painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(fillRect);
        painter.restore();
        if (m_isFocus) {
            //绘制焦点获取效果
            QPen pen;
            pen.setWidth(2);
            pen.setColor(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor());
            painter.setPen(pen);
            //在原有的选中效果外面再绘制一圈
            QRectF focusRect(fillRect.x() - 2, fillRect.y() - 2, fillRect.width() + 4, fillRect.height() + 4);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(focusRect);
        }
    }

    painter.setPen(Qt::SolidLine);

    const QString dayNum = getCellDayNum(pos);

    // draw text of day
    if (isSelectedCell) {
        painter.setPen(m_selectedTextColor);
    } else if (isCurrentDay) {
        painter.setPen(m_currentDayTextColor);
    } else {
        if (m_selectDate.month() == getCellDate(pos).month())
            painter.setPen(m_defaultTextColor);
        else
            painter.setPen(m_notCurrentTextColor);
    }

    QRect test;
    painter.setFont(m_dayNumFont);

    painter.drawText(rect, Qt::AlignCenter, dayNum, &test);

    if (m_vlineflag.count() == DDEDayCalendar::PainterCellNum) {
        if (m_vlineflag[pos]) {
            if (m_selectDate.month() == getCellDate(pos).month()) {
                painter.save();
                QPen pen;
                pen.setWidth(2);
                pen.setColor(m_ceventColor);
                painter.setPen(pen);
                painter.setBrush(QBrush(m_ceventColor));
                painter.setPen(Qt::NoPen);
                int r = int(cell->width() * 0.1);
                if (r < 4) {
                    r = 4;
                } else if (r > 7) {
                    r = 7;
                }
                painter.drawEllipse(cell->width() - r - 6, 4, r, r);
                painter.restore();
            }
        }
    }
}

bool CDayMonthWidget::eventFilter(QObject *o, QEvent *e)
{
    if (m_showDays.size() != 42)
        return false;
    QWidget *cell = qobject_cast<QWidget *>(o);
    if (cell && m_cellList.contains(cell)) {
        const int pos = m_cellList.indexOf(cell);
        QDate date = m_showDays[pos];
        //如果需要显示时间小于1900年则退出
        if (date.year() < DDECalendar::QueryEarliestYear)
            return false;
        if (e->type() == QEvent::Paint) {
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            m_dayMouseState = 1;
        } else if (e->type() == QEvent::MouseMove) {
            if (m_dayMouseState == 1) {
                m_dayMouseState = 2;
            }

            if (2 == m_dayMouseState) {
                QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);

                //如果m_dayCreateState为0需要判断是否离开了选择区域，否则不需要判断
                if (m_dayCreateState == 0 && !cell->rect().contains(mouseEvent->pos())) {
                    //如果离开选择日期区域则修改鼠标显示
                    setCursor(Qt::ClosedHandCursor);
                    m_dayCreateState = 1;
                }

                if (m_dayCreateState != 0) {
                    //根据是否在日历界面内设置鼠标形状和创建状态
                    QPoint globalPoint =  QCursor::pos();
                    QWidget *widget = QApplication::activeWindow();
                    //
                    if (widget != nullptr) {
                        QRect rect = widget->geometry();

                        if (rect.contains(globalPoint)) {
                            setCursor(Qt::ClosedHandCursor);
                            m_dayCreateState = 1;
                        } else {
                            setCursor(Qt::ForbiddenCursor);
                            m_dayCreateState = 2;
                        }
                    }
                }
            }
        } else if (e->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
            //
            if (m_dayCreateState == 1) {
                //新建日程
                const int pos = m_cellList.indexOf(cell);
                QDate date = m_showDays.at(pos);
                //设置日程开始时间
                QDateTime _beginTime(date, QTime::currentTime());
                //新建日程对话框
                CScheduleDlg _scheduleDig(1, this, false);
                //设置开始时间
                _scheduleDig.setDate(_beginTime);
                _scheduleDig.exec();

            } else {
                //如果不在点击区域内则不跳转日期
                //跳转日期
                if (mouseEvent->button() == Qt::LeftButton && cell->rect().contains(mouseEvent->pos())) {
                    cellClicked(cell);
                }
            }

            setCursor(Qt::ArrowCursor);
            m_dayMouseState = 0;
            m_dayCreateState = 0;
        }
    }
    return false;
}

void CDayMonthWidget::resizeEvent(QResizeEvent *event)
{
    //获取每个时间widget的高度和宽度
    qreal width = this->width() / 7;
    qreal height = this->height() / 6;
    const qreal r = width > height ? height * 0.9 : width * 0.9;
    //根据高度和宽度设置时间字体的大小
    m_dayNumFont.setPixelSize(qRound(12 + (r - 18) * 6 / 17.0));
    QWidget::resizeEvent(event);
}

void CDayMonthWidget::focusInEvent(QFocusEvent *event)
{
    switch (event->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
    case Qt::ActiveWindowFocusReason:
        m_isFocus = true;
        break;
    default:
        m_isFocus = false;
        break;
    };
    update();
}

void CDayMonthWidget::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    m_isFocus = false;
    update();
}

void CDayMonthWidget::keyPressEvent(QKeyEvent *event)
{
    if (m_isFocus) {
        switch (event->key()) {
        case Qt::Key_Left:
            emit signalChangeSelectDate(m_selectDate.addDays(-1));
            break;
        case Qt::Key_Right:
            emit signalChangeSelectDate(m_selectDate.addDays(1));
            break;
        case Qt::Key_Up:
            emit signalChangeSelectDate(m_selectDate.addDays(-7));
            break;
        case Qt::Key_Down:
            emit signalChangeSelectDate(m_selectDate.addDays(7));
            break;
        default:

            break;
        }
    }
    QWidget::keyPressEvent(event);
}

void CDayMonthWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    m_isFocus = false;
    if (event->button() & Qt::LeftButton)
        m_startPos = event->pos();
}

void CDayMonthWidget::cellClicked(QWidget *cell)
{
    const int pos = m_cellList.indexOf(cell);
    if (pos == -1)
        return;
    setSelectedCell(pos);
}

void CDayMonthWidget::setSelectedCell(int index)
{
    if (m_selectedCell == index)
        return;
    emit signalChangeSelectDate(m_showDays.at(index));
}

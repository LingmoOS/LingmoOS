// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleitem.h"
#include "schedulecoormanage.h"
#include "scheduledatamanage.h"
#include "calendarmanage.h"

#include <DFontSizeManager>

#include <QDebug>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMarginsF>

#include <dtkwidget_global.h>

DWIDGET_USE_NAMESPACE

CScheduleItem::CScheduleItem(QRectF rect, QGraphicsItem *parent, int type)
    : DragInfoItem(rect, parent)
    , m_type(type)
    , m_totalNum(0)
    , m_transparentcolor("#000000")
    , m_timeFormat(CalendarManager::getInstance()->getTimeFormat())
{
    m_transparentcolor.setAlphaF(0.05);
    connect(CalendarManager::getInstance(), &CalendarManager::signalTimeFormatChanged, this, &CScheduleItem::timeFormatChanged);
}

CScheduleItem::~CScheduleItem()
{
}

/**
 * @brief CScheduleItem::setData        设置显示数据
 * @param info
 * @param date
 * @param totalNum
 */
void CScheduleItem::setData(const DSchedule::Ptr &info, QDate date, int totalNum)
{
    m_vScheduleInfo = info;
    m_totalNum = totalNum;
    setDate(date);
    update();
}

/**
 * @brief CScheduleItem::hasSelectSchedule      是否含有选中日程
 * @param info
 * @return
 */
bool CScheduleItem::hasSelectSchedule(const DSchedule::Ptr &info)
{
    return info == m_vScheduleInfo;
}

/**
 * @brief CScheduleItem::splitText      根据字体大小,宽度和高度将标题切换为多行
 * @param font
 * @param w
 * @param h
 * @param str
 * @param listStr
 * @param fontM
 */
void CScheduleItem::splitText(QFont font, int w, int h, QString str, QStringList &listStr, QFontMetrics &fontM)
{
    if (str.isEmpty())
        return;
    QFontMetrics fontMetrics(font);
    int heightT = fontM.height();
    QString tStr;
    QStringList tListStr;

    for (int i = 0; i < str.count(); i++) {
        tStr.append(str.at(i));
        int widthT = fontMetrics.horizontalAdvance(tStr) + 5;

        if (widthT >= w) {
            tStr.chop(1);
            if (tStr.isEmpty())
                break;
            tListStr.append(tStr);
            tStr.clear();
            i--;
        }
    }
    tListStr.append(tStr);

    if (w < 30) {
        QFontMetrics fm_s(fontM);
        QFontMetrics f_st(font);
        QString s = tListStr.at(0) + "...";

        if (h < 23) {
            tListStr.append("");
        } else {
            if (tListStr.isEmpty()) {
                listStr.append("");
            } else {
                QString c = str.at(0);
                QString str = c + "...";
                QFontMetrics fm(font);
                while (f_st.horizontalAdvance(str) > w && f_st.horizontalAdvance(str) > 24) {
                    str.chop(1);
                }
                listStr.append(str);
            }
        }
    } else {
        for (int i = 0; i < tListStr.count(); i++) {
            if ((i + 1) * heightT <= h - 1) {
                listStr.append(tListStr.at(i));
            } else {
                if (i == 0) {
                    break;
                } else {
                    QString s;
                    QFontMetrics fm_str(fontM);

                    if (i == tListStr.count())
                        s = fontM.elidedText(tListStr.at(i - 1), Qt::ElideRight, w);
                    else {
                        s = fontM.elidedText(tListStr.at(i - 1) + "...", Qt::ElideRight, w);
                    }
                    listStr.removeAt(i - 1);
                    listStr.append(s);
                    break;
                }
            }
        }
    }
}

/**
 * @brief CScheduleItem::timeFormatChanged 更新时间显示格式
 */
void CScheduleItem::timeFormatChanged(int value)
{
    if (value) {
        m_timeFormat = "hh:mm";
    } else {
        m_timeFormat = "h:mm";
    }
    update();
}

/**
 * @brief CScheduleItem::paintBackground        绘制item显示效果
 * @param painter
 * @param rect
 * @param isPixMap
 */
void CScheduleItem::paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap)
{
    Q_UNUSED(isPixMap);
    //根据日程类型获取颜色
    CSchedulesColor gdColor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo->scheduleTypeID());

    QColor textPenColor = CScheduleDataManage::getScheduleDataManage()->getTextColor();
    //判断是否为选中日程
    if (m_vScheduleInfo == m_pressInfo) {
        //判断当前日程是否为拖拽移动日程
        if (m_vScheduleInfo->isMoved() == m_pressInfo->isMoved()) {
            m_vHighflag = true;
        } else {
            painter->setOpacity(0.4);
            textPenColor.setAlphaF(0.4);
            gdColor.orginalColor.setAlphaF(0.4);
            m_vHighflag = false;
        }
        m_vSelectflag = m_press;
    }

    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    QColor bColor = gdColor.normalColor;
    QFontMetrics fm = painter->fontMetrics();
    int h = fm.height();

    if (m_vHoverflag) {
        bColor = gdColor.hoverColor;
    } else if (m_vHighflag) {
        bColor = gdColor.hightColor;
    } else if (m_vSelectflag) {
        bColor = gdColor.pressColor;
    }
    painter->setBrush(bColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    if (m_vHoverflag && !m_vSelectflag) {
        painter->save();
        QRectF tRect = QRectF(rect.x() + 0.5, rect.y() + 0.5, rect.width() - 1, rect.height() - 1);
        QPen tPen;
        QColor cc = "#FFFFFF";
        if (themetype == 2) {
            cc = "#FFFFFF";
        } else {
            cc = "#000000";
        }
        cc.setAlphaF(0.08);
        tPen.setColor(cc);
        tPen.setWidthF(1);
        tPen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(tPen);
        painter->drawRect(tRect);
        painter->restore();
    }
    if (m_vSelectflag) {
        if (themetype == 0 || themetype == 1) {
            textPenColor.setAlphaF(0.4);
            gdColor.orginalColor.setAlphaF(0.4);
        } else if (themetype == 2) {
            textPenColor.setAlphaF(0.6);
            gdColor.orginalColor.setAlphaF(0.6);
        }
    }

    painter->save();
    QPen pen(gdColor.orginalColor);
    pen.setWidth(2);
    painter->setPen(pen);
    //左侧绘制竖线
    QPointF top(rect.topLeft().x(), rect.topLeft().y() + 1);
    QPointF bottom(rect.bottomLeft().x(), rect.bottomLeft().y() - 1);
    painter->drawLine(top, bottom);
    painter->restore();
    int tMargin = 10;

    if (m_totalNum > 1)
        tMargin = 5;

    if (m_type == 0) {
        int timeTextHight = 0;
        QFont font;
        font.setWeight(QFont::Normal);

        font = DFontSizeManager::instance()->get(DFontSizeManager::T8, font);

        //绘制日程起始时间
        if (m_vScheduleInfo->dtStart().date() == getDate()) {
            painter->save();
            painter->setFont(font);
            painter->setPen(gdColor.orginalColor);

            QTime stime = m_vScheduleInfo->dtStart().time();
            QString str = stime.toString((CalendarManager::getInstance()->getTimeShowType() ? "AP " : "") + m_timeFormat);
            QFontMetrics fontMetrics(font);
            qreal drawTextWidth = rect.width() - m_offset * 2;

            if (fm.horizontalAdvance(str) > drawTextWidth - 5) {
                QString tStr;
                for (int i = 0; i < str.count(); i++) {
                    tStr.append(str.at(i));
                    int widthT = fm.horizontalAdvance(tStr) - 5;

                    if (widthT >= drawTextWidth) {
                        if (i < 1) {
                            tStr.chop(1);
                        } else {
                            tStr.chop(2);
                        }
                        tStr = tStr + "...";
                        break;
                    }
                }
                QString tStrs = fontMetrics.elidedText(str, Qt::ElideRight, qRound(drawTextWidth - 5));
                painter->drawText(
                    QRectF(rect.topLeft().x() + tMargin, rect.topLeft().y() + 3, drawTextWidth - 5, h),
                    Qt::AlignLeft, tStrs);

            } else {
                painter->drawText(
                    QRectF(rect.topLeft().x() + tMargin, rect.topLeft().y() + 3, drawTextWidth - 5, h),
                    Qt::AlignLeft, str);
            }
            painter->restore();
        } else {
            timeTextHight = -20;
        }
        painter->save();

        //绘制日程标题
        font = DFontSizeManager::instance()->get(DFontSizeManager::T6, font);
        font.setLetterSpacing(QFont::PercentageSpacing, 105);
        painter->setFont(font);
        painter->setPen(textPenColor);
        QStringList liststr;
        QRect textRect = rect.toRect();
        textRect.setWidth(textRect.width() - m_offset * 2);
        splitText(font,
                  textRect.width() - tMargin - 8,
                  textRect.height() - 20,
                  m_vScheduleInfo->summary(),
                  liststr, fm);

        for (int i = 0; i < liststr.count(); i++) {
            if ((20 + timeTextHight + (i + 1) * (h - 3)) > rect.height())
                return;
            painter->drawText(
                QRect(textRect.topLeft().x() + tMargin,
                      textRect.topLeft().y() + 20 + timeTextHight + i * (h - 3),
                      textRect.width() - 2,
                      h),
                Qt::AlignLeft, liststr.at(i));
        }
        painter->restore();
    } else {
        painter->save();
        QFont font;
        font.setWeight(QFont::Normal);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T8, font);
        painter->setFont(font);
        painter->setPen(textPenColor);
        painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, "...");
        painter->restore();
    }
    if (m_vSelectflag) {
        QColor selcolor = m_transparentcolor;
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);
    }
    if (getItemFocus()) {
        //获取tab图形
        QRectF drawRect = rect.marginsRemoved(QMarginsF(1, 1, 1, 1));
        painter->setBrush(Qt::NoBrush);
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
        painter->drawRect(drawRect);
    }
}

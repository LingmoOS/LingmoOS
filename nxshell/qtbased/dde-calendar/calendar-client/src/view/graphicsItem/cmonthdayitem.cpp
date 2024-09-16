// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cmonthdayitem.h"
#include "constants.h"
#include "scheduledatamanage.h"

#include <DHiDPIHelper>

#include <QPainter>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

bool CMonthDayItem::m_LunarVisible = false;
CMonthDayItem::CMonthDayItem(QGraphicsItem *parent)
    : CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView, parent)
    , m_DayLunar("")
    , m_DayStatus(H_NONE)
{
    //设置字体
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_dayNumFont.setWeight(QFont::Light);

    m_LunerFont.setPixelSize(DDECalendar::FontSizeTwelve);
    m_LunerFont.setWeight(QFont::Normal);
}

CMonthDayItem::~CMonthDayItem()
{
}

/**
 * @brief CMonthDayItem::setLunar   设置农历信息
 * @param lunar
 */
void CMonthDayItem::setLunar(const QString &lunar)
{
    m_DayLunar = lunar;
}

/**
 * @brief CMonthDayItem::setStatus  设置班休状态
 * @param status
 */
void CMonthDayItem::setStatus(const CMonthDayItem::HolidayStatus &status)
{
    m_DayStatus = status;
}

/**
 * @brief CMonthDayItem::setTheMe   设置主题
 * @param type
 */
void CMonthDayItem::setTheMe(int type)
{
    m_themetype = type;

    if (type == 0 || type == 1) {
        m_dayNumColor = "#000000";
        m_dayNumCurrentColor = "#FFFFFF";

        m_LunerColor = "#5E5E5E";
        m_LunerColor.setAlphaF(0.5);

        m_fillColor = Qt::white;
        m_banColor = "#FF7171";
        m_banColor.setAlphaF(0.1);
        m_xiuColor = "#ADFF71";
        m_xiuColor.setAlphaF(0.1);

        m_BorderColor = "#000000";
        m_BorderColor.setAlphaF(0.05);
    } else if (type == 2) {
        m_dayNumColor = "#C0C6D4";
        m_dayNumCurrentColor = "#B8D3FF";

        m_LunerColor = "#ABDAFF";
        m_LunerColor.setAlphaF(0.5);

        m_fillColor = "#000000";
        m_fillColor.setAlphaF(0.05);
        m_banColor = "#FF7171";
        m_banColor.setAlphaF(0.1);
        m_xiuColor = "#ADFF71";
        m_xiuColor.setAlphaF(0.1);

        m_BorderColor = "#000000";
        m_BorderColor.setAlphaF(0.05);
    }
    update();
}

void CMonthDayItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    const int hh = 36;
    m_currentColor = getSystemActiveColor();
    painter->setRenderHints(QPainter::Antialiasing);
    //绘制背景
    if (m_LunarVisible) {
        switch (m_DayStatus) {
        case H_WORK:
            painter->setBrush(QBrush(m_banColor));
            break;
        case H_REST:
            painter->setBrush(QBrush(m_xiuColor));
            break;
        default:
            painter->setBrush(QBrush(m_fillColor));
            break;
        }
    } else {
        painter->setBrush(QBrush(m_fillColor));
    }

    if (!m_IsCurrentMonth) {
        painter->setOpacity(0.4);
    }
    QPen pen;
    pen.setWidth(1);
    pen.setColor(m_BorderColor);
    painter->setPen(pen);
    painter->drawRect(this->rect());
    painter->save();
    //绘制日期
    painter->setFont(m_dayNumFont);
    QRectF fillRect;
    if (m_LunarVisible) {
        fillRect.setRect(this->rect().x() + 3, this->rect().y() + 4, hh, hh);
    } else {
        fillRect.setRect(this->rect().x(), this->rect().y() + 4, this->rect().width(), hh);
    }
    //如果为当前时间
    if (m_Date == QDate::currentDate()) {
        //设置不透明度为1
        painter->setOpacity(1);
        QFont tFont = m_dayNumFont;
        tFont.setPixelSize(DDECalendar::FontSizeTwenty);
        painter->setFont(tFont);
        painter->setPen(m_dayNumCurrentColor);
        painter->save();
        painter->setBrush(QBrush(m_currentColor));
        painter->setPen(Qt::NoPen);
        if (m_LunarVisible)
            painter->drawEllipse(QRectF(this->rect().x() + 6, this->rect().y() + 4, hh - 8, hh - 8));
        else
            painter->drawEllipse(QRectF((this->rect().width() - hh + 8) / 2 + this->rect().x(), this->rect().y() + 4, hh - 8, hh - 8));
        painter->restore();
    } else {
        painter->setPen(m_dayNumColor);
    }

    fillRect.setY(fillRect.y() - 10);
    fillRect.setX(fillRect.x() - 1);
    painter->drawText(fillRect,
                      Qt::AlignCenter,
                      QString::number(m_Date.day()));
    painter->restore();
    //绘制农历
    if (m_LunarVisible) {
        QFontMetrics metrics(m_LunerFont);
        int Lunarwidth = metrics.width(m_DayLunar);
        qreal filleRectX = this->rect().width() - 12 - 3 - (58 + Lunarwidth) / 2;
        QRectF fillRectT(this->rect().x() + filleRectX,
                         this->rect().y() + 9,
                         12,
                         12);

        if (filleRectX > hh) {
            painter->setRenderHint(QPainter::Antialiasing);
            switch (m_DayStatus) {
            case H_WORK: {
                QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/icons/dde_calendar_ban_32px.svg");
                painter->drawPixmap(fillRectT.toRect(), pixmap);
            } break;
            case H_REST: {
                QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/icons/deepin/builtin/icons/dde_calendar_xiu.svg");
                painter->drawPixmap(fillRectT.toRect(), pixmap);
            } break;
            default:
                break;
            }
        }
        painter->setFont(m_LunerFont);
        painter->setPen(m_LunerColor);
        painter->drawText(QRectF(this->rect().x() + this->rect().width() - 58,
                                 this->rect().y() + 6, 58, 18),
                          Qt::AlignCenter, m_DayLunar);
    }
    //如果有焦点则绘制焦点效果
    if (getItemFocus()) {
        const int offset = 1;
        //获取tab图形
        QRectF drawRect(rect().x() + offset, rect().y() + offset, rect().width() - offset * 2, rect().height() - offset * 2);
        QPen framePen;
        //设置边框宽度
        framePen.setWidth(2);
        //设置边框颜色
        framePen.setColor(m_currentColor);
        painter->setPen(framePen);
        painter->setBrush(Qt::NoBrush);
        painter->setOpacity(1);
        //半径
        qreal radius = 16;
        //直径
        qreal diameter = radius * 2;
        //绘制的路径
        QPainterPath path;
        path.moveTo(drawRect.x(), drawRect.y());
        //如果为左下角则绘制圆角
        if (this->getBackgroundNum() == 35) {
            path.lineTo(drawRect.x(), drawRect.y() + drawRect.height() - radius);
            QRectF arcRect(drawRect.x(), drawRect.y() + drawRect.height() - diameter, diameter, diameter);
            path.arcTo(arcRect, 180, 90);
        } else {
            path.lineTo(drawRect.x(), drawRect.y() + drawRect.height());
        }
        //如果为右下角则绘制圆角
        if (this->getBackgroundNum() == 41) {
            path.lineTo(drawRect.x() + drawRect.width() - radius, drawRect.y() + drawRect.height());
            QRectF arcRect(drawRect.x() + drawRect.width() - diameter, drawRect.y() + drawRect.height() - diameter, diameter, diameter);
            path.arcTo(arcRect, 270, 90);
        } else {
            path.lineTo(drawRect.x() + drawRect.width(), drawRect.y() + drawRect.height());
        }
        path.lineTo(drawRect.x() + drawRect.width(), drawRect.y());
        path.lineTo(drawRect.x(), drawRect.y());
        painter->drawPath(path);
    }
}

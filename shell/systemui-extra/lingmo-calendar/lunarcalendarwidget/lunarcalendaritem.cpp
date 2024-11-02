/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#pragma execution_character_set("utf-8")

#include "lunarcalendaritem.h"
#include "lunarcalendarwidget.h"
#include "lunarcalendarinfo.h"
#include "calendarcolor.h"

#include "qpainter.h"
#include "qevent.h"
#include "qdatetime.h"
#include "qdebug.h"
#include <QGSettings>

//日历字号
const int day_font_size = 16;
const int lunar_font_size = 12;
const int other_font_size = 12;

LunarCalendarItem::LunarCalendarItem(QString font,bool isHaveSchedule,QWidget *parent) : QWidget(parent),m_font(font),hasSchedule(isHaveSchedule)
{
    setProperty("useStyleWindowManager",false);
    setAttribute(Qt::WA_TransparentForMouseEvents,true);
    hover = false;
    pressed = false;

    select = false;
    showLunar = true;
    bgImage = ":/image/bg_calendar.png";
    selectType = SelectType_Rect;

    date = QDate::currentDate();
    lunar = "初一";
    dayType = DayType_MonthCurrent;

    //实时监听主题变化
    const QByteArray id("org.lingmo.style");
    QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
    connect(fontSetting, &QGSettings::changed,[=](QString key) {
        if((key.compare("style-name") == 0) || (key.compare("theme-color") == 0)|| (key.compare("widget-theme-name")==0) ||
            (key.compare("styleName") == 0) || (key.compare("themeColor") == 0)|| (key.compare("widgetThemeName")==0) ){
            weekColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            otherTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherLunarColor.setAlphaF(0.35);
            currentLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor.setAlphaF(0.45);
            lunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            lunarColor.setAlphaF(0.35);
            selectBgColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
            //hoverBgColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
            qreal mix = 0.2;
            hoverBgColor = CalendarColor::CalendarColor::mixColor(CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED),mix);
            this->update();
        }
    });

    weekColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
    currentTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
    otherTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
    otherLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
    otherLunarColor.setAlphaF(0.45);
    currentLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
    lunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
    lunarColor.setAlphaF(0.35);
    superColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::WORK);
    selectTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED_TEXT);
    hoverTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED_TEXT);
    selectLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::WORK);
    hoverLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::WORK);
    selectBgColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
    //hoverBgColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
    qreal mix = 0.2;
    hoverBgColor = CalendarColor::CalendarColor::mixColor(CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED),mix);
}

void LunarCalendarItem::onEnter()
{
        hover = true;
        this->update();
}
void LunarCalendarItem::onLeave()
{
        hover = false;
        this->update();
}
void LunarCalendarItem::onSelected()
{
    Q_EMIT clicked(date, dayType);
    update();
}

QString LunarCalendarItem::handleJsMap(QString year,QString month2day)
{
    QString oneNUmber = "worktime.y" + year;
    QString twoNumber = "d" + month2day;

    QMap<QString,QMap<QString,QString>>::Iterator it = worktime.begin();

    while(it!=worktime.end()) {
         if(it.key() == oneNUmber) {
            QMap<QString,QString>::Iterator it1 = it.value().begin();
            while(it1!=it.value().end()) {
                if(it1.key() == twoNumber) {
                    return it1.value();
                }
                it1++;
            }
         }
         it++;
    }
    return "-1";
}


void LunarCalendarItem::paintEvent(QPaintEvent *)
{
    QDate dateNow = QDate::currentDate();
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    //绘制背景和边框
    drawBg(&painter);
    //对比当前的时间，画选中状态
    if(dateNow == date) {
         drawBgCurrent(&painter, selectBgColor);
         if(hover || select) {
            drawBgHover(&painter, hoverBgColor);
         }
         //设置一个字的颜色
         if (dayType == DayType_MonthCurrent) {
            currentTextColor =CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED_TEXT);
            currentLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED_TEXT);
            currentLunarColor.setAlphaF(0.45);
         }
    } else if (hover) {
        //绘制悬停状态、绘制选中状态
        drawBgHover(&painter, hoverBgColor);
        //设置一个字的颜色
        if (dayType == DayType_MonthCurrent) {
            currentTextColor =CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor.setAlphaF(0.45);
        }else {
            otherLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherLunarColor.setAlphaF(0.15);
        }
    } else if (select){
        if(LunarCalendarWidget::getClickedDate() == date){
            drawBgHover(&painter, hoverBgColor);
        }
        //设置一个字的颜色
        if (dayType == DayType_MonthCurrent) {
            currentTextColor =CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor.setAlphaF(0.45);
        }else {
            otherLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherLunarColor.setAlphaF(0.15);
        }
    }else {
        if (dayType == DayType_MonthCurrent) {
            currentTextColor =CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::TEXT);
            currentLunarColor.setAlphaF(0.45);
        }else {
            otherLunarColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherTextColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::OTHER_TEXT);
            otherLunarColor.setAlphaF(0.15);
        }
    }
    //绘制日期
    drawDay(&painter);
    //绘制农历信息
    drawLunar(&painter);
    const QByteArray i("org.lingmo.style");
    QGSettings * fontSetting = new QGSettings(i, QByteArray(), this);
    connect(fontSetting,&QGSettings::changed,[=](QString key){
        if(key.compare("themeColor") == 0){
           selectBgColor = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
           this->update();
          }
        });

    if(hasSchedule)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(selectBgColor);
        int dotSize = 6;
        int dotX = width() /2 - dotSize/2;
        int dotY = height() - 8;
        painter.drawEllipse(dotX, dotY, 6, 4);

    }
    if(isToday)
    {
        if(hasSchedule)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::white);
            int dotSize = 6;
            int dotX = width() /2 - dotSize/2;
            int dotY = height() - 8;
            painter.drawEllipse(dotX, dotY, 6, 4);
        }

    }
}

void LunarCalendarItem::drawBg(QPainter *painter)
{
    painter->save();

    //根据当前类型选择对应的颜色
    QColor bgColor = currentBgColor;
    if (dayType == DayType_MonthPre || dayType == DayType_MonthNext) {
        bgColor = otherBgColor;
    }

    painter->restore();
}

void LunarCalendarItem::drawBgCurrent(QPainter *painter, const QColor &color)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);

    QRect rect = this->rect();
    rect = QRect(rect.x()+1,rect.y()+1,rect.width()-2,rect.height()-2);
    painter->drawRoundedRect(rect,6,6);
    painter->restore();
}

void LunarCalendarItem::drawBgHover(QPainter *painter, const QColor &color)
{
    painter->save();
    QRect rect = this->rect();
    rect = QRect(rect.x()+1,rect.y()+1,rect.width()-2,rect.height()-2);
    painter->setPen(QPen(color,3));
    painter->drawRoundedRect(rect,6,6);
    painter->restore();
}

void LunarCalendarItem::drawDay(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    int side = qMin(width, height);
    painter->save();
    //根据当前类型选择对应的颜色
    QColor color = currentTextColor;
    if (dayType == DayType_MonthPre || dayType == DayType_MonthNext) {
        color = otherTextColor;
    } else if (dayType == DayType_WeekEnd) {
        color = weekColor;
    }

    painter->setPen(color);

    QFont font = m_font;
    font.setPixelSize(day_font_size);
    //设置文字粗细
    font.setBold(true);
    painter->setFont(font);

    QLocale locale = (QLocale::system().name() == "zh_CN" ? (QLocale::Chinese) : (QLocale::English));
    //代码复用率待优化
    if (showLunar) {
        QRect dayRect;
        if(locale == QLocale::Chinese)
        {
            dayRect = QRect(0, height / 8 - 2, width, height / 2.3);
        }
        else
        {
            dayRect = QRect(0, 0, width, height);
        }
        painter->drawText(dayRect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(date.day()));
        if (handleJsMap(date.toString("yyyy"),date.toString("MMdd")) == "2") {
            painter->setPen(Qt::NoPen);
            if(locale == QLocale::Chinese){
                painter->setBrush(CalendarColor::CalendarColor::getThemeColor(CalendarColor::HOLIDAY));
            }
            QRect dayRect1 = QRect(0, 0, width/3.5,height/3.5);
            painter->drawRoundedRect(dayRect1,4,4);
            font.setPixelSize(other_font_size);
            painter->setFont(font);
            painter->setPen(Qt::white);
            if(locale == QLocale::Chinese){
                painter->drawText(dayRect1, Qt::AlignHCenter | Qt::AlignVCenter,"休");
            }
        } else if (handleJsMap(date.toString("yyyy"),date.toString("MMdd")) == "1") {
            painter->setPen(Qt::NoPen);
            if(locale == QLocale::Chinese){
                painter->setBrush(CalendarColor::CalendarColor::getThemeColor(CalendarColor::WORK));
            }
            QRect dayRect1 = QRect(0, 0, width/3.5,height/3.5);
            painter->drawRoundedRect(dayRect1,4,4);
            font.setPixelSize(other_font_size);
            painter->setFont(font);
            painter->setPen(Qt::white);
            if(locale == QLocale::Chinese){
                painter->drawText(dayRect1, Qt::AlignHCenter | Qt::AlignVCenter,"班");
            }
        }
    }
    else {
        //非农历
        QRect dayRect = QRect(0, 0, width, height);
        painter->drawText(dayRect, Qt::AlignCenter, QString::number(date.day()));
        if (handleJsMap(date.toString("yyyy"),date.toString("MMdd")) == "2") {
            painter->setPen(Qt::NoPen);
            QRect dayRect1 = QRect(0, 0, width/3.5,height/3.5);
            painter->drawRoundedRect(dayRect1,6,6);
            font.setPixelSize(other_font_size);
            painter->setFont(font);
            painter->setPen(Qt::white);
        } else if (handleJsMap(date.toString("yyyy"),date.toString("MMdd")) == "1") {
            painter->setPen(Qt::NoPen);
            QRect dayRect1 = QRect(0, 0, width/3.5,height/3.5);
            painter->drawRoundedRect(dayRect1,6,6);
            font.setPixelSize(other_font_size);
            painter->setFont(font);
            painter->setPen(Qt::white);
        }
    }

    painter->restore();
}

void LunarCalendarItem::drawLunar(QPainter *painter)
{
    int width = this->width();
    int height = this->height();
    QLocale locale = (QLocale::system().name() == "zh_CN" ? (QLocale::Chinese) : (QLocale::English));
    painter->save();
    if (showLunar){
        if(locale == QLocale::Chinese){
            QStringList listDayName;
            listDayName << "*" << "初一" << "初二" << "初三" << "初四" << "初五" << "初六" << "初七" << "初八" << "初九" << "初十"
                        << "十一" << "十二" << "十三" << "十四" << "十五" << "十六" << "十七" << "十八" << "十九" << "二十"
                        << "廿一" << "廿二" << "廿三" << "廿四" << "廿五" << "廿六" << "廿七" << "廿八" << "廿九" << "三十";

            //判断当前农历文字是否节日,是节日且是当月则用农历节日颜色显示
            bool exist = (!listDayName.contains(lunar) && dayType != DayType_MonthPre && dayType != DayType_MonthNext);
            //根据当前类型选择对应的颜色
            QColor color = currentLunarColor;
            if (dayType == DayType_MonthPre || dayType == DayType_MonthNext) {
                color = otherLunarColor;
            }
            if (exist) {
                color = currentLunarColor;
            }
            painter->setPen(color);
            QFont font = m_font;
            font.setPixelSize(lunar_font_size);
            painter->setFont(font);

            QRect lunarRect(0, 30 - 2, width, height / 3.1);
            painter->drawText(lunarRect, Qt::AlignTop|Qt::AlignHCenter, lunar);
            }
        }
    painter->restore();
}


bool LunarCalendarItem::getSelect() const
{
    return this->select;
}

bool LunarCalendarItem::getShowLunar() const
{
    return this->showLunar;
}

QString LunarCalendarItem::getBgImage() const
{
    return this->bgImage;
}

LunarCalendarItem::SelectType LunarCalendarItem::getSelectType() const
{
    return this->selectType;
}

QDate LunarCalendarItem::getDate() const
{
    return this->date;
}

QString LunarCalendarItem::getLunar() const
{
    return this->lunar;
}

LunarCalendarItem::DayType LunarCalendarItem::getDayType() const
{
    return this->dayType;
}

QColor LunarCalendarItem::getBorderColor() const
{
    return this->borderColor;
}

QColor LunarCalendarItem::getWeekColor() const
{
    return this->weekColor;
}

QColor LunarCalendarItem::getSuperColor() const
{
    return this->superColor;
}

QColor LunarCalendarItem::getLunarColor() const
{
    return this->lunarColor;
}

QColor LunarCalendarItem::getCurrentTextColor() const
{
    return this->currentTextColor;
}

QColor LunarCalendarItem::getOtherTextColor() const
{
    return this->otherTextColor;
}

QColor LunarCalendarItem::getSelectTextColor() const
{
    return this->selectTextColor;
}

QColor LunarCalendarItem::getHoverTextColor() const
{
    return this->hoverTextColor;
}

QColor LunarCalendarItem::getCurrentLunarColor() const
{
    return this->currentLunarColor;
}

QColor LunarCalendarItem::getOtherLunarColor() const
{
    return this->otherLunarColor;
}

QColor LunarCalendarItem::getSelectLunarColor() const
{
    return this->selectLunarColor;
}

QColor LunarCalendarItem::getHoverLunarColor() const
{
    return this->hoverLunarColor;
}

QColor LunarCalendarItem::getCurrentBgColor() const
{
    return this->currentBgColor;
}

QColor LunarCalendarItem::getOtherBgColor() const
{
    return this->otherBgColor;
}

QColor LunarCalendarItem::getSelectBgColor() const
{
    return this->selectBgColor;
}

QColor LunarCalendarItem::getHoverBgColor() const
{
    return this->hoverBgColor;
}

QSize LunarCalendarItem::sizeHint() const
{
    return QSize(100, 100);
}

QSize LunarCalendarItem::minimumSizeHint() const
{
    return QSize(20, 20);
}

void LunarCalendarItem::setSelect(bool select)
{
    if (this->select != select) {
        this->select = select;
        this->update();
    }
}

void LunarCalendarItem::setShowLunar(bool showLunar)
{
        this->showLunar = showLunar;
        this->update();
}

void LunarCalendarItem::setBgImage(const QString &bgImage)
{
    if (this->bgImage != bgImage) {
        this->bgImage = bgImage;
        this->update();
    }
}

void LunarCalendarItem::setSelectType(const LunarCalendarItem::SelectType &selectType)
{
    if (this->selectType != selectType) {
        this->selectType = selectType;
        this->update();
    }
}

void LunarCalendarItem::setDate(const QDate &date)
{
    if (this->date != date) {
        this->date = date;
        this->update();
    }
}

void LunarCalendarItem::setLunar(const QString &lunar)
{
    if (this->lunar != lunar) {
        this->lunar = lunar;
        this->update();
    }
}

void LunarCalendarItem::setDayType(const LunarCalendarItem::DayType &dayType)
{
    if (this->dayType != dayType) {
        this->dayType = dayType;
        this->update();
    }
}

void LunarCalendarItem::setDate(const QDate &date, const QString &lunar, const DayType &dayType)
{
    this->date = date;
    this->lunar = lunar;
    this->dayType = dayType;
    this->update();
}

void LunarCalendarItem::setBorderColor(const QColor &borderColor)
{
    if (this->borderColor != borderColor) {
        this->borderColor = borderColor;
        this->update();
    }
}

void LunarCalendarItem::setWeekColor(const QColor &weekColor)
{
    if (this->weekColor != weekColor) {
        this->weekColor = weekColor;
        this->update();
    }
}

void LunarCalendarItem::setSuperColor(const QColor &superColor)
{
    if (this->superColor != superColor) {
        this->superColor = superColor;
        this->update();
    }
}

void LunarCalendarItem::setLunarColor(const QColor &lunarColor)
{
    if (this->lunarColor != lunarColor) {
        this->lunarColor = lunarColor;
        this->update();
    }
}

void LunarCalendarItem::setCurrentTextColor(const QColor &currentTextColor)
{
    if (this->currentTextColor != currentTextColor) {
        this->currentTextColor = currentTextColor;
        this->update();
    }
}

void LunarCalendarItem::setOtherTextColor(const QColor &otherTextColor)
{
    if (this->otherTextColor != otherTextColor) {
        this->otherTextColor = otherTextColor;
        this->update();
    }
}

void LunarCalendarItem::setSelectTextColor(const QColor &selectTextColor)
{
    if (this->selectTextColor != selectTextColor) {
        this->selectTextColor = selectTextColor;
        this->update();
    }
}

void LunarCalendarItem::setHoverTextColor(const QColor &hoverTextColor)
{
    if (this->hoverTextColor != hoverTextColor) {
        this->hoverTextColor = hoverTextColor;
        this->update();
    }
}

void LunarCalendarItem::setCurrentLunarColor(const QColor &currentLunarColor)
{
    if (this->currentLunarColor != currentLunarColor) {
        this->currentLunarColor = currentLunarColor;
        this->update();
    }
}

void LunarCalendarItem::setOtherLunarColor(const QColor &otherLunarColor)
{
    if (this->otherLunarColor != otherLunarColor) {
        this->otherLunarColor = otherLunarColor;
        this->update();
    }
}

void LunarCalendarItem::setSelectLunarColor(const QColor &selectLunarColor)
{
    if (this->selectLunarColor != selectLunarColor) {
        this->selectLunarColor = selectLunarColor;
        this->update();
    }
}

void LunarCalendarItem::setHoverLunarColor(const QColor &hoverLunarColor)
{
    if (this->hoverLunarColor != hoverLunarColor) {
        this->hoverLunarColor = hoverLunarColor;
        this->update();
    }
}

void LunarCalendarItem::setCurrentBgColor(const QColor &currentBgColor)
{
    if (this->currentBgColor != currentBgColor) {
        this->currentBgColor = currentBgColor;
        this->update();
    }
}

void LunarCalendarItem::setOtherBgColor(const QColor &otherBgColor)
{
    if (this->otherBgColor != otherBgColor) {
        this->otherBgColor = otherBgColor;
        this->update();
    }
}

void LunarCalendarItem::setSelectBgColor(const QColor &selectBgColor)
{
    if (this->selectBgColor != selectBgColor) {
        this->selectBgColor = selectBgColor;
        this->update();
    }
}

void LunarCalendarItem::setHoverBgColor(const QColor &hoverBgColor)
{
    if (this->hoverBgColor != hoverBgColor) {
        this->hoverBgColor = hoverBgColor;
        this->update();
    }
}

bool LunarCalendarItem::event(QEvent *event)
{
    if(event->type()==QEvent::ToolTip){
        if(date.month()==11 && date.day()==9 ){
            setToolTip(tr("消防宣传日"));
        }
        if(date.month()==3 && date.day()==5 ){
            setToolTip(tr("志愿者服务日"));
        }
        if(date.month()==6 && date.day()==6 ){
            setToolTip(tr("全国爱眼日"));
        }
        if(date.month()==7 && date.day()==7 ){
            setToolTip(tr("抗战纪念日"));
        }
    }
    return QWidget::event(event);
}
void LunarCalendarItem::isHaveSchedule()
{
    CalendarDataBase::getInstance().searchDataSlot(QDate::currentDate());
   int num =  CalendarDataBase::getInstance().m_markInfoList.count();
   if(num != 0){
       hasSchedule = true;
       update();
//       return true;
   }
   else {
       hasSchedule = false;
//       return false;
   }
}

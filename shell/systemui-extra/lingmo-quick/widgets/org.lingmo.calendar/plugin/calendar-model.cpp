/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "calendar-model.h"
#include "lunar-calendar-info.h"
#include "calendar-item.h"

#include <QMetaEnum>
#include <QFile>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>

static CalendarModel* m_instance = nullptr;
CalendarModel *CalendarModel::instance()
{
    if (!m_instance) {
        m_instance = new CalendarModel();
    }
    return m_instance;
}

int CalendarModel::rowCount(const QModelIndex &parent) const
{
    return m_item.length();
}

QVariant CalendarModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if ((i < 0) || (i >= m_item.length())) {
        return QVariant();
    }
    CalendarItem* item = m_item[i];
    switch (static_cast<Role>(role)) {
    case Role::Day:
        return item->getItemDate().day();
    case Role::LunarDay:
        return item->getItemLunar();
    case Role::Month:
        return item->getItemDate().month();
    case Role::DateType:
        return getWorKType(item->getItemDate());
    case Role::IsCurrentDay:
        return (item->getItemDate() == m_currentDate);
    case Role::IsCurrentMonth:
        return item->getItemDate().month() == m_displayDate.month();
    default:
        return {};
    }
}

QHash<int, QByteArray> CalendarModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("Role"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

CalendarModel::WorkType CalendarModel::getWorKType(QDate date) const
{
    QString type = handleJsMap(date.toString("yyyy"),date.toString("MMdd"));
    if (type == "1") {
        return WorkType::Work;
    } else if (type == "2") {
        return WorkType::Rest;
    } else {
        return WorkType::Default;
    }
}

int CalendarModel::getDisplayYear()
{
    return m_displayDate.year();
}

int CalendarModel::getDisplayMonth()
{
    return m_displayDate.month();
}

int CalendarModel::getCurrentYear()
{
    return m_currentDate.year();
}

int CalendarModel::getCurrentMonth()
{
    return m_currentDate.month();
}

void CalendarModel::displayMonthChange(int value)
{
    if (value == 0) {
        m_displayDate = m_currentDate;
    } else {
        m_displayDate = m_displayDate.addMonths(value);
    }

    updateCalendarData();
    Q_EMIT displayDateChanged();
}

void CalendarModel::displayMonthChangeTo(int month)
{
    if (month < 1 || month > 12) {
        return;
    }

    m_displayDate.setDate(m_displayDate.year(), month, m_displayDate.day());
    updateCalendarData();
    Q_EMIT displayDateChanged();
}

void CalendarModel::displayYearChangeTo(int year)
{
    if (year < 1974 || year > 2094) {
        return;
    }

    m_displayDate.setDate(year, m_displayDate.month(), m_displayDate.day());
    updateCalendarData();
    Q_EMIT displayDateChanged();
}

CalendarModel::CalendarModel(QObject *parent) : QAbstractListModel(parent)
{
    m_currentDate = QDate::currentDate();
    m_displayDate = m_currentDate;
    m_firstdayisSun = false;

    analysisWorktimeJs();
    updateCalendarData();
}

void CalendarModel::updateCalendarData()
{
    int year = m_displayDate.year();
    int month = m_displayDate.month();
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, m_firstdayisSun);
    int daysOfMonth = LunarCalendarInfo::Instance()->getMonthDays(year, month);
    int daysOfPreMonth = LunarCalendarInfo::Instance()->getMonthDays(month == 1 ? year - 1 : year, month == 1 ? 12 : month - 1);

    int endPre, tempYear, tempMonth, tempDay;
    endPre = week == 0 ? 7 : week;

    beginResetModel();
    qDeleteAll(m_item);
    m_item.clear();
    //添加上月天数
    tempYear = year;
    tempMonth = month - 1;
    if (tempMonth < 1) {
        tempYear--;
        tempMonth = 12;
    }

    for (int i = 0; i < endPre; i++) {
        tempDay = daysOfPreMonth - endPre + i + 1;
        QDate date(tempYear, tempMonth, tempDay);
        CalendarItem* item = new CalendarItem(date);
        m_item.append(item);
    }

    //添加本月天数
    tempYear = year;
    tempMonth = month;

    for (int i = 0; i < daysOfMonth; i++) {
        tempDay = i + 1;
        QDate date(tempYear, tempMonth, tempDay);
        CalendarItem* item = new CalendarItem(date);
        m_item.append(item);
    }

    //添加下月天数
    tempYear = year;
    tempMonth = month + 1;
    if (tempMonth > 12) {
        tempYear++;
        tempMonth = 1;
    }

    for (int i = 0; m_item.length() < 42; i++) {
        tempDay = i + 1;
        QDate date(tempYear, tempMonth, tempDay);
        CalendarItem* item = new CalendarItem(date);
        m_item.append(item);
    }
    endResetModel();
}

void CalendarModel::analysisWorktimeJs()
{
    QFile file("://res/jiejiari.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(),&parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug()<< "Parsing json file error !";
        return;
    }
    QJsonObject jsonObject = document.object();
    QStringList args = jsonObject.keys();

    QMap<QString,QString> workTimeInside;
    for (int i=0;i<args.count();i++) {
        if(jsonObject.contains(args.at(i)))
        {
            QJsonValue jsonValueList = jsonObject.value(args.at(i));
            QJsonObject item = jsonValueList.toObject();
            QStringList arg2 = item.keys();
            for (int j=0;j<arg2.count();j++) {
                workTimeInside.insert(arg2.at(j),item[arg2.at(j)].toString());
            }
        }
        m_workTime.insert(args.at(i),workTimeInside);
        workTimeInside.clear();
    }
}

QString CalendarModel::handleJsMap(QString year, QString month2day) const
{
    QString oneNUmber = "worktime.y" + year;
    QString twoNumber = "d" + month2day;

    QMap<QString,QMap<QString,QString>>::const_iterator it = m_workTime.constBegin();

    while(it != m_workTime.end()) {
         if(it.key() == oneNUmber) {
            QMap<QString,QString>::const_iterator it1 = it.value().begin();
            while(it1 != it.value().end()) {
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

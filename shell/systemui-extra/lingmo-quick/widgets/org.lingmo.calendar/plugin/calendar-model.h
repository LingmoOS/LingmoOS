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

#ifndef CALENDARMODEL_H
#define CALENDARMODEL_H

#include <QAbstractListModel>
#include <QDate>

class CalendarItem;
class CalendarModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        Day,
        Week,
        LunarDay,
        Month,
        DateType,
        IsCurrentDay,
        IsCurrentMonth
    };
    Q_ENUM(Role)

    enum WorkType {Default, Work, Rest};
    Q_ENUM(WorkType)

    static CalendarModel *instance();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    WorkType getWorKType(QDate date) const;

    Q_INVOKABLE int getDisplayYear();
    Q_INVOKABLE int getDisplayMonth();
    Q_INVOKABLE int getCurrentYear();
    Q_INVOKABLE int getCurrentMonth();
    Q_INVOKABLE void displayMonthChange(int value);
    Q_INVOKABLE void displayMonthChangeTo(int month);
    Q_INVOKABLE void displayYearChangeTo(int year);

Q_SIGNALS:
    void displayDateChanged();

private:
    CalendarModel(QObject *parent = nullptr);
    void updateCalendarData();
    void analysisWorktimeJs();
    QString handleJsMap(QString year,QString month2day) const;

    QVector<CalendarItem *> m_item;
    QDate m_currentDate;
    QDate m_displayDate;
    bool m_firstdayisSun;
    QMap<QString,QMap<QString,QString>> m_workTime;
};

#endif // CALENDARMODEL_H

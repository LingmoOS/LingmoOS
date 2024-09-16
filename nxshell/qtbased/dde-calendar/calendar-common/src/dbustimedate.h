// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSTIMEDATE_H
#define DBUSTIMEDATE_H

#include <QDBusAbstractInterface>

//通过dbus获取控制中心相关时间设置
class DBusTimedate : public QDBusAbstractInterface
{
    Q_OBJECT
    Q_PROPERTY(int ShortTimeFormat READ shortTimeFormat NOTIFY ShortTimeFormatChanged)
    Q_PROPERTY(int ShortDateFormat READ shortDateFormat NOTIFY ShortDateFormatChanged)
    Q_PROPERTY(int WeekBegins READ weekBegins NOTIFY WeekBeginsChanged)
public:
    explicit DBusTimedate(QObject *parent = nullptr);
    int shortTimeFormat();
    int shortDateFormat();
    Qt::DayOfWeek weekBegins();
signals:
    void ShortDateFormatChanged(int  value) const;
    void ShortTimeFormatChanged(int  value) const;
    void WeekBeginsChanged(int value) const;

public slots:
    void propertiesChanged(const QDBusMessage &msg);
private:
    QVariant getPropertyByName(const char *porpertyName);
    bool getHasDateTimeFormat();
private:
    bool m_hasDateTimeFormat = false;       //是否含有
};

#endif // DBUSTIMEDATE_H

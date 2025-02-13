/***************************************************************************
 *   Copyright (C) 2021 Lingmo OS Team <reionwong@gmail.com>                   *
 *   Copyright (C) 2015 Kai Uwe Broulik <kde@privat.broulik.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef BATTERYHISTORYMODEL_H
#define BATTERYHISTORYMODEL_H

#include <QObject>
#include <QQmlParserStatus>
#include <QPointF>

struct HistoryReply {
public:
    uint time = 0;
    double value = 0.0;
    uint charging = 0;
};

Q_DECLARE_METATYPE(HistoryReply)

class BatteryHistoryModel : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_ENUMS(HistoryType)

    Q_PROPERTY(QString device MEMBER m_device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(uint duration MEMBER m_duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(HistoryType type MEMBER m_type WRITE setType NOTIFY typeChanged)

    Q_PROPERTY(QVariantList points READ asPoints NOTIFY dataChanged)
    Q_PROPERTY(int count READ count NOTIFY dataChanged)
    Q_PROPERTY(int firstDataPointTime READ firstDataPointTime NOTIFY dataChanged)
    Q_PROPERTY(int lastDataPointTime READ lastDataPointTime NOTIFY dataChanged)
    Q_PROPERTY(int largestValue READ largestValue NOTIFY dataChanged)

public:
    enum HistoryType {
        RateType,
        ChargeType
    };

    enum HistoryRoles {
        TimeRole = Qt::UserRole + 1,
        ValueRole,
        ChargingRole
    };

    explicit BatteryHistoryModel(QObject *parent = nullptr);

    void setDevice(const QString &device);
    void setDuration(uint duration);
    void setType(HistoryType type);

    void load();

    void classBegin() override;
    void componentComplete() override;

    QVariantList asPoints() const;
    int count() const;

    int firstDataPointTime() const;
    int lastDataPointTime() const;
    int largestValue() const;

Q_SIGNALS:
    void deviceChanged();
    void typeChanged();
    void durationChanged();

    void dataChanged();

public Q_SLOTS:
    void refresh();

private:
    QString m_device;
    HistoryType m_type;
    uint m_duration; //in seconds

    QList<HistoryReply> m_values;
    bool m_isComplete = false;
};

#endif

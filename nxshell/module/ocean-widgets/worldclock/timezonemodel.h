// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QBasicTimer>
#include <QStandardItemModel>

namespace dwclock {

class TimezoneModel : public QStandardItemModel {
    Q_OBJECT
public:
    enum Timezone {
        CityName = Qt::UserRole + 1,
        ZoneName,
        ZoneOffset
    };

    TimezoneModel(QObject *parent = nullptr);
    virtual ~TimezoneModel();
    void appendItems(const QStringList &timezones);
    void updateTimezone(const QModelIndex &index, const QString &timezone);
    void updateModel(const QStringList &timezones);
    QStringList timezones() const;

    static QStringList defaultLocations();

Q_SIGNALS:
    void timezonesChanged();
    void modifyLocationClicked(const QModelIndex &index);

private Q_SLOTS:
    void emitTimezoneChanged();
    void onModifyLocationActionTriggered();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    void updateTimezone(QStandardItem *item, const QString &timezone);
    void updateTimezoneOffset(QStandardItem *item, const QString &timezone);

    QBasicTimer *m_timezoneChangedTimer = nullptr;
};
}

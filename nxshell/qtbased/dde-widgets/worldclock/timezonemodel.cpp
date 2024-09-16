// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timezonemodel.h"
#include "utils/zoneinfo.h"
#include "utils/timezone.h"

#include <QDebug>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QEvent>
#include <DStyledItemDelegate>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

namespace dwclock {

static QString getTimezoneCity(const QString &timezone)
{
    const QString locale = QLocale::system().name();
    return installer::GetLocalTimezoneName(timezone, locale);
}

TimezoneModel::TimezoneModel(QObject *parent)
    : QStandardItemModel(parent)
{
    connect(this, &TimezoneModel::dataChanged, this, &TimezoneModel::emitTimezoneChanged);
    setItemPrototype(new DStandardItem());
}

TimezoneModel::~TimezoneModel()
{
    if (m_timezoneChangedTimer) {
        m_timezoneChangedTimer->stop();
        delete m_timezoneChangedTimer;
    }
}

void TimezoneModel::appendItems(const QStringList &timezones)
{
    QList<QStandardItem *> items;
    for (int i = 0; i < timezones.count(); ++i) {
        const auto &timezone = timezones[i];
        auto cityName = getTimezoneCity(timezone);
        if (cityName.isEmpty()) {
            qWarning() << "GetZoneInfoByZone() error for the zone:" << timezone;
            cityName = timezone;
        }

        auto itemModel = new DStandardItem();
        itemModel->setDragEnabled(true);
        itemModel->setDropEnabled(false);
        itemModel->setEditable(false);
        auto modifyAction = new DViewItemAction(Qt::AlignVCenter, QSize(), QSize(), true);
        // TODO Action need to release by creater.
        modifyAction->setParent(this);
        // TODO DciIcon
        modifyAction->setIcon(QIcon::fromTheme("edit-rename"));
        // TODO can't get index of action triggered when draged, it maybe a bug in DStandardItem.
        itemModel->setActionList(Qt::RightEdge, {modifyAction});
        connect(modifyAction, &DViewItemAction::triggered, this, &TimezoneModel::onModifyLocationActionTriggered);

        itemModel->setText(cityName);
        itemModel->setData(timezone, ZoneName);
        updateTimezoneOffset(itemModel, timezone);
        items << itemModel;

    }
    appendColumn(items);
}

void TimezoneModel::updateTimezone(const QModelIndex &index, const QString &timezone)
{
    auto itemModel = item(index.row());
    if (!itemModel)
        return;

    const auto &cityName = getTimezoneCity(timezone);
    itemModel->setText(cityName);
    itemModel->setData(timezone, ZoneName);
    updateTimezoneOffset(itemModel, timezone);
}

void TimezoneModel::updateModel(const QStringList &timezones)
{
    // TODO Action need to release by itself.
    for (int i = 0; i < rowCount(); i++) {
        auto itemModel = item(i);
        const auto &actionList = qvariant_cast<DViewItemActionList>(itemModel->data(Dtk::RightActionListRole));
        for (auto item : actionList)
            item->deleteLater();
    }
    clear();
    appendItems(timezones);
}

QStringList TimezoneModel::timezones() const
{
    QStringList result;
    for (int i = 0; i < rowCount(QModelIndex()); i++) {
        result << data(index(i, 0), ZoneName).toString();
    }
    return result;
}

QStringList TimezoneModel::defaultLocations()
{
    return QStringList{"Asia/Beijing", "America/New_York", "Europe/London", "Australia/Sydney"};
}

void TimezoneModel::updateTimezoneOffset(QStandardItem *item, const QString &timezone)
{
    QDBusInterface interface ("org.deepin.dde.Timedate1", "/org/deepin/dde/Timedate1", "org.deepin.dde.Timedate1", QDBusConnection::sessionBus());
    auto reply = interface.asyncCall("GetZoneInfo", timezone);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [this, timezone, item](QDBusPendingCallWatcher* call) {
        static bool Registered = false;
        if (!Registered) {
            registerZoneInfoMetaType();
            Registered = true;
        }

        QDBusPendingReply<ZoneInfo> reply = *call;
        if (reply.isError()) {
            qWarning() << "It's error for GetZoneInfo [" << timezone << "] :" << reply.error().message();
        } else {
            const auto info = reply.value();
            item->setData(info.getUTCOffset(), ZoneOffset);
        }
        call->deleteLater();
    });
}

void TimezoneModel::emitTimezoneChanged()
{
    if (m_timezoneChangedTimer && m_timezoneChangedTimer->isActive())
        return;

    if (!m_timezoneChangedTimer)
        m_timezoneChangedTimer = new QBasicTimer();

    m_timezoneChangedTimer->start(100, this);
}

void TimezoneModel::onModifyLocationActionTriggered()
{
    const auto targetAction = qobject_cast<DViewItemAction *>(sender());
    qDebug() << "onModifyLocationActionTriggered() " << targetAction;
    for (int i = 0; i < rowCount(); i++) {
        auto itemModel = dynamic_cast<DStandardItem *>(item(i));
        if (!itemModel)
            continue;

        const DViewItemActionList actionList = itemModel->actionList(Qt::RightEdge);
        if (actionList.contains(targetAction)) {
            qDebug() << "onModifyLocationActionTriggered() the row clicked:" << i;
            Q_EMIT modifyLocationClicked(index(i, 0));
            break;
        }
    }
}

void TimezoneModel::timerEvent(QTimerEvent *event)
{
    if (m_timezoneChangedTimer && m_timezoneChangedTimer->timerId() == event->timerId()) {
        Q_EMIT timezonesChanged();
        m_timezoneChangedTimer->stop();
    }
}
}

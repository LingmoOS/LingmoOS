// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifyitem.h"

#include <QDateTime>
#include <QLoggingCategory>

#include "notifyaccessor.h"

namespace notification {
Q_DECLARE_LOGGING_CATEGORY(notifyLog)
}
namespace notifycenter {

AppNotifyItem::AppNotifyItem(const NotifyEntity &entity)
    : m_appId(entity.appName())
{
    setEntity(entity);

    auto pin = NotifyAccessor::instance()->applicationPin(m_entity.appName());
    setPinned(pin);
}

void AppNotifyItem::setEntity(const NotifyEntity &entity)
{
    m_entity = entity;
    refresh();
}

NotifyEntity AppNotifyItem::entity() const
{
    Q_ASSERT(m_entity.isValid());
    return m_entity;
}

NotifyType AppNotifyItem::type() const
{
    return NotifyType::Normal;
}

QString AppNotifyItem::appId() const
{
    return m_appId;
}

QString AppNotifyItem::appName() const
{
    Q_ASSERT(m_entity.isValid());
    return m_entity.appName();
}

qint64 AppNotifyItem::id() const
{
    Q_ASSERT(m_entity.isValid());
    return m_entity.id();
}

QString AppNotifyItem::time() const
{
    return m_time;
}

void AppNotifyItem::updateTime()
{
    QDateTime time = QDateTime::fromMSecsSinceEpoch(m_entity.cTime());
    if (!time.isValid())
        return;

    QString ret;
    QDateTime currentTime = QDateTime::currentDateTime();
    auto elapsedDay = time.daysTo(currentTime);
    if (elapsedDay == 0) {
        qint64 msec = QDateTime::currentMSecsSinceEpoch() - m_entity.cTime();
        auto minute = msec / 1000 / 60;
        if (minute <= 0) {
            ret = tr("Just now");
        } else if (minute > 0 && minute < 60) {
            ret = tr("%1 minutes ago").arg(minute);
        } else {
            ret = tr("%1 hours ago").arg(minute / 60);
        }
    } else if (elapsedDay >= 1 && elapsedDay < 2) {
        ret = tr("Yesterday ") + " " + time.toString("hh:mm");
    } else if (elapsedDay >= 2 && elapsedDay < 7) {
        ret = time.toString("ddd hh:mm");
    } else {
        ret = time.toString("yyyy/MM/dd");
    }

    m_time = ret;
}

bool AppNotifyItem::strongInteractive() const
{
    return m_strongInteractive;
}

QString AppNotifyItem::contentIcon() const
{
    Q_ASSERT(m_entity.isValid());
    const auto path = m_entity.bodyIcon();
    return path;
}

QString AppNotifyItem::defaultAction() const
{
    return m_defaultAction;
}

QVariantList AppNotifyItem::actions() const
{
    return m_actions;
}

void AppNotifyItem::updateActions()
{
    QStringList actions = m_entity.actions();
    const auto index = actions.indexOf(QLatin1String("default"));
    if (index >= 0) {
        // default Action maybe have text.
        m_defaultAction = QLatin1String("default");
        if (actions.size() % 2 == 1) {
            actions.remove(index);
        } else {
            actions.remove(index, 2);
        }
    }

    Q_ASSERT(actions.size() % 2 != 1);
    if (actions.size() % 2 == 1) {
        qWarning(notifyLog) << "Actions must be an even number except for default, The notify appName:" << m_entity.appName()
                            << ", actions:" << m_entity.actions();
        return;
    }

    QVariantList array;
    for (int i = 0; i < actions.size(); i += 2) {
        const auto id = actions[i];
        const auto text = actions[i + 1];
        QVariantMap item;
        item["id"] = id;
        item["text"] = text;
        array.append(item);
    }

    m_actions = array;
}

void AppNotifyItem::updateStrongInteractive()
{
    QMap<QString, QVariant> hints = m_entity.hints();
    if (hints.isEmpty())
        return;
    bool ret = false;
    QMap<QString, QVariant>::const_iterator i = hints.constBegin();
    while (i != hints.constEnd()) {
        if (i.key() == QLatin1String("urgency")) {
            ret = i.value().toUInt() == NotifyEntity::Critical;
            break;
        }
        ++i;
    }
    m_strongInteractive = ret;
}

void AppNotifyItem::refresh()
{
    updateTime();
    updateActions();
    updateStrongInteractive();
}

bool AppNotifyItem::pinned() const
{
    return m_pinned;
}

void AppNotifyItem::setPinned(bool newPinned)
{
    m_pinned = newPinned;
}

OverlapAppNotifyItem::OverlapAppNotifyItem(const NotifyEntity &entity)
    : AppNotifyItem(entity)
{
}

NotifyType OverlapAppNotifyItem::type()const
{
    return NotifyType::Overlap;
}

void OverlapAppNotifyItem::updateCount(int source)
{
    int count = source - 1;
    if (count > FullCount) {
        m_count = FullCount;
    } else if (count <= EmptyCount) {
        m_count = EmptyCount;
    } else {
        m_count = count;
    }
}

int OverlapAppNotifyItem::count() const
{
    return m_count;
}

bool OverlapAppNotifyItem::isEmpty() const
{
    return m_count <= EmptyCount;
}

AppGroupNotifyItem::AppGroupNotifyItem(const QString &appName)
    : AppNotifyItem(NotifyEntity(std::numeric_limits<qint64>().max(), appName))
{
}

NotifyType AppGroupNotifyItem::type() const
{
    return NotifyType::Group;
}

void AppGroupNotifyItem::updateLastEntity(const NotifyEntity &entity)
{
    m_lastEntity = entity;
}

NotifyEntity AppGroupNotifyItem::lastEntity() const
{
    return m_lastEntity;
}
}

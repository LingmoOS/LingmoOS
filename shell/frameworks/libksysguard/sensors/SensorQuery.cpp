/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "SensorQuery.h"

#include <QCollator>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QRegularExpression>

#include "SensorDaemonInterface_p.h"
#include "sensors_logging.h"

using namespace KSysGuard;

class SensorQuery::Private
{
public:
    enum class State { Initial, Running, Finished };

    void updateResult(const QDBusPendingReply<SensorInfoMap> &reply);

    QString path;
    State state = State::Initial;
    QList<QPair<QString, SensorInfo>> result;

    QDBusPendingCallWatcher *watcher = nullptr;
};

KSysGuard::SensorQuery::SensorQuery(const QString &path, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->path = path;
}

KSysGuard::SensorQuery::~SensorQuery()
{
}

QString KSysGuard::SensorQuery::path() const
{
    return d->path;
}

void KSysGuard::SensorQuery::setPath(const QString &path)
{
    if (path == d->path) {
        return;
    }

    if (d->state != Private::State::Initial) {
        qCWarning(LIBKSYSGUARD_SENSORS) << "Cannot modify a running or finished query";
        return;
    }

    d->path = path;
}

bool KSysGuard::SensorQuery::execute()
{
    if (d->state != Private::State::Initial) {
        return false;
    }

    d->state = Private::State::Running;

    auto watcher = SensorDaemonInterface::instance()->allSensors();
    d->watcher = watcher;
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [watcher, this]() {
        watcher->deleteLater();
        d->watcher = nullptr;
        d->state = Private::State::Finished;
        d->updateResult(QDBusPendingReply<SensorInfoMap>(*watcher));
        Q_EMIT finished(this);
    });

    return true;
}

bool KSysGuard::SensorQuery::waitForFinished()
{
    if (!d->watcher) {
        return false;
    }

    d->watcher->waitForFinished();
    return true;
}

QStringList KSysGuard::SensorQuery::sensorIds() const
{
    QStringList ids;
    std::transform(d->result.cbegin(), d->result.cend(), std::back_inserter(ids), [](auto entry) {
        return entry.first;
    });
    return ids;
}

void KSysGuard::SensorQuery::sortByName()
{
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(d->result.begin(), d->result.end(), [&collator](const QPair<QString, SensorInfo> &left, const QPair<QString, SensorInfo> &right) {
        return collator.compare(left.second.name, right.second.name) < 0;
    });
}

QList<QPair<QString, SensorInfo>> KSysGuard::SensorQuery::result() const
{
    return d->result;
}

void KSysGuard::SensorQuery::Private::updateResult(const QDBusPendingReply<SensorInfoMap> &reply)
{
    if (path.isEmpty()) { // add everything
        const SensorInfoMap response = reply.value();
        for (auto it = response.constBegin(); it != response.constEnd(); it++) {
            result.append(qMakePair(it.key(), it.value()));
        }
        return;
    }

    auto regexp = QRegularExpression{QStringLiteral("^") % path % QStringLiteral("$")};

    const auto sensorIds = reply.value().keys();
    for (auto id : sensorIds) {
        if (id == path || regexp.match(id).hasMatch()) {
            result.append(qMakePair(id, reply.value().value(id)));
        }
    }
}

/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "sensors_export.h"
#include <QObject>
#include <memory>

namespace KSysGuard
{
class SensorInfo;

/**
 * An object to query the daemon for a list of sensors and their metadata.
 *
 * This class will request a list of sensors from the daemon, then filter them
 * based on the supplied path. The path can include the wildcard "*" to get a
 * list of all sensors matching the specified part of their path. In addition,
 * if left empty, all sensors will be returned.
 */
class SENSORS_EXPORT SensorQuery : public QObject
{
    Q_OBJECT

public:
    SensorQuery(const QString &path = QString{}, QObject *parent = nullptr);
    ~SensorQuery() override;

    QString path() const;
    void setPath(const QString &path);

    /**
     * A list of sensors ids that match the query.
     */
    QStringList sensorIds() const;
    /**
     * Sort the retrieved sensors by their user visible names.
     */
    void sortByName();

    /**
     * Start processing the query.
     */
    bool execute();
    /**
     * Wait for the query to finish.
     *
     * Mostly useful for code that needs the result to be available before
     * continuing. Ideally the finished() signal should be used instead.
     */
    bool waitForFinished();

    Q_SIGNAL void finished(SensorQuery *query);

private:
    friend class Sensor;
    friend class SensorTreeModel;
    QList<QPair<QString, SensorInfo>> result() const;

    class Private;
    const std::unique_ptr<Private> d;
};

} // namespace KSysGuard

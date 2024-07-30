/*
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QScopedPointer>
#include <QString>

#include <KService>

#include "processcore_export.h"

namespace KSysGuard
{
class Process;
class CGroupPrivate;

/**
 * @brief The CGroup class represents a cgroup. This could be a
 * service, slice or scope
 */
class PROCESSCORE_EXPORT CGroup
{
public:
    virtual ~CGroup();
    /**
     * @brief id
     * @return The cgroup ID passed from the constructor
     */
    QString id() const;

    /**
     * @brief Returns metadata about the given service
     * Only applicable for .service entries and really only useful for applications.
     * This KService object is always valid, but may not correspond to a real desktop entry
     * @return
     */
    KService::Ptr service() const;

    /**
     * @brief The list of pids contained in this group.
     * @return A Vector of pids
     */
    QList<pid_t> pids() const;

    /**
     * @internal
     */
    void setPids(const QList<pid_t> &pids);

    /**
     * Request fetching the list of processes associated with this cgroup.
     *
     * This is done in a separate thread. Once it has completed, \p callback is
     * called with the list of pids of this cgroup.
     *
     * It is the callers responsibility to call setPids in response.
     *
     * \param context An object that is used to track if the caller still exists.
     * \param callback A callback that gets called once the list of pids has
     *                 been retrieved.
     */
    void requestPids(QObject *context, std::function<void(QList<pid_t>)> callback);

    /**
     * Returns the base path to exposed cgroup information. Either /sys/fs/cgroup or /sys/fs/cgroup/unified as applicable
     * If cgroups are unavailable this will be an empty string
     */
    static QString cgroupSysBasePath();

private:
    /**
     * Create a new cgroup object for a given cgroup entry
     * The id is the fully formed separated path, such as
     * "system.slice/dbus.service"
     */
    CGroup(const QString &id);

    QScopedPointer<CGroupPrivate> d;
    friend class CGroupDataModel;
    friend class CGroupDataModelPrivate;
};

}

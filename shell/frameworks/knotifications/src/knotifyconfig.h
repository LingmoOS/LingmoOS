/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNOTIFYCONFIG_H
#define KNOTIFYCONFIG_H

#include "knotifications_export.h"
#include <QSharedDataPointer>

class KNotifyConfigPrivate;

/**
 * @class KNotifyConfig knotifyconfig.h KNotifyConfig
 *
 * Represent the configuration for an event
 *
 * @author Olivier Goffart <ogoffart@kde.org>
 * @author Kai Uwe Broulik <kde@broulik.de>
 */
class KNOTIFICATIONS_EXPORT KNotifyConfig
{
public:
    /**
     * Creates a notify config for the given application name and event id
     * @param applicationName The application name, typically the name of the notifyrc file without its extension.
     * @param eventId The notification event ID, i.e. the part after Event/ in its notifyrc file.
     */
    KNotifyConfig(const QString &applicationName, const QString &eventId);
    ~KNotifyConfig();

    KNotifyConfig(const KNotifyConfig &other);
    KNotifyConfig &operator=(const KNotifyConfig &other);

    /**
     * the name of the application that triggered the notification
     */
    QString applicationName() const;

    /**
     * the name of the notification
     */
    QString eventId() const;

    /**
     * Whether there exists an event with the given id under the given application name.
     */
    bool isValid() const;

    /**
     * @return entry from the relevant Global notifyrc config group
     *
     * This will return the configuration from the user for the given key.
     * It first look into the user config file, and then in the global config file.
     *
     * return a null string if the entry doesn't exist
     */
    QString readGlobalEntry(const QString &key) const;

    /**
     * @return entry from the relevant Event/ notifyrc config group
     *
     * This will return the configuration from the user for the given key.
     * It first look into the user config file, and then in the global config file.
     *
     * return a null string if the entry doesn't exist
     */
    QString readEntry(const QString &key) const;

    /**
     * @return path entry from the relevant Event/ notifyrc config group
     *
     * This will return the configuration from the user for the given key
     * and interpret it as a path.
     */
    QString readPathEntry(const QString &key) const;
    /**
     * reparse the cached configs.  to be used when the config may have changed
     */
    static void reparseConfiguration();

    static void reparseSingleConfiguration(const QString &app);

private:
    QSharedDataPointer<KNotifyConfigPrivate> d;
};

#endif

/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KNOTIFYCONFIGELEMENT_H
#define KNOTIFYCONFIGELEMENT_H

#include <QMap>
#include <QString>

class KConfig;
class KConfigGroup;

/**
 * Represent the config for an event
        @author Olivier Goffart <ogoffart@kde.org>
 */
class KNotifyConfigElement
{
public:
    KNotifyConfigElement(const QString &eventid, KConfig *config);
    ~KNotifyConfigElement();

    KNotifyConfigElement(const KNotifyConfigElement &) = delete;
    KNotifyConfigElement &operator=(const KNotifyConfigElement &) = delete;

    QString readEntry(const QString &entry, bool path = false);
    void writeEntry(const QString &entry, const QString &data);

    QString eventId() const;

    void save();

private:
    QMap<QString, QString> m_cache;
    KConfigGroup *m_config;
    QString m_eventId;
};

#endif

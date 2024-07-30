/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2011 Nokia Corporation and/or its subsidiary(-ies).
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR LicenseRef-Qt-Commercial
*/

#ifndef KPROCESSLIST_H
#define KPROCESSLIST_H

#include <QList>
#include <QSharedDataPointer>
#include <QString>
#include <kcoreaddons_export.h>

namespace KProcessList
{
class KProcessInfoPrivate;

/**
 * @brief Contains information about a process. This class is usually not used alone but rather returned by
 * processInfoList and processInfo. To check if the data contained in this class is valid use the isValid method.
 * @since 5.58
 */
class KCOREADDONS_EXPORT KProcessInfo
{
public:
    KProcessInfo();
    KProcessInfo(qint64 pid, const QString &command, const QString &user);
    KProcessInfo(qint64 pid, const QString &command, const QString &name, const QString &user);

    KProcessInfo(const KProcessInfo &other);
    ~KProcessInfo();
    KProcessInfo &operator=(const KProcessInfo &other);
    /**
     * @brief If the KProcessInfo contains valid information. If it returns true the pid, name and user function
     * returns valid information, otherwise they return value is undefined.
     */
    bool isValid() const;
    /**
     * @brief The pid of the process
     */
    qint64 pid() const;
    /**
     * @brief The name of the process. The class will try to get the full path to the executable file for the process
     * but if it is not available the name of the process will be used instead.
     * e.g /bin/ls
     */
    QString name() const;
    /**
     * @brief The username the process is running under.
     */
    QString user() const;
    /**
     * @brief The command line running this process
     * e.g /bin/ls /some/path -R
     * @since 5.61
     */
    QString command() const;

private:
    QSharedDataPointer<KProcessInfoPrivate> d_ptr;
};

typedef QList<KProcessInfo> KProcessInfoList;

/**
 * @brief Retrieves the list of currently active processes.
 * @since 5.58
 */
KCOREADDONS_EXPORT KProcessInfoList processInfoList();

/**
 * @brief Retrieves process information for a specific process-id. If the process is not found a KProcessInfo with
 * isValid == false will be returned.
 * @param pid The process-id to retrieve information for.
 * @since 5.58
 */
KCOREADDONS_EXPORT KProcessInfo processInfo(qint64 pid);

} // KProcessList namespace

Q_DECLARE_TYPEINFO(KProcessList::KProcessInfo, Q_RELOCATABLE_TYPE);

#endif // KPROCESSLIST_H

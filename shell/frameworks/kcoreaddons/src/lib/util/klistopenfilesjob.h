/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2010 Jacopo De Simoi <wilderkde@gmail.com>
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KLISTOPENFILESJOB_H
#define KLISTOPENFILESJOB_H

#include <QObject>
#include <QString>
#include <kcoreaddons_export.h>
#include <kjob.h>
#include <kprocesslist.h>

#include <memory>

class KListOpenFilesJobPrivate;

/**
 * @brief Provides information about processes that have open files in a given path or subdirectory of path.
 *
 * When start() is invoked it starts to collect information about processes that have any files open in path or a
 * subdirectory of path. When it is done the KJob::result signal is emitted and the result can be retrieved with the
 * processInfoList function.
 *
 * On Unix like systems the lsof utility is used to get the list of processes.
 * On Windows the listing always fails with error code NotSupported.
 *
 * @since 5.63
 */
class KCOREADDONS_EXPORT KListOpenFilesJob : public KJob
{
    Q_OBJECT
public:
    explicit KListOpenFilesJob(const QString &path);
    ~KListOpenFilesJob() override;
    void start() override;
    /**
     * @brief Returns the list of processes with open files for the requested path
     * @return The list of processes with open files for the requested path
     */
    KProcessList::KProcessInfoList processInfoList() const;

public:
    /**
     * @brief Special error codes emitted by KListOpenFilesJob
     *
     * The KListOpenFilesJob uses the error codes defined here besides the standard error codes defined by KJob
     */
    enum class Error {
        /*** Indicates that the platform doesn't support listing open files by processes */
        NotSupported = KJob::UserDefinedError + 1,
        /*** Internal error has ocurred */
        InternalError = KJob::UserDefinedError + 2,
        /*** The specified path does not exist */
        DoesNotExist = KJob::UserDefinedError + 11,
    };

private:
    friend class KListOpenFilesJobPrivate;
    std::unique_ptr<KListOpenFilesJobPrivate> const d;
};

#endif // KLISTOPENFILESJOB_H

/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef UNPACKFSC_FSARCHIVERRUNNER_H
#define UNPACKFSC_FSARCHIVERRUNNER_H

#include "Runners.h"

/** @brief Base class for runners of FSArchiver
 *
 */
class FSArchiverRunner : public Runner
{
    Q_OBJECT
public:
    using Runner::Runner;

protected Q_SLOTS:
    void fsarchiverProgress( QString line );

protected:
    /** @brief Checks prerequisites, sets full path of fsarchiver in @p executable
     */
    Calamares::JobResult checkPrerequisites( QString& executable ) const;
    Calamares::JobResult checkDestination( QString& destinationPath ) const;

    int m_since = 0;
};

/** @brief Running FSArchiver in **dir** mode
 *
 */
class FSArchiverDirRunner : public FSArchiverRunner
{
public:
    using FSArchiverRunner::FSArchiverRunner;

    Calamares::JobResult run() override;
};

/** @brief Running FSArchiver in **dir** mode
 *
 */
class FSArchiverFSRunner : public FSArchiverRunner
{
public:
    using FSArchiverRunner::FSArchiverRunner;

    Calamares::JobResult run() override;
};


#endif

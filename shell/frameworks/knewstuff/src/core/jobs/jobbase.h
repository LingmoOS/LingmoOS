/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef JOBBASE_H
#define JOBBASE_H

#include <KJob>

namespace KNSCore
{
enum JobFlag {
    None = 0,
    HideProgressInfo = 1,
    Resume = 2,
    Overwrite = 4,
    DefaultFlags = None,
};
Q_DECLARE_FLAGS(JobFlags, JobFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(JobFlags)

enum LoadType { Reload, NoReload };

}

#endif // JOBBASE_H

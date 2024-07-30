/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Ashish Bansal <bansal.ashish096@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "engine_export.h"
#include <QString>

namespace Baloo {
    class Database;

    /*
     * If BALOO_DB_PATH environment variable is set, then it returns value of that variable.
     * Otherwise returns the default database path.
     */
    BALOO_ENGINE_EXPORT QString fileIndexDbPath();

    /*
     * lmdb doesn't support opening database twice at the same time in the single process
     * because if we open database twice at the same time and closes one of them, then it
     * would invalidate the handles of both the instances and may lead to crash or some
     * other undesirable behaviour. So, keeping one global database would solve this problem
     * and improve the performance too.
     */
    BALOO_ENGINE_EXPORT Database* globalDatabaseInstance();
}

#endif // GLOBAL_H

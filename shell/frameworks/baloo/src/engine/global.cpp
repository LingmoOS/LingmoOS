/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Ashish Bansal <bansal.ashish096@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "database.h"
#include "global.h"

#include <QStandardPaths>
using namespace Baloo;

Q_GLOBAL_STATIC_WITH_ARGS(Database, s_db, (fileIndexDbPath()))

QString Baloo::fileIndexDbPath()
{
    QString envBalooPath = QString::fromLocal8Bit(qgetenv("BALOO_DB_PATH"));
    if (!envBalooPath.isEmpty()) {
        return envBalooPath;
    }

    static QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/baloo");
    return path;
}

Database* Baloo::globalDatabaseInstance()
{
    return s_db;
}

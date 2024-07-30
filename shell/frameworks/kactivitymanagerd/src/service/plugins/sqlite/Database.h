/*
 *   SPDX-FileCopyrightText: 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

// Qt
#include <QDateTime>
#include <QSqlError>
#include <QString>

// Utils
#include <utils/d_ptr.h>

// Local
#include <common/database/Database.h>

namespace Common
{
class Database;
} // namespace Common

class ResourcesDatabaseInitializer
{
public:
    // static Database *self();

private:
    ResourcesDatabaseInitializer();
    ~ResourcesDatabaseInitializer();

    void initDatabase(bool retryOnFail = true);

    D_PTR;

    friend Common::Database::Ptr resourcesDatabase();
};

Common::Database::Ptr resourcesDatabase();

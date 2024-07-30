/*
 *   SPDX-FileCopyrightText: 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

// Qt
#include <QString>

// Utils
#include <utils/d_ptr.h>

/**
 * ResourceScoreCache handles the persistence of the usage ratings for
 * the resources.
 *
 * It contains the logic to update the score of a single resource.
 */
class ResourceScoreCache
{
public:
    ResourceScoreCache(const QString &activity, const QString &application, const QString &resource);
    virtual ~ResourceScoreCache();

    void update();

private:
    D_PTR;
    class Queries;
};

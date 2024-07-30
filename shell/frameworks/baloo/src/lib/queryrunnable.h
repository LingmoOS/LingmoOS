/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef QUERYRUNNABLE_H
#define QUERYRUNNABLE_H

#include "query.h"
#include <QRunnable>
#include <QObject>

#include <memory>

namespace Baloo {

/**
 * @class QueryRunnable queryrunnable.h <Baloo/QueryRunnable>
 */
class BALOO_CORE_EXPORT QueryRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    QueryRunnable(const Query& query, QObject* parent = nullptr);
    ~QueryRunnable() override;
    void run() override;

    void stop();

Q_SIGNALS:
    void queryResult(Baloo::QueryRunnable* queryRunnable, const QString& filePath);
    void finished(Baloo::QueryRunnable* queryRunnable);

private:
    class Private;
    std::unique_ptr<Private> const d;
};

}

#endif // QUERYRUNNABLE_H

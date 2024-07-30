/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dbusfuture_p.h"

namespace DBusFuture
{
namespace detail
{ //_

template<>
void DBusCallFutureInterface<void>::callFinished()
{
    deleteLater();

    // qDebug() << "This is call end";

    this->reportFinished();
}

ValueFutureInterface<void>::ValueFutureInterface()
{
}

QFuture<void> ValueFutureInterface<void>::start()
{
    auto future = this->future();

    this->reportFinished();

    deleteLater();

    return future;
}

} //^ namespace detail

QFuture<void> fromVoid()
{
    using namespace detail;

    auto valueFutureInterface = new ValueFutureInterface<void>();

    return valueFutureInterface->start();
}

} // namespace DBusFuture

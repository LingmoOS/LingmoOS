/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_CONS_KJOBFUTURE_H
#define ASYNQT_CONS_KJOBFUTURE_H

#include <KJob>

#include <QFuture>
#include <QFutureInterface>
#include <QObject>

#include <memory>

#include "../private/wrappers/kjob_p.h"

namespace AsynQt
{
/**
 * Creates a future from the specified dbus reply
 */
template<typename _Result>
QFuture<_Result> makeFuture(KJob *job)
{
    using namespace detail;

    return (new KJobFutureInterface<_Result>(job))->start();
}

} // namespace AsynQt

#endif // ASYNQT_CONS_DBUSFUTURE_H

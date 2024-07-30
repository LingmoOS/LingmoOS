/*
    SPDX-FileCopyrightText: 2014-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "mainthreadexecutor_p.h"

#include <mutex>

#include <QCoreApplication>
#include <QMetaObject>
#include <QThread>

namespace KActivities
{
namespace detail
{
MainThreadExecutor::MainThreadExecutor(std::function<void()> &&f)
    : m_function(std::forward<std::function<void()>>(f))
{
}

void MainThreadExecutor::start()
{
    m_function();
    deleteLater();
}

} // namespace detail

void runInMainThread(std::function<void()> &&f)
{
    static auto mainThread = QCoreApplication::instance()->thread();

    if (QThread::currentThread() == mainThread) {
        f();

    } else {
        auto executor = new detail::MainThreadExecutor(std::forward<std::function<void()>>(f));

        executor->moveToThread(mainThread);

        QMetaObject::invokeMethod(executor, "start", Qt::BlockingQueuedConnection);
    }
}

} // namespace KActivities

#include "moc_mainthreadexecutor_p.cpp"

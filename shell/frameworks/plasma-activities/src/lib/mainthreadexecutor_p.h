/*
    SPDX-FileCopyrightText: 2014-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_MAINTHREADEXECUTOR_P
#define ACTIVITIES_MAINTHREADEXECUTOR_P

#include <functional>

#include <QObject>

namespace KActivities
{
namespace detail
{
class MainThreadExecutor : public QObject
{
    Q_OBJECT

public:
    MainThreadExecutor(std::function<void()> &&f);

    Q_INVOKABLE void start();

private:
    std::function<void()> m_function;
};
} // namespace detail

void runInMainThread(std::function<void()> &&f);

} // namespace KActivities

#endif // ACTIVITIES_MAINTHREADEXECUTOR_P

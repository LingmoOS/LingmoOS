/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef ASYNQT_CONS_PROCESS_H
#define ASYNQT_CONS_PROCESS_H

#include <QFuture>
#include <QFutureInterface>
#include <QProcess>

#include "../private/wrappers/process_p.h"

namespace AsynQt
{
/**
 * Creates a future that will be completed when the process finishes.
 * @arg process process to wrap inside a future
 * @arg map function that extracts the needed information from the process
 *
 * <code>
 *     // If we want to get the exit code of the process
 *     makeFuture(process, [] (QProcess *p) { return p->exitCode(); })
 *
 *     // If we want to get the output of the process
 *     makeFuture(process, [] (QProcess *p) { return p->readAllStandardOutput(); })
 * </code>
 */
template<typename _Function>
auto makeFuture(QProcess *process, _Function map) -> QFuture<decltype(map(Q_NULLPTR))>
{
    using namespace detail;

    auto futureInterface = new ProcessFutureInterface<decltype(map(Q_NULLPTR)), _Function>(process, map);

    return futureInterface->start();
}

inline auto makeFuture(QProcess *process) -> QFuture<QProcess *>
{
    using namespace detail;

    auto id = [](QProcess *process) {
        return process;
    };

    auto futureInterface = new ProcessFutureInterface<QProcess *, decltype(id)>(process, id);

    return futureInterface->start();
}

namespace Process
{
/**
 * Executes the specified command, with the specified arguments.
 */
template<typename _Function>
auto exec(const QString &command, const QStringList &arguments, _Function &&map) -> QFuture<decltype(map(Q_NULLPTR))>
{
    // TODO: Where to delete this process?
    auto process = new QProcess();

    // process->setProcessChannelMode(QProcess::ForwardedChannels);
    process->setProgram(command);
    process->setArguments(arguments);

    return AsynQt::makeFuture(process, std::forward<_Function>(map));
}

/**
 * Executes the specified command, with the specified arguments,
 * and returns the future containing the process it created.
 */
inline QFuture<QProcess *> exec(const QString &command, const QStringList &arguments)
{
    return exec(command, arguments, [](QProcess *process) {
        return process;
    });
}

/**
 * Executes the specified command, with the specified arguments,
 * and it returns a future containing the output of that command.
 */
inline QFuture<QByteArray> getOutput(const QString &command, const QStringList &arguments)
{
    return exec(command, arguments, [](QProcess *process) {
        return process->readAllStandardOutput();
    });
}

} // namespace Process

} // namespace qfuture

#endif // ASYNQT_CONS_PROCESS_H

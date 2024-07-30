/*
 *   SPDX-FileCopyrightText: 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

//
// W A R N I N G
// -------------
//
// This file is not part of the AsynQt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <asynqt/operations/transform.h>


namespace AsynQt
{
namespace detail
{
template<typename _Result, typename _Function>
class ProcessFutureInterface : public QObject, public QFutureInterface<_Result>
{
public:
    ProcessFutureInterface(QProcess *process, _Function map)
        : m_process(process)
        , m_map(map)
    {
    }

    QFuture<_Result> start()
    {
        m_running = true;
        auto onProcessFinished = [this]() {
            this->finished();
        };
        QObject::connect(m_process,
                         // Pretty new Qt connect syntax :)
                         (void (QProcess::*)(int, QProcess::ExitStatus)) & QProcess::finished,
                         this,
                         onProcessFinished,
                         Qt::QueuedConnection);

        QObject::connect(m_process, &QProcess::errorOccurred, this, onProcessFinished, Qt::QueuedConnection);

        this->reportStarted();

        m_process->start();

        return this->future();
    }

    void finished();

private:
    QProcess *m_process;
    _Function m_map;
    bool m_running;
};

template<typename _Result, typename _Function>
void ProcessFutureInterface<_Result, _Function>::finished()
{
    if (m_running) {
        m_running = false;
        this->reportResult(m_map(m_process));
        this->reportFinished();
    }
}

} // namespace detail
} // namespace AsynQt

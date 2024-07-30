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

namespace AsynQt
{
namespace detail
{
template<typename _Result>
class KJobFutureInterface : public QObject, public QFutureInterface<_Result>
{
public:
    KJobFutureInterface(KJob *job)
        : job(job)
    {
        job->setAutoDelete(false);
    }

    ~KJobFutureInterface() override
    {
    }

    void callFinished();

    QFuture<_Result> start()
    {
        auto onCallFinished = [this]() {
            callFinished();
        };
        QObject::connect(job, &KJob::result, this, onCallFinished, Qt::QueuedConnection);

        this->reportStarted();

        job->start();

        return this->future();
    }

private:
    KJob *job;
};

template<typename _Result>
void KJobFutureInterface<_Result>::callFinished()
{
    this->reportResult(job);
    this->reportFinished();

    deleteLater();
}

template<>
void KJobFutureInterface<void>::callFinished()
{
    this->reportFinished();

    job->deleteLater();
    deleteLater();
}

} // namespace detail
} // namespace AsynQt

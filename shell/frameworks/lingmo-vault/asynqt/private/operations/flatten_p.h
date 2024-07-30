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

#include <type_traits>

#include "../utils_p.h"

namespace AsynQt
{
namespace detail
{
template<typename _Result>
class FlattenFutureInterface : public QObject, public QFutureInterface<_Result>
{
public:
    FlattenFutureInterface(QFuture<QFuture<_Result>> future)
        : m_outerFuture(future)
    {
    }

    inline void setFutureResultAt(int, std::true_type /* _Result is void */)
    {
        // nothing to do
    }

    inline void setFutureResultAt(int index, std::false_type /* _Result is not void */)
    {
        this->reportResult(m_currentInnerFuture.resultAt(index));
    }

    void processNextInnerFuture()
    {
        // Already processing something
        if (m_innerFutureWatcher)
            return;

        m_innerFutureWatcher.reset(new QFutureWatcher<_Result>());
        m_currentInnerFuture = m_innerFutures.head();

        onFinished(m_innerFutureWatcher, [this]() {
            dequeueInnerFuture();
        });

        onCanceled(m_innerFutureWatcher, [this]() {
            this->reportCanceled();
        });

        onResultReadyAt(m_innerFutureWatcher, [this](int index) {
            setFutureResultAt(index, typename std::is_void<_Result>::type());
        });

        m_innerFutureWatcher->setFuture(m_currentInnerFuture);

        this->reportStarted();
    }

    void dequeueInnerFuture()
    {
        m_innerFutureWatcher.reset();
        m_innerFutures.dequeue();

        if (m_innerFutures.size() == 0) {
            if (m_outerFuture.isCanceled()) {
                this->reportCanceled();
            }

            if (m_outerFuture.isFinished()) {
                this->reportFinished();
            }

        } else {
            processNextInnerFuture();
        }
    }

    QFuture<_Result> start()
    {
        m_outerFutureWatcher.reset(new QFutureWatcher<QFuture<_Result>>());

        onFinished(m_outerFutureWatcher, [this]() {
            if (m_innerFutures.isEmpty()) {
                this->reportFinished();
            }
        });

        onCanceled(m_outerFutureWatcher, [this]() {
            if (m_innerFutures.isEmpty()) {
                this->reportCanceled();
            }
        });

        onResultReadyAt(m_outerFutureWatcher, [this](int index) {
            m_innerFutures.enqueue(m_outerFuture.resultAt(index));
            processNextInnerFuture();
        });

        m_outerFutureWatcher->setFuture(m_outerFuture);

        this->reportStarted();

        return this->future();
    }

private:
    QFuture<QFuture<_Result>> m_outerFuture;
    std::unique_ptr<QFutureWatcher<QFuture<_Result>>> m_outerFutureWatcher;

    QFuture<_Result> m_currentInnerFuture;
    QQueue<QFuture<_Result>> m_innerFutures;
    std::unique_ptr<QFutureWatcher<_Result>> m_innerFutureWatcher;
};

template<typename _Result>
QFuture<_Result> flatten_impl(const QFuture<QFuture<_Result>> &future)
{
    return (new FlattenFutureInterface<_Result>(future))->start();
}

namespace operators
{
struct FlattenModifier {
};

template<typename _Result>
QFuture<_Result> operator|(const QFuture<QFuture<_Result>> &future, FlattenModifier)
{
    return flatten_impl(future);
}

} // namespace operators

} // namespace detail
} // namespace AsynQt

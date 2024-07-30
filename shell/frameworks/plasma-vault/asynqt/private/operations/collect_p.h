/*
 *   SPDX-FileCopyrightText: 2017 Ivan Čukić <ivan.cukic(at)kde.org>
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

#include "../cxxutils_p.h"
#include "../utils_p.h"

namespace AsynQt
{
namespace detail
{
template<typename... _Results>
class CollectFutureInterface : public QObject, public QFutureInterface<std::tuple<_Results...>>
{
public:
    CollectFutureInterface(QFuture<_Results>... futures)
        : m_waitingCount(m_count)
        , m_futures(std::make_tuple(futures...))
    {
    }

    template<int index>
    bool connectFuture()
    {
        auto future = std::get<index>(m_futures);
        auto &watcher = std::get<index>(m_watchers);

        onFinished(watcher, [this] {
            m_waitingCount--;

            // We are saving the result
            std::get<index>(m_results) = std::get<index>(m_futures).result();

            // If all futures are done, report the result
            if (m_waitingCount == 0) {
                this->reportResult(m_results);
                this->reportFinished();
            }
        });

        onCanceled(watcher, [this] {
            // If any of the futures fail, we can not return
            // the result :(
            this->reportCanceled();
        });

        watcher.setFuture(future);

        return true;
    }

    template<int... Indices>
    void connectFutures(index_sequence<Indices...>)
    {
        auto ignore = {connectFuture<Indices>()...};
        Q_UNUSED(ignore);
    }

    QFuture<std::tuple<_Results...>> start()
    {
        // we need to call connectFuture for index = 0 to sizeof..._Results
        connectFutures(make_index_sequence<m_count>());

        this->reportStarted();

        return this->future();
    }

private:
    static const constexpr std::size_t m_count = sizeof...(_Results);
    int m_waitingCount;
    std::tuple<QFuture<_Results>...> m_futures;
    std::tuple<QFutureWatcher<_Results>...> m_watchers;
    std::tuple<_Results...> m_results;
};

template<typename... _Results>
QFuture<std::tuple<_Results...>> collect_impl(QFuture<_Results>... futures)
{
    return (new CollectFutureInterface<_Results...>(futures...))->start();
}

} // namespace detail
} // namespace AsynQt

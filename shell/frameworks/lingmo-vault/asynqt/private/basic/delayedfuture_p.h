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
class DelayedFutureInterface : public QObject, public QFutureInterface<_Result>
{
public:
    DelayedFutureInterface(_Result value, int milliseconds)
        : m_value(value)
        , m_milliseconds(milliseconds)
    {
    }

    QFuture<_Result> start()
    {
        auto future = this->future();

        this->reportStarted();

        QTimer::singleShot(m_milliseconds, [this] {
            this->reportResult(m_value);
            this->reportFinished();
            deleteLater();
        });

        return future;
    }

private:
    _Result m_value;
    int m_milliseconds;
};

template<typename T = void>
class DelayedVoidFutureInterface : public QObject, QFutureInterface<void>
{
public:
    DelayedVoidFutureInterface(int milliseconds)
        : m_milliseconds(milliseconds)
    {
    }

    QFuture<void> start()
    {
        auto future = this->future();

        this->reportStarted();

        QTimer::singleShot(m_milliseconds, [this] {
            this->reportFinished();
            deleteLater();
        });

        deleteLater();

        return future;
    }

private:
    int m_milliseconds;
};

template<typename _Result>
DelayedFutureInterface<typename std::decay<_Result>::type> *newDelayedFutureInterface(_Result &&result, int milliseconds)
{
    return new DelayedFutureInterface<typename std::decay<_Result>::type>(std::forward<_Result>(result), milliseconds);
}

} // namespace detail
} // namespace AsynQt

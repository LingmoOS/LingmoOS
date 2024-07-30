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

#include "../cxxutils_p.h"
#include "../utils_p.h"

namespace AsynQt
{
namespace detail
{
template<typename _In, typename _Transformation>
struct TransformFutureInterfaceHelper {
    typedef typename std::invoke_result<_Transformation &, _In &&>::type result_type;
};

template<typename... _In, typename _Transformation>
struct TransformFutureInterfaceHelper<std::tuple<_In...>, _Transformation> {
    typedef typename std::invoke_result<_Transformation &, _In &&...>::type result_type;
};

template<typename _Transformation>
struct TransformFutureInterfaceHelper<void, _Transformation> {
    typedef typename std::invoke_result<_Transformation &>::type result_type;
};

template<typename _In, typename _Transformation>
class TransformFutureInterface : public QObject, public QFutureInterface<typename TransformFutureInterfaceHelper<_In, _Transformation>::result_type>
{
public:
    typedef typename TransformFutureInterfaceHelper<_In, _Transformation>::result_type result_type;
    typedef typename std::is_void<_In>::type in_type_is_void;
    typedef typename std::is_void<result_type> result_type_is_void;

    TransformFutureInterface(QFuture<_In> future, _Transformation transformation)
        : m_future(future)
        , m_transformation(transformation)
    {
    }

    ~TransformFutureInterface() override
    {
    }

    // If _In is void, we are never going to get a result,
    // so, we need to pretend like we got one when the
    // future is successfully finished
    inline void setFutureResultOnFinished(std::true_type, // _In is void
                                          std::true_type // result_type is void
    )
    {
        // no value, no result to create, but we still
        // want to call the transformation function
        if (!m_future.isCanceled()) {
            m_transformation();
        }
    }

    inline void setFutureResultOnFinished(std::true_type, // _In is void
                                          std::false_type // result_type is not void
    )
    {
        if (!m_future.isCanceled()) {
            this->reportResult(m_transformation());
        }
    }

    // Ignore id _In is not void
    template<typename T>
    inline void setFutureResultOnFinished(std::false_type, T)
    {
    }

    // We need to test whether we can unpack a tuple to call
    // the transformation
    template<typename _InType>
    result_type applyTransformation(_InType &&result)
    {
        return m_transformation(std::forward<_InType>(result));
    }

    template<typename _Tuple, int... _Indices>
    result_type applyTransformationImpl(_Tuple &&result, index_sequence<_Indices...>)
    {
        return m_transformation(std::get<_Indices>(result)...);
    }

    template<typename... _InTypes>
    result_type applyTransformation(std::tuple<_InTypes...> &&result)
    {
        return applyTransformationImpl(std::forward<std::tuple<_InTypes...>>(result), make_index_sequence<sizeof...(_InTypes)>());
    }

    // If _In is not void, then all is as it should be
    inline void setFutureResultAt(int index,
                                  std::false_type, // _In is not void
                                  std::true_type // result_type is void
    )
    {
        // nothing to do with the value, but we still
        // want to call the transformation function
        applyTransformation(m_future.resultAt(index));
    }

    inline void setFutureResultAt(int index,
                                  std::false_type, // _In is not void
                                  std::false_type // result_type is not void
    )
    {
        this->reportResult(applyTransformation(m_future.resultAt(index)));
    }

    template<typename T>
    inline void setFutureResultAt(int, std::true_type, T)
    {
    }

    QFuture<result_type> start()
    {
        m_futureWatcher.reset(new QFutureWatcher<_In>());

        onFinished(m_futureWatcher, [this]() {
            setFutureResultOnFinished(in_type_is_void(), result_type_is_void());
            this->reportFinished();
        });

        onCanceled(m_futureWatcher, [this]() {
            this->reportCanceled();
        });

        onResultReadyAt(m_futureWatcher, [this](int index) {
            setFutureResultAt(index, in_type_is_void(), result_type_is_void());
        });

        m_futureWatcher->setFuture(m_future);

        this->reportStarted();

        return this->future();
    }

private:
    QFuture<_In> m_future;
    _Transformation m_transformation;
    std::unique_ptr<QFutureWatcher<_In>> m_futureWatcher;
};

template<typename _In, typename _Transformation>
QFuture<typename detail::TransformFutureInterface<_In, _Transformation>::result_type> transform_impl(const QFuture<_In> &future,
                                                                                                     _Transformation &&transformation)
{
    return (new TransformFutureInterface<_In, _Transformation>(future, std::forward<_Transformation>(transformation)))->start();
}

namespace operators
{
template<typename _Transformation>
class TransformationModifier
{
public:
    TransformationModifier(_Transformation transformation)
        : m_transformation(transformation)
    {
    }

    _Transformation m_transformation;
};

template<typename _In, typename _Transformation>
auto operator|(const QFuture<_In> &future, TransformationModifier<_Transformation> &&modifier) -> decltype(transform_impl(future, modifier.m_transformation))
{
    return transform_impl(future, modifier.m_transformation);
}

} // namespace operators

} // namespace detail
} // namespace AsynQt

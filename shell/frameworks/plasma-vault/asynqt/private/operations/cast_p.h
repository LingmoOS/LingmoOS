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
template<typename _Out, typename _In>
QFuture<_Out> qfuture_cast_impl(const QFuture<_In> &future)
{
    return transform(future, [](const _In &value) -> _Out {
        return static_cast<_Out>(value);
    });
}

namespace operators
{
template<typename _Out>
struct CastModifier {
};

template<typename _In, typename _Out>
auto operator|(const QFuture<_In> &future, CastModifier<_Out> &&) -> decltype(qfuture_cast_impl<_Out>(future))
{
    return qfuture_cast_impl<_Out>(future);
}

} // namespace operators

}
}

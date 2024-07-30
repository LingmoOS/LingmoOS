// SPDX-License-Identifier: LGPL-2.0-or-later

#include "exception_p.h"

#include <exception>

#include <QUnhandledException>

namespace
{
std::optional<KCrash::ExceptionMetadata> qUnhandledExceptionMetadata(const QUnhandledException &exception)
{
    auto exceptionPtr = exception.exception();
    if (!exceptionPtr) {
        return {};
    }

    try {
        std::rethrow_exception(exceptionPtr);
    } catch (const std::exception &e) {
        return KCrash::ExceptionMetadata{
            .ptr = exceptionPtr,
            .klass = typeid(e).name(),
            .what = e.what(),
        };
    }

    return {};
}
} // namespace

namespace KCrash
{
std::optional<ExceptionMetadata> exceptionMetadata()
{
    auto exceptionPtr = std::current_exception();
    if (!exceptionPtr) {
        return {};
    }

    try {
        std::rethrow_exception(exceptionPtr);
    } catch (const QUnhandledException &e) {
        return qUnhandledExceptionMetadata(e);
    } catch (const std::bad_alloc &e) {
        return {};
    } catch (const std::exception &e) {
        return KCrash::ExceptionMetadata{
            .ptr = exceptionPtr,
            .klass = typeid(e).name(),
            .what = e.what(),
        };
    }

    return {};
}
} // namespace KCrash

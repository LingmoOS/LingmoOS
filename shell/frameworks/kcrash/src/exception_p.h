// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include "kcrash_export.h"

#include <exception>
#include <optional>

namespace KCrash
{
struct KCRASH_NO_EXPORT ExceptionMetadata {
    std::exception_ptr ptr;
    const char *klass;
    const char *what;
};

KCRASH_NO_EXPORT std::optional<ExceptionMetadata> exceptionMetadata();
} // namespace KCrash

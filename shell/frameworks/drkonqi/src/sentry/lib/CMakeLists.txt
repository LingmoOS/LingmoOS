# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

set(sentry_SRCS)
ecm_qt_declare_logging_category(
    sentry_SRCS
    HEADER debug.h
    IDENTIFIER SENTRY_DEBUG
    CATEGORY_NAME org.kde.drkonqi.sentry
    DESCRIPTION "drkonqi-sentry"
    EXPORT DRKONQI
)

add_library(DrKonqiSentryInternal STATIC
    ${sentry_SRCS}
    sentryconnection.cpp
    sentrydsns.cpp
    sentryenvelope.cpp
    sentrypostbox.cpp
    sentrypaths.cpp
    sentrypostman.cpp)
target_link_libraries(DrKonqiSentryInternal
    Qt::Core
    Qt::Network
)

# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

if (CMAKE_CROSSCOMPILING)
    return()
endif()

add_executable(ki18n-iso-codes-cachegen)
target_include_directories(ki18n-iso-codes-cachegen PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)
target_sources(ki18n-iso-codes-cachegen PRIVATE
    cachegen.cpp
    ../isocodescache.cpp
)
ecm_qt_declare_logging_category(ki18n-iso-codes-cachegen
    HEADER logging.h
    IDENTIFIER KI18NLD
    CATEGORY_NAME kf.i18n.localeData
    DESCRIPTION "KI18n Locale Data"
)
target_link_libraries(ki18n-iso-codes-cachegen PRIVATE
    Qt6::Core
)

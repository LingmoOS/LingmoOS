# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

option(KI18N_EMBEDDED_ISO_CODES_CACHE "Use compiled-in iso-codes data." OFF)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config-localedata.h.in" "${CMAKE_CURRENT_BINARY_DIR}/config-localedata.h")
add_subdirectory(cachegen)

add_library(KF6I18nLocaleData)
add_library(KF6::I18nLocaleData ALIAS KF6I18nLocaleData)

set_target_properties(KF6I18nLocaleData PROPERTIES
    VERSION     ${KI18N_VERSION}
    SOVERSION   ${KI18N_SOVERSION}
    EXPORT_NAME I18nLocaleData
)

target_sources(KF6I18nLocaleData PRIVATE
    isocodes.cpp
    isocodescache.cpp
    kcountry.cpp
    kcountrysubdivision.cpp
    ktimezone.cpp
    spatial_index.cpp
    spatial_index_entry.cpp
    spatial_index_property.cpp
    timezonedata.cpp
)

ecm_generate_export_header(KF6I18nLocaleData
    BASE_NAME KI18nLocaleData
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KI18n
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

ecm_qt_declare_logging_category(KF6I18nLocaleData
    HEADER logging.h
    IDENTIFIER KI18NLD
    CATEGORY_NAME kf.i18n.localeData
    DESCRIPTION "KI18n Locale Data"
    EXPORT KI18N
)

if (KI18N_EMBEDDED_ISO_CODES_CACHE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iso_3166-1
        COMMAND ki18n-iso-codes-cachegen --input ${IsoCodes_PREFIX}/share/iso-codes/json/iso_3166-1.json --output ${CMAKE_CURRENT_BINARY_DIR}/iso_3166-1 --code 3166-1
        DEPENDS ${IsoCodes_PREFIX}/share/iso-codes/json/iso_3166-1.json
        COMMENT "Generating ISO 3166-1 cache"
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/iso_3166-2
        COMMAND ki18n-iso-codes-cachegen --input ${IsoCodes_PREFIX}/share/iso-codes/json/iso_3166-2.json --output ${CMAKE_CURRENT_BINARY_DIR}/iso_3166-2 --code 3166-2
        DEPENDS ${IsoCodes_PREFIX}/share/iso-codes/json/iso_3166-2.json
        COMMENT "Generating ISO 3166-2 cache"
    )
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/isocodescache.qrc ${CMAKE_CURRENT_BINARY_DIR}/isocodescache.qrc)
    target_sources(KF6I18nLocaleData PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/isocodescache.qrc)
endif()

target_include_directories(KF6I18nLocaleData
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KI18nLocaleData>"
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KI18n>" # for version header
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>" # for version header
)

target_link_libraries(KF6I18nLocaleData PUBLIC Qt6::Core)
target_link_libraries(KF6I18nLocaleData PRIVATE KF6I18n)
target_compile_options(KF6I18n PRIVATE -DTRANSLATION_DOMAIN=\"ki18n6\")

install(TARGETS KF6I18nLocaleData EXPORT KF6I18nTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(KI18nLocaleData_HEADERS
  HEADER_NAMES
    KCountry
    KCountrySubdivision
    KTimeZone
    REQUIRED_HEADERS KI18nLocaleData_HEADERS
)

install(FILES
    ${KI18nLocaleData_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/ki18nlocaledata_export.h
    DESTINATION "${KDE_INSTALL_INCLUDEDIR_KF}/KI18nLocaleData" COMPONENT Devel
)

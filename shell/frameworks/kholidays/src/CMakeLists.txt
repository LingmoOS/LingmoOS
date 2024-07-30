add_library(KF6Holidays)
add_library(KF6::Holidays ALIAS KF6Holidays)

set_target_properties(KF6Holidays PROPERTIES
  VERSION     ${KHOLIDAYS_VERSION}
  SOVERSION   ${KHOLIDAYS_SOVERSION}
  EXPORT_NAME Holidays
)

target_sources(KF6Holidays PRIVATE
    astroseasons.cpp
    astroseasons.h
    holiday.cpp
    holiday.h
    holiday_p.h
    holidayregion.cpp
    holidayregion.h
    lunarphase.cpp
    lunarphase.h
    parsers/holidayparserdriver.cpp
    parsers/holidayparserdriver_p.h
    parsers/plan2/FlexLexer.h
    parsers/plan2/holidayparserdriverplan.cpp
    parsers/plan2/holidayparserdriverplan_p.h
    parsers/plan2/holidayparserplan.cpp
    parsers/plan2/holidayscannerplan.cpp
    parsers/plan2/holidayscannerplan_p.h
    parsers/qcalendarsystem.cpp
    parsers/qcalendarsystem_p.h
    sunriseset.cpp
    sunriseset.h
    zodiac.cpp
    zodiac.h

  ../holidays/holidays.qrc
)

ecm_create_qm_loader(KF6Holidays libkholidays6_qt)

ecm_qt_declare_logging_category(KF6Holidays
    HEADER kholidays_debug.h
    IDENTIFIER KHOLIDAYS_LOG
    CATEGORY_NAME kf.holidays
    DESCRIPTION "KHolidays"
    EXPORT KHOLIDAYS
)

ecm_generate_export_header(KF6Holidays 
    VERSION  ${KF_VERSION}
    USE_VERSION_HEADER
    BASE_NAME KHolidays
    GROUP_BASE_NAME KF
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Holidays INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KHolidays>")
target_include_directories(KF6Holidays PUBLIC "$<BUILD_INTERFACE:${KHolidays_SOURCE_DIR}/src;${KHolidays_BINARY_DIR}/src>")
target_include_directories(KF6Holidays PUBLIC "$<BUILD_INTERFACE:${KHolidays_SOURCE_DIR}/src/parsers/plan2>")

target_link_libraries(KF6Holidays PUBLIC Qt6::Core)

ecm_generate_headers(KHolidays_CamelCase_HEADERS
  HEADER_NAMES
  AstroSeasons
  Holiday
  HolidayRegion
  LunarPhase
  SunRiseSet
  Zodiac
  PREFIX KHolidays
  REQUIRED_HEADERS KHolidays_HEADERS
)

install(TARGETS KF6Holidays EXPORT KF6HolidaysTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kholidays_export.h
  ${KHolidays_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KHolidays/kholidays
  COMPONENT Devel
)

install(FILES
  ${KHolidays_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KHolidays/KHolidays
  COMPONENT Devel
)

if(BUILD_QCH)
  ecm_add_qch(
    KF6Holidays_QCH
    NAME KHolidays
    BASE_NAME KF6Holidays
    VERSION ${KF_VERSION}
    ORG_DOMAIN org.kde
    SOURCES # using only public headers, to cover only public API
      ${KHolidays_HEADERS}
    MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
    LINK_QCHS
      Qt6Core_QCH
    INCLUDE_DIRS
      ${CMAKE_CURRENT_BINARY_DIR}
    BLANK_MACROS
      KHOLIDAYS_EXPORT
    TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
    QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
    COMPONENT Devel
  )
endif()

add_subdirectory(declarative)

ecm_qt_install_logging_categories(
    EXPORT KHOLIDAYS
    FILE kholidays.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

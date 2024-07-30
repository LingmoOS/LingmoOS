add_library(KF6CalendarCore)
add_library(KF6::CalendarCore ALIAS KF6CalendarCore)

qt_extract_metatypes(KF6CalendarCore)

set_target_properties(KF6CalendarCore PROPERTIES
    VERSION     ${KCALENDARCORE_VERSION}
    SOVERSION   ${KCALENDARCORE_SOVERSION}
    EXPORT_NAME CalendarCore
)

target_sources(KF6CalendarCore PRIVATE
    alarm.cpp
    alarm.h
    attachment.cpp
    attachment.h
    attendee.cpp
    attendee.h
    calendar.cpp
    calendar.h
    calendar_p.h
    calendarlistmodel.cpp
    calendarplugin.cpp
    calendarplugin.h
    calendarpluginloader.cpp
    calendarpluginloader.h
    calfilter.cpp
    calfilter.h
    calformat.cpp
    calformat.h
    calstorage.cpp
    calstorage.h
    compat.cpp
    compat_p.h
    conference.cpp
    conference.h
    customproperties.cpp
    customproperties.h
    duration.cpp
    duration.h
    event.cpp
    event.h
    exceptions.cpp
    exceptions.h
    filestorage.cpp
    filestorage.h
    freebusycache.cpp
    freebusycache.h
    freebusy.cpp
    freebusy.h
    freebusyperiod.cpp
    freebusyperiod.h
    icalformat.cpp
    icalformat.h
    icalformat_p.cpp
    icalformat_p.h
    icaltimezones.cpp
    icaltimezones_p.h
    incidencebase.cpp
    incidencebase.h
    incidencebase_p.h
    incidence.cpp
    incidence.h
    incidence_p.h
    journal.cpp
    journal.h
    memorycalendar.cpp
    memorycalendar.h
    occurrenceiterator.cpp
    occurrenceiterator.h
    period.cpp
    period.h
    person.cpp
    person.h
    person_p.h
    recurrence.cpp
    recurrence.h
    recurrencehelper_p.h
    recurrencerule.cpp
    recurrencerule.h
    schedulemessage.cpp
    schedulemessage.h
    sorting.cpp
    sorting.h
    todo.cpp
    todo.h
    utils.cpp
    utils_p.h
    vcalformat.cpp
    vcalformat.h
    visitor.cpp
    visitor.h

)
ecm_qt_declare_logging_category(KF6CalendarCore
    HEADER kcalendarcore_debug.h
    IDENTIFIER KCALCORE_LOG
    CATEGORY_NAME kf.calendarcore
    OLD_CATEGORY_NAMES log_kcalcore org.kde.pim.kcalcore
    DESCRIPTION "KCalendarCore"
    EXPORT KCALENDARCORE
)

ecm_generate_export_header(KF6CalendarCore
    BASE_NAME kcalendarcore
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6CalendarCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCalendarCore>")
target_include_directories(KF6CalendarCore PUBLIC "$<BUILD_INTERFACE:${KCalendarCore_SOURCE_DIR}/src;${KCalendarCore_BINARY_DIR}/src>")

target_link_libraries(KF6CalendarCore
PUBLIC
    Qt6::Core
    Qt6::Gui
PRIVATE
    LibIcal
)

install(TARGETS KF6CalendarCore EXPORT KF6CalendarCoreTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
set(kcalendarcore_headers
  Alarm
  Attachment
  Attendee
  CalFilter
  CalFormat
  CalStorage
  Calendar
  CalendarListModel
  CalendarPlugin
  CalendarPluginLoader
  Conference
  CustomProperties
  Duration
  Event
  Exceptions
  FileStorage
  FreeBusy
  FreeBusyCache
  FreeBusyPeriod
  ICalFormat
  Incidence
  IncidenceBase
  Journal
  MemoryCalendar
  OccurrenceIterator
  Period
  Person
  Recurrence
  RecurrenceRule
  ScheduleMessage
  Sorting
  Todo
  VCalFormat
  Visitor
)

ecm_generate_headers(KCalendarCore_CamelCase_HEADERS
  HEADER_NAMES
    ${kcalendarcore_headers}
  PREFIX KCalendarCore
  REQUIRED_HEADERS KCalendarCore_HEADERS
)

add_subdirectory(qml)

########### install files ###############

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kcalendarcore_export.h
  ${KCalendarCore_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCalendarCore/kcalendarcore
  COMPONENT Devel
)

install(FILES
  ${KCalendarCore_CamelCase_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCalendarCore/KCalendarCore
  COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KCALENDARCORE
    FILE kcalendarcore.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6CalendarCore_QCH
        NAME KCalendarCore
        BASE_NAME KF6CalendarCore
        VERSION ${KCalendarCore_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCalendarCore_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KCALENDARCORE_EXPORT
            "KCALENDARCORE_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

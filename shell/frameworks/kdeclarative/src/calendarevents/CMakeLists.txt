set(calendar-integration_SRCS
    calendareventsplugin.cpp
    calendareventsplugin.h
    eventdata_p.cpp
)

add_library(KF6CalendarEvents ${calendar-integration_SRCS})
add_library(KF6::CalendarEvents ALIAS KF6CalendarEvents)

set_target_properties(KF6CalendarEvents PROPERTIES
    VERSION     ${KDECLARATIVE_VERSION}
    SOVERSION   ${KDECLARATIVE_SOVERSION}
    EXPORT_NAME CalendarEvents
)

ecm_generate_export_header(KF6CalendarEvents
    BASE_NAME CalendarEvents
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KDeclarative
    DEPRECATED_BASE_VERSION 0
)

target_link_libraries(KF6CalendarEvents PUBLIC Qt6::Core)

target_include_directories(KF6CalendarEvents
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KDeclarative>"
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>" # module version header
)

install(TARGETS KF6CalendarEvents EXPORT KF6DeclarativeTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(CalendarEvents_CamelCase_HEADERS
  HEADER_NAMES
  CalendarEventsPlugin

  PREFIX CalendarEvents
  REQUIRED_HEADERS CalendarEvents_HEADERS
)
install(FILES ${CalendarEvents_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDeclarative/CalendarEvents COMPONENT Devel)

install (FILES
        ${CalendarEvents_HEADERS}
	${CMAKE_CURRENT_BINARY_DIR}/calendarevents_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDeclarative/calendarevents COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(CalendarEvents_QCH_SOURCES ${CalendarEvents_HEADERS} PARENT_SCOPE)
set(CalendarEvents_QCH_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

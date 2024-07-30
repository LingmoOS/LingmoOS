add_library(KF6PeopleBackend)
add_library(KF6::PeopleBackend ALIAS KF6PeopleBackend)

set_target_properties(KF6PeopleBackend PROPERTIES
    VERSION     ${KPEOPLE_VERSION}
    SOVERSION   ${KPEOPLE_SOVERSION}
    EXPORT_NAME PeopleBackend
)

target_sources(KF6PeopleBackend PRIVATE
    contactmonitor.cpp
    abstractcontact.cpp
    basepersonsdatasource.cpp
    allcontactsmonitor.cpp
    defaultcontactmonitor.cpp
    abstractpersonaction.cpp
    abstracteditablecontact.cpp
)

ecm_generate_export_header(KF6PeopleBackend
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/kpeoplebackend/kpeoplebackend_export.h
    BASE_NAME KPeopleBackend
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KPeople
    DEPRECATED_BASE_VERSION 0
)

target_link_libraries (KF6PeopleBackend
                    PUBLIC
                        Qt6::Gui
)

target_include_directories (KF6PeopleBackend PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
                                    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KPeople>"
)

add_subdirectory(vcard)

install (TARGETS KF6PeopleBackend
         EXPORT KPeopleTargets
         ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(KPeopleBackend_CamelCase_HEADERS
    HEADER_NAMES
        AbstractContact
        AbstractEditableContact
        AbstractPersonAction
        AllContactsMonitor
        BasePersonsDataSource
        ContactMonitor
    REQUIRED_HEADERS KPeopleBackend_HEADERS
    PREFIX KPeopleBackend
)
install (FILES ${KPeopleBackend_CamelCase_HEADERS}
         DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/KPeopleBackend
         COMPONENT Devel
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kpeoplebackend/kpeoplebackend_export.h
  ${KPeopleBackend_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/kpeoplebackend COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KPeopleBackend_QCH_SOURCES ${KPeopleBackend_HEADERS} PARENT_SCOPE)

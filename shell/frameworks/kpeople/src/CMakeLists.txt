include_directories (${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(declarative)
add_subdirectory(widgets)
add_subdirectory(backends)

add_library(KF6People)
add_library(KF6::People ALIAS KF6People)

set_target_properties (KF6People PROPERTIES
    VERSION     ${KPEOPLE_VERSION}
    SOVERSION   ${KPEOPLE_SOVERSION}
    EXPORT_NAME People
)

qt_extract_metatypes(KF6People)

ecm_generate_export_header(KF6People
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/kpeople/kpeople_export.h
    BASE_NAME KPeople
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
)

ecm_qt_declare_logging_category(KF6People
    HEADER kpeople_debug.h
    IDENTIFIER KPEOPLE_LOG
    CATEGORY_NAME kf.people.core
    OLD_CATEGORY_NAMES kf5.kpeople
    DESCRIPTION "kpeople lib"
    EXPORT KPEOPLE
)

target_sources(KF6People PRIVATE
    actions.cpp
    global.cpp
    metacontact.cpp
    persondata.cpp
    matchessolver.cpp
    match.cpp
    duplicatesfinder.cpp
    personsmodel.cpp
    personpluginmanager.cpp
    personmanager.cpp
    personssortfilterproxymodel.cpp
    resources.qrc
)

target_link_libraries (KF6People
                    PUBLIC
                        Qt6::Gui
                    PRIVATE
                        Qt6::Sql
                        KF6::I18n
                        KF6::CoreAddons
                        KF6::PeopleBackend
)

if(HAVE_QTDBUS)
    target_link_libraries(KF6People PRIVATE Qt6::DBus)
    target_compile_definitions(KF6People PRIVATE -DHAVE_QTDBUS=1)
else()
    target_compile_definitions(KF6People PRIVATE -DHAVE_QTDBUS=0)
endif()

target_include_directories (KF6People PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/kpeople>"
                                    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KPeople>"
)

if (MSVC)
    set_target_properties (KF6People PROPERTIES
                           OUTPUT_NAME libkpeople
    )
endif (MSVC)

# Install:
install (TARGETS KF6People
         EXPORT KPeopleTargets
         ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(KPeople_CamelCase_HEADERS
    HEADER_NAMES
        Actions
        PersonData
        PersonPluginManager
        PersonsModel
        PersonsSortFilterProxyModel
        Global
    REQUIRED_HEADERS KPeople_HEADERS
    PREFIX KPeople
)
install (FILES ${KPeople_CamelCase_HEADERS}
         DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/KPeople
         COMPONENT Devel
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kpeople/kpeople_export.h
  ${KPeople_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/kpeople COMPONENT Devel
)

install(FILES personmanager_p.h DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/kpeopleprivate)

if(BUILD_QCH)
    ecm_add_qch(
        KF6People_QCH
        NAME KPeople
        BASE_NAME KF6People
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KPeople_HEADERS}
            ${KPeopleWidgets_QCH_SOURCES}
            ${KPeopleBackend_QCH_SOURCES}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Gui_QCH
            Qt6Widgets_QCH
        BLANK_MACROS
            KPEOPLE_EXPORT
            KPEOPLE_DEPRECATED
            KPEOPLE_DEPRECATED_EXPORT
            KPEOPLEWIDGETS_EXPORT
            KPEOPLEWIDGETS_DEPRECATED
            KPEOPLEWIDGETS_DEPRECATED_EXPORT
            KPEOPLEBACKEND_EXPORT
            KPEOPLEBACKEND_DEPRECATED
            KPEOPLEBACKEND_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

ecm_qt_install_logging_categories(
    EXPORT KPEOPLE
    FILE kpeople.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (ANDROID)
    install(FILES KF6People-android-dependencies.xml DESTINATION ${KDE_INSTALL_LIBDIR})
endif()

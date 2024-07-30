add_library(KF6PeopleWidgets)
add_library(KF6::PeopleWidgets ALIAS KF6PeopleWidgets)

set_target_properties(KF6PeopleWidgets PROPERTIES
    VERSION     ${KPEOPLE_VERSION}
    SOVERSION   ${KPEOPLE_SOVERSION}
    EXPORT_NAME PeopleWidgets
)

target_sources(KF6PeopleWidgets PRIVATE
    persondetailsdialog.cpp
    persondetailsview.cpp
    persondetailsdialog.cpp
    emaildetailswidget.cpp
    abstractfieldwidgetfactory.cpp

    mergedialog.cpp
    mergedelegate.cpp
)

ecm_generate_export_header(KF6PeopleWidgets
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/kpeople/widgets/kpeoplewidgets_export.h
    BASE_NAME KPeopleWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KPeople
    DEPRECATED_BASE_VERSION 0
)

ecm_qt_declare_logging_category(KF6PeopleWidgets
    HEADER kpeople_widgets_debug.h
    IDENTIFIER KPEOPLE_WIDGETS_LOG
    CATEGORY_NAME kf.people.widgets
    OLD_CATEGORY_NAMES kf5.kpeople.widgets
    DESCRIPTION "kpeople widgets"
    EXPORT KPEOPLE
)

ki18n_wrap_ui(KF6PeopleWidgets person-details-presentation.ui)

target_link_libraries (KF6PeopleWidgets
                    PUBLIC
                        KF6::People
                        Qt6::Widgets
                    PRIVATE
                        KF6::I18n
                        KF6::ItemViews
                        KF6::WidgetsAddons
                        KF6::PeopleBackend
                        KF6::CoreAddons
)

if (MSVC)
    set_target_properties (KF6PeopleWidgets PROPERTIES
                           OUTPUT_NAME libKPeopleWidgets
    )
endif (MSVC)

# Install:
install (TARGETS KF6PeopleWidgets
         EXPORT KPeopleTargets
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

ecm_generate_headers(KPeopleWidgets_CamelCase_HEADERS
    HEADER_NAMES
            AbstractFieldWidgetFactory
            PersonDetailsView
            MergeDialog
            PersonDetailsDialog
    REQUIRED_HEADERS KPeopleWidgets_HEADERS
    PREFIX KPeople/Widgets
)

install (FILES ${KPeopleWidgets_CamelCase_HEADERS}
         DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/KPeople/Widgets
         COMPONENT Devel
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kpeople/widgets/kpeoplewidgets_export.h
  ${KPeopleWidgets_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople/kpeople/widgets COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(KPeopleWidgets_QCH_SOURCES ${KPeopleWidgets_HEADERS} PARENT_SCOPE)

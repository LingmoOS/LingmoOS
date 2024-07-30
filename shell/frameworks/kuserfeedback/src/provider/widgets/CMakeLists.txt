set(userfeedback_widgets_srcs
    feedbackconfigdialog.cpp
    feedbackconfigwidget.cpp
    notificationpopup.cpp
    styleinfosource.cpp
    auditlogbrowserdialog.cpp

    feedbackconfigdialog.h
    feedbackconfigwidget.h
    notificationpopup.h
    styleinfosource.h
    auditlogbrowserdialog.h
)

add_library(KF6UserFeedbackWidgets ${userfeedback_widgets_srcs})
add_library(KF6::UserFeedbackWidgets ALIAS KF6UserFeedbackWidgets)

set_target_properties(KF6UserFeedbackWidgets PROPERTIES
    SOVERSION ${KUSERFEEDBACK_SOVERSION}
    VERSION ${KUSERFEEDBACK_VERSION}
    EXPORT_NAME UserFeedbackWidgets
)
generate_export_header(KF6UserFeedbackWidgets BASE_NAME KUserFeedbackWidgets)
target_link_libraries(KF6UserFeedbackWidgets PUBLIC Qt6::Widgets KF6UserFeedbackCore)
target_include_directories(KF6UserFeedbackWidgets PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KF6UserFeedbackWidgets INTERFACE
                           "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedback;${KDE_INSTALL_INCLUDEDIR}>"
                           "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackWidgets;${KDE_INSTALL_INCLUDEDIR}>"
                          )

ecm_generate_headers(KUserFeedbackWidgets_CamelCase_HEADERS
    HEADER_NAMES
        FeedbackConfigDialog
        FeedbackConfigWidget
        NotificationPopup
        StyleInfoSource
    PREFIX KUserFeedback
    REQUIRED_HEADERS KUserFeedbackWidgets_HEADERS
)

install(TARGETS KF6UserFeedbackWidgets EXPORT KF6UserFeedbackTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kuserfeedbackwidgets_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackWidgets
    COMPONENT Devel
)
install(FILES
    ${KUserFeedbackWidgets_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackWidgets/kuserfeedback
    COMPONENT Devel
)

install(FILES
    ${KUserFeedbackWidgets_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackWidgets/KUserFeedback
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KF6UserFeedbackWidgets
                      LIB_NAME KF6UserFeedbackWidgets
                      DEPS "KF6UserFeedbackCore widgets"
                      FILENAME_VAR PRI_FILENAME
)

install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

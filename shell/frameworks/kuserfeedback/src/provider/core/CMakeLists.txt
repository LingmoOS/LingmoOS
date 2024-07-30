ecm_create_qm_loader(userfeedback_core_QM_LOADER userfeedbackprovider6_qt)

ecm_qt_declare_logging_category(userfeedback_core_debug_srcs
                                HEADER logging_p.h
                                IDENTIFIER KUserFeedback::Log
                                CATEGORY_NAME org.kde.UserFeedback
                                DESCRIPTION "User Feedback Framework"
                                EXPORT KUSERFEEDBACK)

set(userfeedback_core_srcs
    abstractdatasource.cpp
    applicationversionsource.cpp
    compilerinfosource.cpp
    cpuinfosource.cpp
    feedbackconfiguicontroller.cpp
    localeinfosource.cpp
    openglinfosource.cpp
    platforminfosource.cpp
    propertyratiosource.cpp
    provider.cpp
    qpainfosource.cpp
    qtversionsource.cpp
    screeninfosource.cpp
    selectionratiosource.cpp
    startcountsource.cpp
    surveyinfo.cpp
    usagetimesource.cpp
    auditloguicontroller.cpp

    abstractdatasource.h
    applicationversionsource.h
    compilerinfosource.h
    cpuinfosource.h
    feedbackconfiguicontroller.h
    localeinfosource.h
    openglinfosource.h
    platforminfosource.h
    propertyratiosource.h
    provider.h
    qpainfosource.h
    qtversionsource.h
    screeninfosource.h
    selectionratiosource.h
    startcountsource.h
    surveyinfo.h
    usagetimesource.h
    auditloguicontroller.h
    ${userfeedback_core_QM_LOADER}
    ${userfeedback_core_debug_srcs}
)



add_library(KF6UserFeedbackCore ${userfeedback_core_srcs})
add_library(KF6::UserFeedbackCore ALIAS KF6UserFeedbackCore)

set_target_properties(KF6UserFeedbackCore PROPERTIES
    SOVERSION ${KUSERFEEDBACK_SOVERSION}
    VERSION ${KUSERFEEDBACK_VERSION}
    EXPORT_NAME UserFeedbackCore
)
generate_export_header(KF6UserFeedbackCore BASE_NAME KUserFeedbackCore)

target_link_libraries(KF6UserFeedbackCore PUBLIC Qt6::Core PRIVATE Qt6::Gui Qt6::Network)
if(ANDROID)
    target_link_libraries(KF6UserFeedbackCore PRIVATE GLESv2)
endif()
target_compile_definitions(KF6UserFeedbackCore PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x050300)
target_link_libraries(KF6UserFeedbackCore PRIVATE KF6UserFeedbackCommon)
target_include_directories(KF6UserFeedbackCore PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KF6UserFeedbackCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedback;${KDE_INSTALL_INCLUDEDIR}>"
                                                         "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackCore;${KDE_INSTALL_INCLUDEDIR}>"
                          )

ecm_generate_headers(KUserFeedbackCore_CamelCase_HEADERS
    HEADER_NAMES
        AbstractDataSource
        ApplicationVersionSource
        CompilerInfoSource
        CpuInfoSource
        LocaleInfoSource
        OpenGLInfoSource
        PlatformInfoSource
        PropertyRatioSource
        Provider
        QPAInfoSource
        QtVersionSource
        ScreenInfoSource
        StartCountSource
        SurveyInfo
        UsageTimeSource
        FeedbackConfigUiController
    PREFIX KUserFeedback
    REQUIRED_HEADERS KUserFeedbackCore_HEADERS
)

install(TARGETS KF6UserFeedbackCore EXPORT KF6UserFeedbackTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kuserfeedbackcore_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackCore
    COMPONENT Devel
)

install(FILES
  ${KUserFeedbackCore_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackCore/kuserfeedback
  COMPONENT Devel
)

install(FILES
    ${KUserFeedbackCore_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedbackCore/KUserFeedback/
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KF6UserFeedbackCore
                      LIB_NAME KF6UserFeedbackCore
                      DEPS "core network"
                      FILENAME_VAR PRI_FILENAME
)

install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

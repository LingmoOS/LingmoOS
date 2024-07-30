ecm_create_qm_loader(userfeedback_console_QM_LOADER userfeedbackconsole6_qt)

set(console_lib_srcs
    core/aggregation.cpp
    core/aggregationelement.cpp
    core/product.cpp
    core/sample.cpp
    core/schemaentry.cpp
    core/schemaentryelement.cpp
    core/schemaentrytemplates.cpp
    core/survey.cpp

    rest/restapi.cpp
    rest/restclient.cpp
    rest/serverinfo.cpp

    jobs/job.cpp
    jobs/handshakejob.cpp
    jobs/productexportjob.cpp
    jobs/productimportjob.cpp
    jobs/securityscanjob.cpp

    model/aggregateddatamodel.cpp
    model/aggregationeditormodel.cpp
    model/aggregationelementmodel.cpp
    model/aggregationelementeditmodel.cpp
    model/categoryaggregationmodel.cpp
    model/datamodel.cpp
    model/extrarowsproxymodel.cpp
    model/numericaggregationmodel.cpp
    model/productmodel.cpp
    model/ratiosetaggregationmodel.cpp
    model/rolemappingproxymodel.cpp
    model/schemamodel.cpp
    model/singlerowfilterproxymodel.cpp
    model/surveymodel.cpp
    model/timeaggregationmodel.cpp

    core/aggregation.h
    core/aggregationelement.h
    core/product.h
    core/sample.h
    core/schemaentry.h
    core/schemaentryelement.h
    core/schemaentrytemplates.h
    core/survey.h

    rest/restapi.h
    rest/restclient.h
    rest/serverinfo.h

    jobs/job.h
    jobs/handshakejob.h
    jobs/productexportjob.h
    jobs/productimportjob.h
    jobs/securityscanjob.h

    model/aggregateddatamodel.h
    model/aggregationeditormodel.h
    model/aggregationelementmodel.h
    model/aggregationelementeditmodel.h
    model/categoryaggregationmodel.h
    model/datamodel.h
    model/extrarowsproxymodel.h
    model/numericaggregationmodel.h
    model/productmodel.h
    model/ratiosetaggregationmodel.h
    model/rolemappingproxymodel.h
    model/schemamodel.h
    model/singlerowfilterproxymodel.h
    model/surveymodel.h
    model/timeaggregationmodel.h

    schematemplates/schematemplates.qrc
)

add_library(KF6UserFeedbackConsole STATIC ${console_lib_srcs})
add_library(KF6::UserFeedbackConsole ALIAS KF6UserFeedbackConsole)

target_link_libraries(KF6UserFeedbackConsole Qt6::Network)
target_include_directories(KF6UserFeedbackConsole PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_compile_features(KF6UserFeedbackConsole PRIVATE cxx_generic_lambdas)
target_compile_definitions(KF6UserFeedbackConsole PUBLIC QT_DISABLE_DEPRECATED_BEFORE=0x050600 QT_DEPRECATED_WARNINGS)

if(NOT TARGET KF6UserFeedbackWidgets)
    return()
endif()

set(console_srcs
    connectdialog.cpp
    main.cpp
    mainwindow.cpp
    helpcontroller.cpp

    analytics/aggregator.cpp
    analytics/analyticsview.cpp
    analytics/categoryaggregator.cpp
    analytics/chartexportdialog.cpp
    analytics/chartutil.cpp
    analytics/numericaggregator.cpp
    analytics/ratiosetaggregator.cpp
    analytics/totalaggregator.cpp

    schemaeditor/aggregationeditwidget.cpp
    schemaeditor/schemaeditor.cpp
    schemaeditor/schemaeditwidget.cpp
    schemaeditor/schemaentryitemeditorfactory.cpp

    surveyeditor/surveydialog.cpp
    surveyeditor/surveyeditor.cpp

    widgets/metaenumcombobox.cpp

    connectdialog.h
    mainwindow.h
    helpcontroller.h

    analytics/aggregator.h
    analytics/analyticsview.h
    analytics/categoryaggregator.h
    analytics/chartexportdialog.h
    analytics/chartutil.h
    analytics/numericaggregator.h
    analytics/ratiosetaggregator.h
    analytics/totalaggregator.h

    schemaeditor/aggregationeditwidget.h
    schemaeditor/schemaeditor.h
    schemaeditor/schemaeditwidget.h
    schemaeditor/schemaentryitemeditorfactory.h

    surveyeditor/surveydialog.h
    surveyeditor/surveyeditor.h

    widgets/metaenumcombobox.h

    ${userfeedback_console_QM_LOADER}
)

add_executable(KUserFeedbackConsoleApplication ${console_srcs})
target_compile_features(KUserFeedbackConsoleApplication PRIVATE cxx_generic_lambdas)
set_target_properties(KUserFeedbackConsoleApplication PROPERTIES OUTPUT_NAME UserFeedbackConsole)
target_link_libraries(KUserFeedbackConsoleApplication
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    Qt6::PrintSupport
    Qt6::Svg
    KF6UserFeedbackWidgets
    KF6UserFeedbackConsole
    KF6UserFeedbackCommon
)

install(TARGETS KUserFeedbackConsoleApplication ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
if(UNIX AND NOT APPLE)
    install(FILES org.kde.kuserfeedback-console.desktop DESTINATION ${KDE_INSTALL_APPDIR})
    install(FILES org.kde.kuserfeedback-console.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_reviewboard\")
add_subdirectory(icons)

ecm_qt_declare_logging_category(ReviewboardHelpers_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_REVIEWBOARD
    CATEGORY_NAME kf.purpose.plugins.reviewboard
    OLD_CATEGORY_NAMES kdevplatform.plugins.reviewboard
    DESCRIPTION "purpose (reviewboard plugin)"
    EXPORT PURPOSE
)

add_library(ReviewboardHelpers STATIC reviewboardjobs.cpp ${ReviewboardHelpers_SRCS})
set_target_properties(ReviewboardHelpers PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(ReviewboardHelpers KF6::CoreAddons KF6::I18n Qt6::Network)

add_share_plugin(reviewboardplugin reviewboardplugin.cpp)
target_link_libraries(reviewboardplugin Qt6::Widgets ReviewboardHelpers)

set_target_properties(ReviewboardHelpers PROPERTIES VERSION ${PURPOSE_VERSION} SOVERSION ${PURPOSE_SOVERSION})

add_library(rbpurposequickplugin quick/reviewboardquickplugin.cpp quick/rbrepositoriesmodel.cpp quick/rbreviewslistmodel.cpp quick/reviewboardrc.cpp)
target_link_libraries(rbpurposequickplugin Qt6::Qml ReviewboardHelpers)

install(TARGETS rbpurposequickplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/purpose/reviewboard)
install(FILES quick/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/purpose/reviewboard)

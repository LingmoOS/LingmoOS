add_definitions(-DTRANSLATION_DOMAIN="purpose6_fileitemaction")
include_directories(${CMAKE_SOURCE_DIR})

set(sharefileitemaction_debug_SRCS)
ecm_qt_declare_logging_category(sharefileitemaction_debug_SRCS
    HEADER purpose_fileitemaction_plugin_debug.h
    IDENTIFIER PURPOSE_FILEITEMACTION_PLUGIN_LOG
    CATEGORY_NAME kf.kio.widgets.fileitemactions.purpose
    OLD_CATEGORY_NAMES kf5.purpose.fileitemactionplugin
    DESCRIPTION "purpose fileitemaction plugin"
    EXPORT PURPOSE
)

kcoreaddons_add_plugin(sharefileitemaction
    SOURCES sharefileitemaction.cpp ${sharefileitemaction_debug_SRCS}
    INSTALL_NAMESPACE "kf6/kfileitemaction")

target_link_libraries(sharefileitemaction
    KF6::KIOWidgets
    KF6::I18n
    KF6::Notifications
    KF6PurposeWidgets
)

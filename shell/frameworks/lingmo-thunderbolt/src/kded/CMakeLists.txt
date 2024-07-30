add_definitions(-DTRANSLATION_DOMAIN=\"kded_bolt\")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)



add_library(kded_bolt MODULE)
target_sources(kded_bolt PRIVATE
    main.cpp
    kded_bolt.cpp
    kded_bolt.h
)
ecm_qt_declare_logging_category(kded_bolt
    HEADER kded_bolt_debug.h
    IDENTIFIER log_kded_bolt
    CATEGORY_NAME org.kde.bolt.kded
)
set_target_properties(kded_bolt PROPERTIES CXX_STANDARD 14)
target_link_libraries(kded_bolt
    KF6::DBusAddons
    KF6::I18n
    KF6::Notifications
    KF6::CoreAddons
    kbolt
)

install(TARGETS kded_bolt DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)
install(FILES kded_bolt.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

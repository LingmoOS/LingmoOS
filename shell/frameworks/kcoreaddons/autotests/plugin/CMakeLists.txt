find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Widgets)
if (NOT TARGET Qt6::Widgets)
    return()
endif()

add_library(plugin_classes plugins.cpp)
generate_export_header(plugin_classes)

kcoreaddons_add_plugin(jsonplugin_cmake_macro SOURCES kpluginclass.cpp INSTALL_NAMESPACE "namespace")
ecm_mark_as_test(jsonplugin_cmake_macro)
target_link_libraries(jsonplugin_cmake_macro KF6::CoreAddons autotests_static plugin_classes)
target_link_libraries(plugin_classes Qt6::Core)

kcoreaddons_add_plugin(qtplugin SOURCES qtplugin.cpp INSTALL_NAMESPACE "namespace")
ecm_mark_as_test(qtplugin)
target_link_libraries(qtplugin KF6::CoreAddons autotests_static)

kcoreaddons_add_plugin(pluginwithoutmetadata SOURCES pluginwithoutmetadata.cpp INSTALL_NAMESPACE "namespace")
ecm_mark_as_test(pluginwithoutmetadata)
target_link_libraries(pluginwithoutmetadata KF6::CoreAddons autotests_static)

kcoreaddons_add_plugin(widgetsplugin SOURCES widgetsplugin.cpp INSTALL_NAMESPACE "widgets")
ecm_mark_as_test(widgetsplugin)
target_link_libraries(widgetsplugin KF6::CoreAddons Qt6::Widgets)

add_library(org.kde.test MODULE qtplugin.cpp)
target_link_libraries(org.kde.test KF6::CoreAddons)


ecm_add_tests(
    kpluginmetadatatest.cpp
    kpluginfactorytest.cpp
    ${autotests_OPTIONAL_SRCS}
    LINK_LIBRARIES Qt6::Test Qt6::Widgets KF6::CoreAddons autotests_static
)
if (NOT WIN32)
    target_link_libraries(kpluginfactorytest plugin_classes)
endif()

kcoreaddons_add_plugin(static_jsonplugin_cmake_macro SOURCES statickpluginclass.cpp INSTALL_NAMESPACE "staticnamespace" STATIC)
target_link_libraries(static_jsonplugin_cmake_macro KF6::CoreAddons autotests_static)

kcoreaddons_add_plugin(static_jsonplugin_cmake_macro_2 SOURCES statickpluginclass_2.cpp INSTALL_NAMESPACE "staticnamespace2" STATIC)
target_link_libraries(static_jsonplugin_cmake_macro_2 KF6::CoreAddons autotests_static)

kcoreaddons_add_plugin(static_plugin_without_metadata SOURCES staticpluginwithoutmetadata.cpp INSTALL_NAMESPACE "staticnamespace3" STATIC)
target_link_libraries(static_plugin_without_metadata KF6::CoreAddons autotests_static)

kcoreaddons_add_plugin(org.kde.test-staticplugin SOURCES statickpluginclass_3.cpp INSTALL_NAMESPACE "rdnstatic" STATIC)
target_link_libraries(org.kde.test-staticplugin KF6::CoreAddons autotests_static)

kcoreaddons_target_static_plugins(kpluginfactorytest NAMESPACE "staticnamespace")
kcoreaddons_target_static_plugins(kpluginfactorytest NAMESPACE "staticnamespace2")
kcoreaddons_target_static_plugins(kpluginmetadatatest NAMESPACE "staticnamespace")
kcoreaddons_target_static_plugins(kpluginmetadatatest NAMESPACE "staticnamespace2")
kcoreaddons_target_static_plugins(kpluginmetadatatest NAMESPACE "rdnstatic")
# Make the last one use the target by targets list instead of namespace
kcoreaddons_target_static_plugins(kpluginmetadatatest TARGETS static_plugin_without_metadata)

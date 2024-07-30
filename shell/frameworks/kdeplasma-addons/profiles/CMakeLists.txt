add_library(profiles_utility_static STATIC profilesmodel.cpp)
set_property(TARGET profiles_utility_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(profiles_utility_static PRIVATE -DTRANSLATION_DOMAIN="plasma_addons_profiles_utility")
target_link_libraries(profiles_utility_static PRIVATE Qt::Core KF6::I18n KF6::CoreAddons KF6::ConfigCore KF6::KIOGui)
target_include_directories(profiles_utility_static
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

ecm_add_qml_module(profiles_qml_plugin URI org.kde.plasma.private.profiles)
target_sources(profiles_qml_plugin PRIVATE profilesplugin.cpp)
target_link_libraries(profiles_qml_plugin PRIVATE
    Qt::Qml
    profiles_utility_static
)
ecm_finalize_qml_module(profiles_qml_plugin)

qt_add_resources(sharetool_SRCS sharetool-resources.qrc)

add_executable(sharetool main.cpp ${sharetool_SRCS})
target_link_libraries(sharetool KF6::Purpose KF6::I18n KF6::CoreAddons Qt6::Qml Qt6::Widgets)

target_compile_definitions(sharetool PRIVATE TRANSLATION_DOMAIN="purpose6_sharetool")

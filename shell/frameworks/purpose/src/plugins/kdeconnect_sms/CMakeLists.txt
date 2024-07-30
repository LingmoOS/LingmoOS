add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_kdeconnectsms\")
add_share_plugin(kdeconnectsmsplugin kdeconnectsmsplugin.cpp)
target_link_libraries(kdeconnectsmsplugin KF6::I18n KF6::KIOGui)


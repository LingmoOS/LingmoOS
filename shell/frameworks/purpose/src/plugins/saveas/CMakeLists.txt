add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_saveas\")
add_share_plugin(saveasplugin saveasplugin.cpp)
target_link_libraries(saveasplugin KF6::KIOCore KF6::I18n)

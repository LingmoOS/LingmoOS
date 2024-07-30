add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_pastebin\")
add_share_plugin(pastebinplugin pastebinplugin.cpp)
target_link_libraries(pastebinplugin KF6::KIOCore KF6::I18n)


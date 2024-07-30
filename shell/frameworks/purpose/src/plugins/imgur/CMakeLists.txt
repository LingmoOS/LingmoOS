add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_imgur\")
add_share_plugin(imgurplugin imgurplugin.cpp mpform.cpp)
target_link_libraries(imgurplugin KF6::KIOCore KF6::I18n KF6::Notifications)


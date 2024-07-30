add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_youtube\")

kaccounts_add_service(${CMAKE_CURRENT_SOURCE_DIR}/google-youtube.service.in)

add_share_plugin(youtubeplugin youtubeplugin.cpp youtubejob.cpp youtubejobcomposite.cpp)
target_link_libraries(youtubeplugin KF6::WidgetsAddons KF6::KIOCore Qt6::Network KF6::I18n KF6::Purpose KAccounts6)


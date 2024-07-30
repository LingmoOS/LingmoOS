add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_twitter\")
add_share_plugin(twitterplugin twitterplugin.cpp)
target_link_libraries(twitterplugin KF6::KIOGui)


add_share_plugin(telegramplugin telegramplugin.cpp)
target_link_libraries(telegramplugin KF6::ConfigCore KF6::I18n KF6::KIOGui KF6::Service)

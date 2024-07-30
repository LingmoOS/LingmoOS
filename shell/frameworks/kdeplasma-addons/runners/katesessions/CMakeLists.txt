add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_katesessions\")

kcoreaddons_add_plugin(krunner_katesessions SOURCES katesessions.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_katesessions KF6::KIOGui KF6::JobWidgets KF6::I18n KF6::Runner profiles_utility_static)

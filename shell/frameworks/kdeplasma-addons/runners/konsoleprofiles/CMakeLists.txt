add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_konsoleprofiles")

kcoreaddons_add_plugin(krunner_konsoleprofiles SOURCES konsoleprofiles.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_konsoleprofiles
    KF6::Runner
    KF6::ConfigCore
    KF6::I18n
    profiles_utility_static
)

add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_CharacterRunner")

kcoreaddons_add_plugin(krunner_charrunner SOURCES charrunner.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_charrunner
    KF6::Runner
    KF6::I18n
    KF6::ConfigCore
    Qt::Gui
)

kcoreaddons_add_plugin(kcm_krunner_charrunner INSTALL_NAMESPACE "kf6/krunner/kcms")
target_compile_definitions(kcm_krunner_charrunner PRIVATE -DKRUNNER_PLUGIN_NAME="krunner_charrunner")
target_sources(kcm_krunner_charrunner PRIVATE charrunner_config.cpp)
ki18n_wrap_ui(kcm_krunner_charrunner charrunner_config.ui)
target_link_libraries(kcm_krunner_charrunner
    KF6::Runner
    KF6::KCMUtils
    KF6::I18n
)

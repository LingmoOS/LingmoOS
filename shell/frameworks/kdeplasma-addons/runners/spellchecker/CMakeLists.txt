add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_spellcheckrunner")

add_library(krunner_spellcheck_static STATIC spellcheck.cpp)
set_property(TARGET krunner_spellcheck_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(krunner_spellcheck_static
    KF6::Runner
    KF6::KIOWidgets
    KF6::I18n
    KF6::SonnetCore
)

kcoreaddons_add_plugin(krunner_spellcheck SOURCES spellcheck.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_spellcheck
    krunner_spellcheck_static
)

kcoreaddons_add_plugin(kcm_krunner_spellcheck INSTALL_NAMESPACE "kf6/krunner/kcms")
target_compile_definitions(kcm_krunner_spellcheck PRIVATE -DKRUNNER_PLUGIN_NAME="krunner_spellcheck")
ki18n_wrap_ui(kcm_krunner_spellcheck spellcheck_config.ui)
target_sources(kcm_krunner_spellcheck PRIVATE spellcheck_config.cpp)
target_link_libraries(kcm_krunner_spellcheck 
    Qt::Gui
    KF6::Runner
    KF6::KCMUtils
    KF6::I18n
)

if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()

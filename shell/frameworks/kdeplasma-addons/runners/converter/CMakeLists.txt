add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_converterrunner\")

kcoreaddons_add_plugin(unitconverter SOURCES converterrunner.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(unitconverter
    KF6::I18n
    KF6::Runner
    KF6::UnitConversion
    Qt::Widgets
)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_datetime")

kcoreaddons_add_plugin(org.kde.datetime SOURCES datetimerunner.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(org.kde.datetime
    KF6::Runner
    KF6::KIOWidgets
    KF6::I18n
)

if(BUILD_TESTING)
add_subdirectory(autotests)
endif()

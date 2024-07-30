add_definitions(-DTRANSLATION_DOMAIN="lingmo_runner_lingmo-desktop")

kcoreaddons_add_plugin(krunner_lingmo-desktop SOURCES lingmo-desktop-runner.cpp INSTALL_NAMESPACE kf6/krunner)
target_link_libraries(krunner_lingmo-desktop
                        Qt::DBus
                        KF6::KIOCore
                        KF6::I18n
                        KF6::KIOGui
                        KF6::Runner)

# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (LingmoActivitiesCLI)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

add_executable(lingmo-activities-cli6)
ecm_mark_nongui_executable(lingmo-activities-cli6)

target_include_directories(lingmo-activities-cli6 PRIVATE
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

target_sources(lingmo-activities-cli6 PRIVATE
   main.cpp
)

target_link_libraries(lingmo-activities-cli6
   Qt6::Core
   Lingmo::Activities
)

install (TARGETS
   lingmo-activities-cli6
   ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
)

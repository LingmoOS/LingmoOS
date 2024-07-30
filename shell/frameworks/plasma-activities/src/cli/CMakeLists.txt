# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (PlasmaActivitiesCLI)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

add_executable(plasma-activities-cli6)
ecm_mark_nongui_executable(plasma-activities-cli6)

target_include_directories(plasma-activities-cli6 PRIVATE
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

target_sources(plasma-activities-cli6 PRIVATE
   main.cpp
)

target_link_libraries(plasma-activities-cli6
   Qt6::Core
   Plasma::Activities
)

install (TARGETS
   plasma-activities-cli6
   ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
)

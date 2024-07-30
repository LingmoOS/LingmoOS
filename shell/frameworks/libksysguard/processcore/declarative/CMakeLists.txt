include_directories(${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

ecm_add_qml_module(ProcessPlugin URI "org.kde.ksysguard.process" VERSION 1.0)

target_sources(ProcessPlugin PRIVATE ProcessPlugin.cpp ProcessPlugin.h ProcessEnums.cpp ProcessEnums.h)

target_link_libraries(ProcessPlugin PRIVATE Qt::Qml KSysGuard::ProcessCore)

ecm_finalize_qml_module(ProcessPlugin)

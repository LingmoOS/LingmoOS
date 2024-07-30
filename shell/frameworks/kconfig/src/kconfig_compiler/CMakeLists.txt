add_executable(kconfig_compiler)

if(CMAKE_CROSSCOMPILING)
    if(BUILD_TESTING)
        message(WARNING "Testing should be disabled on cross-compilation")
    endif()
else()
    add_executable(KF6::kconfig_compiler ALIAS kconfig_compiler)
endif()

target_sources(kconfig_compiler PRIVATE
    KConfigParameters.cpp
    KConfigCodeGeneratorBase.cpp
    KConfigHeaderGenerator.cpp
    KConfigSourceGenerator.cpp
    KConfigXmlParser.cpp
    kconfig_compiler.cpp
)

set_target_properties(kconfig_compiler PROPERTIES
    OUTPUT_NAME "kconfig_compiler_kf6"
)

target_link_libraries(kconfig_compiler Qt6::Xml)

ecm_mark_nongui_executable(kconfig_compiler)

install(TARGETS kconfig_compiler EXPORT KF6ConfigCompilerTargets DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})

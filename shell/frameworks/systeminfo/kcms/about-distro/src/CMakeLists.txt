configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(dmidecode-helper)

set(kcm_SRCS
    CPUEntry.cpp
    CPUEntry.h
    Entry.cpp
    Entry.h
    FancyString.cpp
    FancyString.h
    GPUEntry.cpp
    GPUEntry.h
    GraphicsPlatformEntry.cpp
    GraphicsPlatformEntry.h
    KernelEntry.cpp
    KernelEntry.h
    main.cpp
    MemoryEntry.cpp
    MemoryEntry.h
    OSVersionEntry.cpp
    OSVersionEntry.h
    LingmoEntry.cpp
    LingmoEntry.h
    ServiceRunner.cpp
    ThirdPartyEntry.cpp
    ThirdPartyEntry.h
)

kcmutils_add_qml_kcm(kcm_about-distro SOURCES ${kcm_SRCS} INSTALL_NAMESPACE "lingmo/kcms")

target_link_libraries(kcm_about-distro  PRIVATE
    KF6::CoreAddons
    KF6::I18n
    KF6::ConfigCore
    KF6::KCMUtilsQuick
    KF6::Solid
    KF6::Service
    KF6::KIOGui
    KF6::AuthCore
)

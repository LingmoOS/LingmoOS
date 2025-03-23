set(BIN_NAME dtk-exhibition)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick QuickControls2)
find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR}Gui REQUIRED)

set(CMAKE_EXE_LINKER_FLAGS  "-z relro -z now -z noexecstack -pie -Wl,--as-needed")

if (EnableQt5)
    set(QML_RCS ${CMAKE_CURRENT_LIST_DIR}/qml.qrc)
endif()
if (EnableQt6)
    set(QML_RCS ${CMAKE_CURRENT_LIST_DIR}/qml-qt6.qrc)
endif()
add_executable(${BIN_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/assets.qrc
    ${QML_RCS}
)

target_compile_options(${BIN_NAME} PRIVATE "-fpic")

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Quick 
    Qt${QT_VERSION_MAJOR}::QuickControls2
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
)

set(EXAMPLE_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
if (EnableQt6)
    set(EXAMPLE_INSTALL_DIR "${LIB_INSTALL_DIR}/dtk${PROJECT_VERSION_MAJOR}/DDeclarative")
endif()

install(TARGETS ${BIN_NAME} DESTINATION ${EXAMPLE_INSTALL_DIR})

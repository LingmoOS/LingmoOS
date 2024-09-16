find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(BIN taskbar)
set(TARGET_NAME ${BIN}${DTK_VERSION_MAJOR})

add_executable(${TARGET_NAME}
    testtaskbarwindow.h 
    testtaskbarwindow.cpp
    main.cpp
)

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    ${LIB_NAME}
)
set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${BIN})

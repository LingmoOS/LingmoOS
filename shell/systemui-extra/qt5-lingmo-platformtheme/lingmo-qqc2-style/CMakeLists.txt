project(lingmo-qqc2-style)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Gui
)

file(GLOB QML_FILES "org.lingmo.style/*.qml")
file(GLOB PRIVATE_FILES "org.lingmo.style/private/*")

SOURCE_GROUP("QMLFiles" FILES ${QML_FILES})
SOURCE_GROUP("QMLPrivateFiles" FILES ${PRIVATE_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${QMLFiles} ${QMLPrivateFiles} )

if(UNIX)
#    file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/org.lingmo.style/*.qml)
    set(LIB_PATH ${imported_location})
    get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
    get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
    get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
    message("lingmo-qqc2-style PARENT_PATH: ${PARENT_PATH}")

    set(TARGET_PATH "${PARENT_PATH}/qml/QtQuick/Controls.2/org.lingmo.style/")
#    message("${TARGET_PATH}")
    set(TARGET_FILES ${QML_FILES})
    install(FILES ${TARGET_FILES} DESTINATION ${TARGET_PATH})
    install(FILES ${PRIVATE_FILES} DESTINATION "${TARGET_PATH}/private")
endif()

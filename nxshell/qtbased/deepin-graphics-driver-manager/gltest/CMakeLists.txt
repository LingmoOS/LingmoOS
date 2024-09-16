
if (POLICY CMP0072)
  set(OpenGL_GL_PREFERENCE LEGACY)
endif()

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(DtkWidget REQUIRED)


if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_BINARY_DIR}/translations/\"")
else()
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-graphics-driver-manager/translations/\"")
endif()


file(GLOB SRCS "*.h" "*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/utils/)

add_executable(gltest ${SRCS} ${UTILS_FILES})
target_link_libraries(gltest
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

target_include_directories(installer PUBLIC
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}

)
install(TARGETS gltest DESTINATION lib/deepin-graphics-driver-manager)
#install(FILES deepin-gradvrmgr-installer.desktop DESTINATION lib/deepin-graphics-driver-manager)

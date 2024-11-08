find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTORCC ON)

qt5_add_resources(VirtualKeyboard_SRC
	src/keyboard.qrc)

set(VirtualKeyboard_SRC
	${VirtualKeyboard_SRC}
	src/cursormonitor.cpp
	src/keyboardwidget.cpp
	src/virtualkeyboard.cpp
	src/x11keyboard.cpp
	src/keyboard.qrc)

include_directories(
	${Qt5Core_INCLUDE_DIRS}
	${Qt5Widgets_INCLUDE_DIRS}
	)

add_library(VirtualKeyboard STATIC ${VirtualKeyboard_SRC})
target_link_libraries(VirtualKeyboard Qt5::Core Qt5::Widgets)

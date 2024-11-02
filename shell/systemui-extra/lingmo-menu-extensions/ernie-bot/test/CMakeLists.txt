include_directories(${CMAKE_SOURCE_DIR}/src/)
set(ernieTest_SRCS
        ${CMAKE_SOURCE_DIR}/src/chat.cpp
        ${CMAKE_SOURCE_DIR}/src/chat.h
        main.cpp)

add_executable(ernie-test ${ernieTest_SRCS})

target_link_libraries(ernie-test PRIVATE
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Quick Qt5::Network
        ernie-bot
        )
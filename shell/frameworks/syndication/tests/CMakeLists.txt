set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)

find_package(Qt6Network ${REQUIRED_QT_VERSION} CONFIG REQUIRED) # for testloader

########### next target ###############

add_executable(testloader testloader.cpp)
target_link_libraries(testloader KF6Syndication Qt6::Core Qt6::Network)


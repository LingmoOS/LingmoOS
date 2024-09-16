project(httpweb)

FILE(GLOB CPP_SRC
    "${CMAKE_SOURCE_DIR}/src/configs/crypt/cert.h"
    "${CMAKE_SOURCE_DIR}/src/common/filesystem.h"
    "${CMAKE_SOURCE_DIR}/src/common/filesystem.cpp"
    *.h
    *.cpp
)

list(APPEND LINKLIBS cppserver)

# httpweb library
add_library(${PROJECT_NAME} ${CPP_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "httpweb")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} ${LINKLIBS})

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

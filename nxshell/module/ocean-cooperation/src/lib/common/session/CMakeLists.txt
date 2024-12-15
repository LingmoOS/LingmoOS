project(session)

FILE(GLOB CPP_SRC
    *.h
    *.cpp
)

list(APPEND LINKLIBS cppserver)
list(APPEND LINKLIBS proto)

# session library
add_library(${PROJECT_NAME} ${CPP_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "session")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC ${LINKLIBS})

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

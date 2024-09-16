include_directories(
    global
)

link_libraries(
    ${Qt_LIBS}
)

set(LIB_NAME login)

aux_source_directory(. SRCS)

# add_library(${LIB_NAME} STATIC ${SRCS})

add_library(${LIB_NAME} SHARED ${SRCS})

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib/dde-session-shell/modules)

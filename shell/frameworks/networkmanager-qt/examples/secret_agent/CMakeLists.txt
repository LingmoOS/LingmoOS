set(secretAgentExample_SRCS
    main.cpp
)

add_executable(secretAgentExample ${secretAgentExample_SRCS})
target_link_libraries(secretAgentExample
    KF6::NetworkManagerQt
    Qt6::Core
)

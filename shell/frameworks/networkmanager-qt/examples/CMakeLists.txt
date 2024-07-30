set(exampleNetworkManagerQt_SRCS
    main.cpp
)

add_executable(exampleNetworkManagerQt ${exampleNetworkManagerQt_SRCS})
target_link_libraries(exampleNetworkManagerQt
    KF6::NetworkManagerQt
    Qt6::Core
)

add_subdirectory(createconnection)
add_subdirectory(secret_agent)

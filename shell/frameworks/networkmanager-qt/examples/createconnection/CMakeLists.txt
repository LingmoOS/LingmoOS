set(createConnectionExample_SRCS
    main.cpp
)

add_executable(createConnectionExample ${createConnectionExample_SRCS})
target_link_libraries(createConnectionExample
    KF6::NetworkManagerQt
    Qt6::Core
)

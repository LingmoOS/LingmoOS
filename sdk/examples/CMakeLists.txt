add_executable(lingmo-sdk-demo
    demo/main.cpp
)

target_link_libraries(lingmo-sdk-demo PRIVATE lingmo-sdk)

add_executable(lingmo-sdk-gallery
    gallery/main.cpp
)

target_link_libraries(lingmo-sdk-gallery PRIVATE lingmo-sdk)

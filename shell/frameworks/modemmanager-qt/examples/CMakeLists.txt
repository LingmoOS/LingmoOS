set(exampleModemManagerQt_SRCS
   main.cpp
)

add_executable(exampleModemManagerQt ${exampleModemManagerQt_SRCS})
target_link_libraries(exampleModemManagerQt
    KF6::ModemManagerQt
    Qt6::Core
)

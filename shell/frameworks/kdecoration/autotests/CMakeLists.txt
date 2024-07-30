include(ECMMarkAsTest)

set(decorationButtonTest_SRCS
    mockbridge.cpp mockbridge.h
    mockbutton.cpp mockbutton.h
    mockclient.cpp mockclient.h
    mockdecoration.cpp mockdecoration.h
    mocksettings.cpp mocksettings.h
    decorationbuttontest.cpp
    )
add_executable(decorationButtonTest ${decorationButtonTest_SRCS})
target_link_libraries(decorationButtonTest kdecorations2 kdecorations2private Qt::Test)
add_test(NAME kdecoration2-decorationButtonTest COMMAND decorationButtonTest)
ecm_mark_as_test(decorationButtonTest)

set(decorationTest_SRCS
    mockbridge.cpp mockbridge.h
    mockbutton.cpp mockbutton.h
    mockclient.cpp mockclient.h
    mockdecoration.cpp mockdecoration.h
    mocksettings.cpp mocksettings.h
    decorationtest.cpp
    )
add_executable(decorationTest ${decorationTest_SRCS})
target_link_libraries(decorationTest kdecorations2 kdecorations2private Qt::Test)
add_test(NAME kdecoration2-decorationTest COMMAND decorationTest)
ecm_mark_as_test(decorationTest)

set(decorationShadowTest_SRCS
    shadowtest.cpp
    )
add_executable(decorationShadowTest ${decorationShadowTest_SRCS})
target_link_libraries(decorationShadowTest kdecorations2 Qt::Test)
add_test(NAME kdecoration2-decorationShadowTest COMMAND decorationShadowTest)
ecm_mark_as_test(decorationShadowTest)

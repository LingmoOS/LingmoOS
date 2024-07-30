add_library(KWinBridge STATIC
    dummydecoratedclient.cpp
    dummydecorationbridge.cpp
    dummydecorationsettings.cpp
    decorationpalette.cpp
    decorationpainter.cpp
    auroraedecorationpainter.cpp
    standarddecorationpainter.cpp
)
set_property(TARGET KWinBridge PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(KWinBridge
  PUBLIC
    KDecoration2::KDecoration
  PRIVATE
    KDecoration2::KDecoration2Private
    KF6::ColorScheme
    KF6::CoreAddons
    Qt::Svg
)

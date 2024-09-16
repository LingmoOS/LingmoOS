find_package(Qt5 COMPONENTS LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DdeControlCenter REQUIRED)

pkg_check_modules(xkbcommon REQUIRED IMPORTED_TARGET xkbcommon)

file(GLOB TS_FILES "translations/*.ts")
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations ALL DEPENDS ${QM_FILES})

add_library(dcc-fcitx5configtool-plugin SHARED
    fcitx5configplugin.cpp
    fcitx5configplugin.h
    addimpage.cpp
    addimpage.h
    imsettingwindow.cpp
    imsettingwindow.h
    imwindow.cpp
    imwindow.h
    layoutwidget.cpp
    layoutwidget.h
    configproxy.cpp
    configproxy.h

    widgets/utils.h
    widgets/contentwidget.cpp
    widgets/contentwidget.h
    widgets/keylabel.h
    widgets/keysettingsitem.cpp
    widgets/keysettingsitem.h

    widgets/labels/clicklabel.cpp
    widgets/labels/clicklabel.h
    widgets/labels/largelabel.h
    widgets/labels/normallabel.h
    widgets/labels/shortenlabel.cpp
    widgets/labels/shortenlabel.h
    widgets/labels/smalllabel.h
    widgets/labels/tipslabel.h

    res/dcc-module.qrc
)

set_target_properties(dcc-fcitx5configtool-plugin
    PROPERTIES
    AUTOMOC TRUE
    AUTORCC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h")

target_link_libraries(dcc-fcitx5configtool-plugin
    Dtk::Core
    Dtk::Widget
    Dde::DCCInterface
    Dde::DCCWidget
    PkgConfig::xkbcommon
    configwidgetslib
)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS dcc-fcitx5configtool-plugin LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dde-control-center/modules)

# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dde-control-center/translations)

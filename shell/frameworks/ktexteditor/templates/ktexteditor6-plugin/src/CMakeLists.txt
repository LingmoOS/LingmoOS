add_definitions(-DTRANSLATION_DOMAIN=\"%{APPNAMELC}\")

set(%{APPNAMELC}_SRCS
    %{APPNAMELC}plugin.cpp
    %{APPNAMELC}view.cpp
)

add_library(%{APPNAMELC} MODULE ${%{APPNAMELC}_SRCS})

target_link_libraries(%{APPNAMELC}
    KF6::TextEditor
    KF6::I18n
)

install(TARGETS %{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/ktexteditor)

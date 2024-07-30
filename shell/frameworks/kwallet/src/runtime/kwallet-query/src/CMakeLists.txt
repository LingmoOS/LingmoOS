remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(kwallet-query)

target_sources(kwallet-query PRIVATE
    main.cpp
    querydriver.cpp
)


TARGET_LINK_LIBRARIES(kwallet-query
    KF6Wallet
    KF6::CoreAddons
    KF6::I18n
    Qt6::Widgets
)

ecm_mark_nongui_executable(kwallet-query)
install( TARGETS kwallet-query DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

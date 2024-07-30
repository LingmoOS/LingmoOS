set(uf_cli_srcs
    main.cpp
)

add_executable(userfeedbackctl ${uf_cli_srcs})
target_compile_features(userfeedbackctl PRIVATE cxx_generic_lambdas)
target_link_libraries(userfeedbackctl KF6UserFeedbackConsole)
install(TARGETS userfeedbackctl ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

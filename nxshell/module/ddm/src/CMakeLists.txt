if (QT_KNOWN_POLICY_QTP0001)
    qt_policy(SET QTP0001 NEW)
endif()

add_subdirectory(common)
add_subdirectory(auth)
add_subdirectory(daemon)
add_subdirectory(helper)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_bolt\")

kcmutils_add_qml_kcm(kcm_bolt SOURCES kcm_bolt.cpp)
target_link_libraries(kcm_bolt PRIVATE
    KF6::KCMUtilsQuick
    KF6::I18n
    kbolt
)

add_library(engine_dict_static STATIC dictengine.cpp)
set_property(TARGET engine_dict_static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(engine_dict_static PRIVATE -DTRANSLATION_DOMAIN="plasma_addons_engine_dict")

target_link_libraries (engine_dict_static
    KF6::I18n
    Qt::Network
)


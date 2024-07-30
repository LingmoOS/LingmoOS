find_package(Qt6Test REQUIRED)

add_library(dummyplugin OBJECT dummy.cpp)
target_compile_definitions(dummyplugin PRIVATE QT_STATICPLUGIN)
target_link_libraries(dummyplugin K::KGlobalAccelD)

ecm_add_test(migrateconfigtest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD)
ecm_add_test(shortcutstest.cpp LINK_LIBRARIES Qt::Test KF6::ConfigCore KF6::Service KGlobalAccelD dummyplugin)

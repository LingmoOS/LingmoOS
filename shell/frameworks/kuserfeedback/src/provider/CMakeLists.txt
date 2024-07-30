add_subdirectory(core)

if(TARGET Qt6::Widgets)
  add_subdirectory(widgets)
endif()

if(TARGET Qt6::Qml)
  add_subdirectory(qml)
endif()

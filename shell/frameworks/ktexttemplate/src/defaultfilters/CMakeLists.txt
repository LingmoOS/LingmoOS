add_library(ktexttemplate_defaultfilters MODULE
  defaultfilters.cpp
  datetime.cpp
  integers.cpp
  lists.cpp
  logic.cpp
  misc.cpp
  stringfilters.cpp
  defaultfilters.h
  datetime.h
  integers.h
  lists.h
  logic.h
  misc.h
  stringfilters.h
)
set_property(TARGET ktexttemplate_defaultfilters PROPERTY
    EXPORT_NAME defaultfilters
)
target_link_libraries(ktexttemplate_defaultfilters PRIVATE
  KF6::TextTemplate
)
ktexttemplate_adjust_plugin_name(ktexttemplate_defaultfilters)

install(TARGETS ktexttemplate_defaultfilters
  EXPORT KF6TextTemplateTargets
  DESTINATION ${KTextTemplate_INSTALL_PLUGINDIR}
)

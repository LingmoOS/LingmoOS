add_library(ktexttemplate_defaulttags MODULE
  defaulttags.cpp
  autoescape.cpp
  comment.cpp
  cycle.cpp
  debug.cpp
  filtertag.cpp
  firstof.cpp
  for.cpp
  if.cpp
  ifchanged.cpp
  ifequal.cpp
  load.cpp
  mediafinder.cpp
  now.cpp
  range.cpp
  regroup.cpp
  spaceless.cpp
  templatetag.cpp
  widthratio.cpp
  with.cpp

  defaulttags.h
  autoescape.h
  comment.h
  cycle.h
  debug.h
  filtertag.h
  firstof.h
  for.h
  if.h
  ifchanged.h
  ifequal.h
  load.h
  mediafinder.h
  now.h
  range.h
  regroup.h
  spaceless.h
  templatetag.h
  widthratio.h
  with.h
)
set_property(TARGET ktexttemplate_defaulttags PROPERTY
  EXPORT_NAME defaulttags
)
target_link_libraries(ktexttemplate_defaulttags PRIVATE
  KF6::TextTemplate
)
ktexttemplate_adjust_plugin_name(ktexttemplate_defaulttags)

install(TARGETS ktexttemplate_defaulttags 
  EXPORT KF6TextTemplateTargets
  DESTINATION ${KTextTemplate_INSTALL_PLUGINDIR}
)

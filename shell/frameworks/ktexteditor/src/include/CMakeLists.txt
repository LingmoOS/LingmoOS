# KTextEditor interface headers
ecm_generate_headers(KTextEditor_CamelCase_HEADERS
  HEADER_NAMES
  AnnotationInterface CodeCompletionModelControllerInterface MovingCursor Range LineRange TextHintInterface
  Cursor InlineNote InlineNoteProvider
  AbstractAnnotationItemDelegate
  Document  MovingRange View
  Attribute Command DocumentCursor Message MovingRangeFeedback SessionConfigInterface
  Editor
  CodeCompletionModel ConfigPage
  Application MainWindow Plugin
  PREFIX KTextEditor RELATIVE ktexteditor REQUIRED_HEADERS KTEXTEDITOR_PUBLIC_HEADERS)

# export headers to parent scope
set (KTEXTEDITOR_PUBLIC_HEADERS ${KTEXTEDITOR_PUBLIC_HEADERS} PARENT_SCOPE)

# install the public header files
install (FILES ${KTEXTEDITOR_PUBLIC_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KTextEditor/ktexteditor COMPONENT Devel)

# install CamelCase headers
install(FILES ${KTextEditor_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KTextEditor/KTextEditor COMPONENT Devel)

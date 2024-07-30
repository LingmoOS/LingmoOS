# jscripts for the part
add_subdirectory( script/data )

if(EditorConfig_FOUND)
    SET (CMAKE_REQUIRED_LIBRARIES ${EditorConfig_LIBRARIES})
    set (KTEXTEDITOR_OPTIONAL_LIBS ${KTEXTEDITOR_OPTIONAL_LIBS} ${EditorConfig_LIBRARIES})
    include_directories(${EditorConfig_INCLUDE_DIRS})
endif()

# handle include files, both normal ones and generated ones
add_subdirectory(include)

# includes
include_directories(
  # for config.h
  ${CMAKE_BINARY_DIR}

  # for generated ktexteditor headers
  ${CMAKE_CURRENT_BINARY_DIR}/include

  # for normal sources
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/ktexteditor
  ${CMAKE_CURRENT_SOURCE_DIR}/buffer
  ${CMAKE_CURRENT_SOURCE_DIR}/completion
  ${CMAKE_CURRENT_SOURCE_DIR}/dialogs
  ${CMAKE_CURRENT_SOURCE_DIR}/document
  ${CMAKE_CURRENT_SOURCE_DIR}/script
  ${CMAKE_CURRENT_SOURCE_DIR}/mode
  ${CMAKE_CURRENT_SOURCE_DIR}/render
  ${CMAKE_CURRENT_SOURCE_DIR}/search
  ${CMAKE_CURRENT_SOURCE_DIR}/syntax
  ${CMAKE_CURRENT_SOURCE_DIR}/undo
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}/inputmode
  ${CMAKE_CURRENT_SOURCE_DIR}/view
  ${CMAKE_CURRENT_SOURCE_DIR}/swapfile
  ${CMAKE_CURRENT_SOURCE_DIR}/variableeditor)

# KTextEditor interface sources
set(ktexteditor_LIB_SRCS
# text buffer & buffer helpers
buffer/katetextbuffer.cpp
buffer/katetextblock.cpp
buffer/katetextline.cpp
buffer/katetextcursor.cpp
buffer/katetextrange.cpp
buffer/katetexthistory.cpp
buffer/katetextfolding.cpp

# completion (widget, model, delegate, ...)
completion/katecompletionwidget.cpp
completion/katecompletionmodel.cpp
completion/katecompletiontree.cpp
completion/kateargumenthinttree.cpp
completion/kateargumenthintmodel.cpp
completion/katecompletiondelegate.cpp
completion/expandingtree/expandingwidgetmodel.cpp
completion/documentation_tip.cpp

# simple internal word completion
completion/katewordcompletion.cpp

# internal syntax-file based keyword completion
completion/katekeywordcompletion.cpp

# dialogs
dialogs/kateconfigpage.cpp
dialogs/katedialogs.cpp
dialogs/kateconfigpage.cpp
dialogs/clipboardhistorydialog.cpp

# document (THE document, buffer, lines/cursors/..., CORE STUFF)
document/katedocument.cpp
document/katebuffer.cpp

# undo
undo/kateundo.cpp
undo/kateundomanager.cpp

# scripting
script/katescript.cpp
script/kateindentscript.cpp
script/katecommandlinescript.cpp
script/katescriptmanager.cpp
script/katescriptaction.cpp

# scripting wrappers
script/katescripteditor.cpp
script/katescriptdocument.cpp
script/katescriptview.cpp
script/katescripthelpers.cpp

# mode (modemanager and co)
mode/katemodemanager.cpp
mode/katemodeconfigpage.cpp
mode/katemodemenu.cpp
mode/katemodemenulist.cpp

# modeline variable editor
variableeditor/variablelineedit.cpp
variableeditor/variablelistview.cpp
variableeditor/variableeditor.cpp
variableeditor/variableitem.cpp

# printing classes
printing/kateprinter.cpp
printing/printpainter.cpp
printing/printconfigwidgets.cpp

# rendering stuff (katerenderer  and helpers)
render/katerenderer.cpp
render/katerenderrange.cpp
render/katelayoutcache.cpp
render/katetextlayout.cpp
render/katelinelayout.cpp

# search stuff
search/kateplaintextsearch.cpp
search/kateregexpsearch.cpp
search/katematch.cpp
search/katesearchbar.cpp

# KSyntaxHighlighting integration
syntax/katecategorydrawer.cpp
syntax/katecolortreewidget.cpp
syntax/katehighlight.cpp
syntax/katehighlightingcmds.cpp
syntax/katehighlightmenu.cpp
syntax/katestyletreewidget.cpp
syntax/katesyntaxmanager.cpp
syntax/katethemeconfig.cpp

# view stuff (THE view and its helpers)
view/kateview.cpp
view/kateviewinternal.cpp
view/kateviewhelpers.cpp
view/kateannotationitemdelegate.cpp
view/katemessagewidget.cpp
view/katefadeeffect.cpp
view/kateanimation.cpp
view/katetextanimation.cpp
view/katetextpreview.cpp
view/katestatusbar.cpp
view/wordcounter.cpp
view/screenshotdialog.cpp

# spell checking
spellcheck/prefixstore.h
spellcheck/prefixstore.cpp
spellcheck/ontheflycheck.h
spellcheck/ontheflycheck.cpp
spellcheck/spellcheck.h
spellcheck/spellcheck.cpp
spellcheck/spellcheckdialog.h
spellcheck/spellcheckdialog.cpp
spellcheck/spellcheckbar.cpp
spellcheck/spellingmenu.h
spellcheck/spellingmenu.cpp

# generic stuff, unsorted...
utils/katecmds.cpp
utils/kateconfig.cpp
utils/katebookmarks.cpp
utils/kateautoindent.cpp
utils/kateindentdetecter.cpp
utils/katetemplatehandler.cpp
utils/kateglobal.cpp
utils/katecmd.cpp
utils/ktexteditor.cpp
utils/document.cpp
utils/range.cpp
utils/documentcursor.cpp
utils/attribute.cpp
utils/codecompletionmodel.cpp
utils/codecompletionmodelcontrollerinterface.cpp
utils/movingapi.cpp
utils/messageinterface.cpp
utils/application.cpp
utils/mainwindow.cpp
utils/katecommandrangeexpressionparser.cpp
utils/katesedcmd.cpp
utils/variable.cpp
utils/katevariableexpansionmanager.cpp
utils/katevariableexpansionhelpers.cpp

# swapfile
swapfile/kateswapdiffcreator.cpp
swapfile/kateswapfile.cpp

# export as HTML
export/exporter.cpp
export/htmlexporter.cpp

# input modes
inputmode/kateabstractinputmode.cpp
inputmode/kateabstractinputmodefactory.cpp
inputmode/katenormalinputmode.cpp
inputmode/katenormalinputmodefactory.cpp
)

# optionally compile with EditorConfig support
if(EditorConfig_FOUND)
    set(ktexteditor_LIB_SRCS ${ktexteditor_LIB_SRCS} document/editorconfig.cpp)
endif()

if(HAVE_KAUTH)
    set(ktexteditor_LIB_SRCS ${ktexteditor_LIB_SRCS} buffer/katesecuretextbuffer.cpp)
endif()

ecm_qt_declare_logging_category(ktexteditor_LIB_SRCS
    HEADER katepartdebug.h
    IDENTIFIER LOG_KTE
    CATEGORY_NAME kf.texteditor
    OLD_CATEGORY_NAMES org.kde.ktexteditor
    DEFAULT_SEVERITY Warning
    DESCRIPTION "ktexteditor lib"
    EXPORT KTEXTEDITOR
)


ki18n_wrap_ui(ktexteditor_LIB_SRCS
dialogs/textareaappearanceconfigwidget.ui
dialogs/bordersappearanceconfigwidget.ui
dialogs/completionconfigtab.ui
dialogs/navigationconfigwidget.ui
dialogs/editconfigwidget.ui
dialogs/filetypeconfigwidget.ui
dialogs/indentationconfigwidget.ui
dialogs/opensaveconfigwidget.ui
dialogs/opensaveconfigadvwidget.ui
dialogs/statusbarconfigwidget.ui
search/searchbarincremental.ui
search/searchbarpower.ui
spellcheck/spellcheckbar.ui
dialogs/spellcheckconfigwidget.ui)

qt_add_resources( ktexteditor_qrc_SRCS data/ktexteditor.qrc "${CMAKE_CURRENT_BINARY_DIR}/script/data/script.qrc")
set_source_files_properties(${ktexteditor_qrc_SRCS} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

# vi mode
ki18n_wrap_ui(ktexteditor_LIB_SRCS vimode/config/configwidget.ui)
set(ktexteditor_LIB_SRCS ${ktexteditor_LIB_SRCS} ${ktexteditor_qrc_SRCS}
inputmode/kateviinputmode.cpp
inputmode/kateviinputmodefactory.cpp

# vi input mode
vimode/config/configtab.cpp

vimode/modes/insertvimode.cpp
vimode/modes/modebase.cpp
vimode/modes/normalvimode.cpp
vimode/modes/replacevimode.cpp
vimode/modes/visualvimode.cpp

vimode/appcommands.cpp
vimode/cmds.cpp
vimode/inputmodemanager.cpp
vimode/command.cpp
vimode/motion.cpp
vimode/range.cpp
vimode/keyparser.cpp
vimode/globalstate.cpp
vimode/emulatedcommandbar/emulatedcommandbar.cpp
vimode/emulatedcommandbar/matchhighlighter.cpp
vimode/emulatedcommandbar/completer.cpp
vimode/emulatedcommandbar/activemode.cpp
vimode/emulatedcommandbar/interactivesedreplacemode.cpp
vimode/emulatedcommandbar/searchmode.cpp
vimode/emulatedcommandbar/commandmode.cpp
vimode/commandrangeexpressionparser.cpp
vimode/keyevent.cpp
vimode/keymapper.cpp
vimode/marks.cpp
vimode/jumps.cpp
vimode/history.cpp
vimode/macros.cpp
vimode/mappings.cpp
vimode/registers.cpp
vimode/searcher.cpp
vimode/completion.cpp
vimode/completionrecorder.cpp
vimode/completionreplayer.cpp
vimode/macrorecorder.cpp
vimode/lastchangerecorder.cpp
)

add_library(KF6TextEditor ${ktexteditor_LIB_SRCS} ${KTEXTEDITOR_PUBLIC_HEADERS})
ecm_generate_export_header(KF6TextEditor
    BASE_NAME KTextEditor
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)
add_library(KF6::TextEditor ALIAS KF6TextEditor)

target_include_directories(KF6TextEditor INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KTextEditor>")

# API is more or less KParts & KSyntaxHighlighting, other stuff is used only internally
target_link_libraries(KF6TextEditor
PUBLIC
  KF6::Parts
  KF6::Completion
  KF6::SyntaxHighlighting
PRIVATE
  Qt6::Qml
  Qt6::PrintSupport
  Qt6::TextToSpeech
  KF6::I18n
  KF6::Archive
  KF6::GuiAddons
  KF6::ItemViews
  KF6::SonnetCore
  KF6::SonnetUi
  KF6::Codecs
  KF6::KIOWidgets
  KF6::TextWidgets
  ${KTEXTEDITOR_OPTIONAL_LIBS}
)

if(HAVE_KAUTH)
  target_link_libraries(KF6TextEditor PRIVATE KF6::AuthCore)
endif()

set_target_properties(KF6TextEditor
   PROPERTIES
   VERSION ${KTEXTEDITOR_VERSION}
   SOVERSION ${KTEXTEDITOR_SOVERSION}
   EXPORT_NAME "TextEditor"
)

add_executable(ktexteditor_script_tester
    scripttester/ktexteditorscripttester.cpp
    scripttester/scripttester.cpp
    scripttester/ktexteditorscripttester.qrc
)
target_link_libraries(ktexteditor_script_tester
    Qt6::Qml
    KF6TextEditor
)
set_property(TARGET ktexteditor_script_tester PROPERTY OUTPUT_NAME ktexteditor-script-tester6)
install(TARGETS ktexteditor_script_tester ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# precompiled headers support
if(ENABLE_PCH)
    target_precompile_headers(KF6TextEditor
        PRIVATE
            <QList>
            <QPointer>
            <QSharedData>
            <QString>
            <QStringList>
            <QJsonDocument>
            <QTextFormat>
            <QWidget>
            <QObject>
            <QDebug>
            <algorithm>
            <functional>
            <type_traits>
            <utility>
            <vector>
    )
endif()

install(TARGETS KF6TextEditor EXPORT KF6TextEditorTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ktexteditor_export.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KTextEditor
  COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6TextEditor_QCH
        NAME KTextEditor
        BASE_NAME KF6TextEditor
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KTEXTEDITOR_PUBLIC_HEADERS}
             "${CMAKE_SOURCE_DIR}/docs/apidocs-groups.dox"
             "${CMAKE_SOURCE_DIR}/docs/coding-guidelines.dox"
             "${CMAKE_SOURCE_DIR}/docs/design.dox"
             "${CMAKE_SOURCE_DIR}/docs/porting.dox"
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            KF6Parts_QCH
        BLANK_MACROS
            KTEXTEDITOR_EXPORT
            KTEXTEDITOR_DEPRECATED
            KTEXTEDITOR_DEPRECATED_EXPORT
            "KTEXTEDITOR_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if(HAVE_KAUTH)
    add_executable(kauth_ktexteditor_helper buffer/katesecuretextbuffer.cpp)
    target_link_libraries(kauth_ktexteditor_helper KF6::AuthCore)
    install(TARGETS kauth_ktexteditor_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR} )
    kauth_install_helper_files(kauth_ktexteditor_helper org.kde.ktexteditor6.katetextbuffer root)
    kauth_install_actions(org.kde.ktexteditor6.katetextbuffer buffer/org.kde.ktexteditor6.katetextbuffer.actions)
endif()

# add part
add_subdirectory(part)

ecm_qt_install_logging_categories(
    EXPORT KTEXTEDITOR
    FILE ktexteditor.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

add_library(knewstuff_qml_STATIC STATIC)
target_sources(knewstuff_qml_STATIC PRIVATE
    quickengine.cpp
    quicksettings.cpp
    quickitemsmodel.cpp
    quickquestionlistener.cpp
    searchpresetmodel.cpp
    categoriesmodel.cpp
    commentsmodel.cpp
)
ecm_qt_declare_logging_category(knewstuff_qml_STATIC
    HEADER knewstuffquick_debug.h
    IDENTIFIER KNEWSTUFFQUICK
    CATEGORY_NAME kf.newstuff.quick
    OLD_CATEGORY_NAMES org.kde.knewstuff.quick
    DESCRIPTION "knewstuff (qtquick)"
    EXPORT KNEWSTUFF
)

set_property(TARGET knewstuff_qml_STATIC PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(knewstuff_qml_STATIC PUBLIC
    Qt6::Core
    Qt6::Gui # QImage
    Qt6::Qml
    KF6::ConfigCore
    KF6::I18n
    KF6::NewStuffCore
)

ecm_add_qml_module(newstuffqmlplugin URI "org.kde.newstuff" VERSION 1.0)

target_sources(newstuffqmlplugin PRIVATE
    qmlplugin.cpp

    author.cpp
    downloadlinkinfo.cpp
)

ecm_target_qml_sources(newstuffqmlplugin VERSION 1.1 SOURCES
    qml/Button.qml
    qml/Dialog.qml
    qml/DialogContent.qml
    qml/DownloadItemsSheet.qml
    qml/EntryDetails.qml
    qml/Page.qml
    qml/QuestionAsker.qml
)

ecm_target_qml_sources(newstuffqmlplugin VERSION 1.81 SOURCES
    qml/Action.qml
)

ecm_target_qml_sources(newstuffqmlplugin VERSION 1.85 SOURCES
    qml/UploadPage.qml
)

ecm_target_qml_sources(newstuffqmlplugin PRIVATE PATH private SOURCES
    qml/private/ConditionalLoader.qml
    qml/private/EntryCommentDelegate.qml
    qml/private/EntryCommentsPage.qml
    qml/private/EntryScreenshots.qml
    qml/private/ErrorDisplayer.qml
    qml/private/GridTileDelegate.qml
    qml/private/Rating.qml
    qml/private/Shadow.qml
)

ecm_target_qml_sources(newstuffqmlplugin PRIVATE PATH private/entrygriddelegates SOURCES
    qml/private/entrygriddelegates/BigPreviewDelegate.qml
    qml/private/entrygriddelegates/FeedbackOverlay.qml
    qml/private/entrygriddelegates/TileDelegate.qml
)

target_link_libraries (newstuffqmlplugin PRIVATE knewstuff_qml_STATIC)

ecm_finalize_qml_module(newstuffqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})

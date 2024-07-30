include(ECMPoQmTools)

add_library(KF6WidgetsAddons)
add_library(KF6::WidgetsAddons ALIAS KF6WidgetsAddons)

set_target_properties(KF6WidgetsAddons PROPERTIES
    VERSION     ${KWIDGETSADDONS_VERSION}
    SOVERSION   ${KWIDGETSADDONS_SOVERSION}
    EXPORT_NAME WidgetsAddons
)

ecm_create_qm_loader(KF6WidgetsAddons kwidgetsaddons6_qt)

target_sources(KF6WidgetsAddons PRIVATE
    common_helpers.cpp
    common_helpers_p.h
    fonthelpers_p.h
    kacceleratormanager.cpp
    kacceleratormanager.h
    kacceleratormanager_p.h
    kactionmenu.cpp
    kactionmenu.h
    kactionselector.cpp
    kactionselector.h
    kanimatedbutton.cpp
    kanimatedbutton.h
    kassistantdialog.cpp
    kassistantdialog.h
    kbusyindicatorwidget.cpp
    kbusyindicatorwidget.h
    kcapacitybar.cpp
    kcapacitybar.h
    kcharselect.cpp
    kcharselectdata.cpp
    kcharselectdata_p.h
    kcharselect.h
    kcharselect_p.h
    kcollapsiblegroupbox.cpp
    kcollapsiblegroupbox.h
    kcolorbutton.cpp
    kcolorbutton.h
    kcolorcombo.cpp
    kcolorcombo.h
    kcolumnresizer.cpp
    kcolumnresizer.h
    kcontextualhelpbutton.cpp
    kcontextualhelpbutton.h
    kcursor.cpp
    kcursor.h
    kcursor_p.h
    kdatecombobox.cpp
    kdatecombobox.h
    kdatepicker.cpp
    kdatepicker.h
    kdatepicker_p.h
    kdatepickerpopup.cpp
    kdatepickerpopup.h
    kdaterangecontrol.cpp
    kdaterangecontrol_p.h
    kdatetable.cpp
    kdatetable_p.h
    kdatetimeedit.cpp
    kdatetimeedit.h
    kdragwidgetdecorator.cpp
    kdragwidgetdecorator.h
    kdualaction.cpp
    kdualaction.h
    kdualaction_p.h
    keditlistwidget.cpp
    keditlistwidget.h
    kfontaction.cpp
    kfontaction.h
    kfontchooser.cpp
    kfontchooserdialog.cpp
    kfontchooserdialog.h
    kfontchooser.h
    kfontrequester.cpp
    kfontrequester.h
    kfontsizeaction.cpp
    kfontsizeaction.h
    kguiitem.cpp
    kguiitem.h
    kled.cpp
    kled.h
    klineediteventhandler.h
    klineediteventhandler.cpp
    kmessagebox.cpp
    kmessageboxdontaskagaininterface.h
    kmessagebox.h
    kmessageboxnotifyinterface.h
    kmessagebox_p.cpp
    kmessagebox_p.h
    kmessagedialog.cpp
    kmessagedialog.h
    kmessagewidget.cpp
    kmessagewidget.h
    kmimetypechooser.cpp
    kmimetypechooser.h
    kmimetypeeditor.cpp
    kmimetypeeditor.h
    kmultitabbar.cpp
    kmultitabbar.h
    kmultitabbar_p.h
    knewpassworddialog.cpp
    knewpassworddialog.h
    knewpasswordwidget.cpp
    knewpasswordwidget.h
    kpagedialog.cpp
    kpagedialog.h
    kpagedialog_p.h
    kpagemodel.cpp
    kpagemodel.h
    kpagemodel_p.h
    kpageview.cpp
    kpageview.h
    kpageview_p.cpp
    kpageview_p.h
    kpagewidget.cpp
    kpagewidget.h
    kpagewidgetmodel.cpp
    kpagewidgetmodel.h
    kpagewidgetmodel_p.h
    kpagewidget_p.h
    kpassword.h
    kpassworddialog.cpp
    kpassworddialog.h
    kpasswordlineedit.cpp
    kpasswordlineedit.h
    kpixmapregionselectordialog.cpp
    kpixmapregionselectordialog.h
    kpixmapregionselectorwidget.cpp
    kpixmapregionselectorwidget.h
    kpixmapsequence.cpp
    kpixmapsequence.h
    kpixmapsequenceoverlaypainter.cpp
    kpixmapsequenceoverlaypainter.h
    kpixmapsequencewidget.cpp
    kpixmapsequencewidget.h
    kpopupframe.cpp
    kpopupframe.h
    kratingpainter.cpp
    kratingpainter.h
    kratingwidget.cpp
    kratingwidget.h
    krecentfilesmenu.cpp
    krecentfilesmenu.h
    kruler.cpp
    kruler.h
    kselectaction.cpp
    kselectaction.h
    kselectaction_p.h
    kselector.cpp
    kselector.h
    kseparator.cpp
    kseparator.h
    ksplittercollapserbutton.cpp
    ksplittercollapserbutton.h
    ksqueezedtextlabel.cpp
    ksqueezedtextlabel.h
    kstandardguiitem.cpp
    kstandardguiitem.h
    kstyleextensions.cpp
    kstyleextensions.h
    ktimecombobox.cpp
    ktimecombobox.h
    ktitlewidget.cpp
    ktitlewidget.h
    ktoggleaction.cpp
    ktoggleaction.h
    ktoggleaction_p.h
    ktogglefullscreenaction.cpp
    ktogglefullscreenaction.h
    ktoolbarlabelaction.cpp
    ktoolbarlabelaction.h
    ktoolbarpopupaction.cpp
    ktoolbarpopupaction.h
    ktoolbarspaceraction.cpp
    ktoolbarspaceraction.h
    ktooltipwidget.cpp
    ktooltipwidget.h
    ktwofingerswipe.cpp
    ktwofingerswipe.h
    ktwofingertap.cpp
    ktwofingertap.h
    kurllabel.cpp # Not good enough quality. Needs to use QUrl instead of QString and should not inherit QLabel, but hold it as a member instead.
    kurllabel.h
    kviewstatemaintainerbase.cpp
    kviewstatemaintainerbase.h
    kviewstateserializer.cpp
    kviewstateserializer.h
    kxyselector.cpp
    kxyselector.h
    klineediturldropeventfilter.cpp
    klineediturldropeventfilter.h
    kjobwidgets.cpp
    kjobwidgets.h

    icons.qrc
)

set(kwidgetsaddons_UI_SRCS)
qt_wrap_ui(kwidgetsaddons_UI_SRCS
   kdatetimeedit.ui
   knewpassworddialog.ui
   knewpasswordwidget.ui
   kpassworddialog.ui
   kfontchooserwidget.ui
)
target_sources(KF6WidgetsAddons PRIVATE ${kwidgetsaddons_UI_SRCS})

ecm_qt_declare_logging_category(KF6WidgetsAddons
    HEADER loggingcategory.h
    IDENTIFIER KWidgetsAddonsLog
    CATEGORY_NAME kf.kwidgetsaddons
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KWidgetsAddons"
    EXPORT KWIDGETSADDONS
)

ecm_generate_export_header(KF6WidgetsAddons
    BASE_NAME KWidgetsAddons
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.5
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6WidgetsAddons PUBLIC Qt6::Widgets)

target_include_directories(KF6WidgetsAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KWidgetsAddons>")

ecm_generate_headers(KWidgetsAddons_HEADERS
  HEADER_NAMES
  KAcceleratorManager
  KAnimatedButton
  KBusyIndicatorWidget
  KCharSelect
  KCollapsibleGroupBox
  KColorButton
  KColumnResizer
  KContextualHelpButton
  KDateComboBox
  KDatePicker
  KDatePickerPopup
  KDateTimeEdit
  KDragWidgetDecorator
  KDualAction
  KFontAction
  KFontChooser
  KFontChooserDialog
  KFontSizeAction
  KGuiItem
  KLed
  KMessageBox
  KMessageBoxDontAskAgainInterface
  KMultiTabBar,KMultiTabBarButton,KMultiTabBarTab
  KNewPasswordWidget
  KPopupFrame
  KSelectAction
  KStandardGuiItem
  KTimeComboBox
  KUrlLabel
  KCapacityBar
  KFontRequester
  KPasswordDialog
  KRuler
  KRecentFilesMenu
  KSelector,KGradientSelector
  KTitleWidget
  KXYSelector
  KSeparator
  KSqueezedTextLabel
  KToggleAction
  KToggleFullScreenAction
  KViewStateSerializer
  KViewStateMaintainerBase
  KEditListWidget
  KCursor
  KRatingPainter
  KRatingWidget
  KActionSelector
  KColorCombo
  KActionMenu
  KToolBarLabelAction
  KToolBarPopupAction
  KToolBarSpacerAction
  KPageDialog
  KPageModel
  KPageView
  KPageWidget
  KPageWidgetModel,KPageWidgetItem
  KAssistantDialog
  KMessageWidget
  KMessageDialog
  KNewPasswordDialog
  KLineEditUrlDropEventFilter
  KLineEditEventHandler
  KPassword
  KPasswordLineEdit
  KPixmapSequence
  KPixmapSequenceOverlayPainter
  KPixmapSequenceWidget
  KPixmapRegionSelectorDialog
  KPixmapRegionSelectorWidget
  KMimeTypeChooser,KMimeTypeChooserDialog
  KMimeTypeEditor
  KMessageBoxNotifyInterface
  KSplitterCollapserButton
  KStyleExtensions
  KToolTipWidget
  KTwoFingerTap
  KTwoFingerSwipe
  KJobWidgets
  REQUIRED_HEADERS KWidgetsAddons_HEADERS
)

install(TARGETS KF6WidgetsAddons EXPORT KF6WidgetsAddonsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${KWidgetsAddons_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/kwidgetsaddons_export.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KWidgetsAddons COMPONENT Devel
)

qt_add_resources(KF6WidgetsAddons "kcharselect-data"
    PREFIX "/kf6/kcharselect/"
    FILES kcharselect-data
)

if (BUILD_QCH)
    ecm_add_qch(
        KF6WidgetsAddons_QCH
        NAME KWidgetsAddons
        BASE_NAME KF6WidgetsAddons
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KWidgetsAddons_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Core_QCH
            Qt6Gui_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KWIDGETSADDONS_EXPORT
            KWIDGETSADDONS_DEPRECATED_EXPORT
            KWIDGETSADDONS_DEPRECATED
            "KWIDGETSADDONS_DEPRECATED_VERSION(x, y, t)"
            "KWIDGETSADDONS_DEPRECATED_VERSION_BELATED(x, y, tx, ty, t)"
            "KWIDGETSADDONS_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KWIDGETSADDONS_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, tx, ty, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

ecm_qt_install_logging_categories(
    EXPORT KWIDGETSADDONS
    FILE kwidgetsaddons.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

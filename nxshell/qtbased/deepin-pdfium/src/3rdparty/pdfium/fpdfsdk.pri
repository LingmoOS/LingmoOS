# module fpdfsdk
SOURCE_DIR = $$PWD/pdfium

INCLUDEPATH += $$PWD/pdfium/third_party/freetype/src/include

HEADERS += \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_button.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_checkbox.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_combobox.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_formfiller.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_interactiveformfiller.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_listbox.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_pushbutton.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_radiobutton.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_textfield.h \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_textobject.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_button.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_caret.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_combo_box.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_edit.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_edit_ctrl.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_edit_impl.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_icon.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_list_box.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_list_impl.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_scroll_bar.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_special_button.h \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_wnd.h \
    $$PWD/pdfium/fpdfsdk/pwl/ipwl_systemhandler.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_actionhandler.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annot.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annothandlermgr.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annotiteration.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annotiterator.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_appstream.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_baannot.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_baannothandler.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_customaccess.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_fieldaction.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_filewriteadapter.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_formfillenvironment.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_helpers.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_interactiveform.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_pageview.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_pauseadapter.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_renderpage.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_widget.h \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_widgethandler.h \
    $$PWD/pdfium/fpdfsdk/ipdfsdk_annothandler.h

SOURCES += \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_button.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_checkbox.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_combobox.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_formfiller.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_interactiveformfiller.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_listbox.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_pushbutton.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_radiobutton.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_textfield.cpp \
    $$PWD/pdfium/fpdfsdk/formfiller/cffl_textobject.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_button.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_caret.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_combo_box.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_edit.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_edit_ctrl.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_edit_impl.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_icon.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_list_box.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_list_impl.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_scroll_bar.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_special_button.cpp \
    $$PWD/pdfium/fpdfsdk/pwl/cpwl_wnd.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_actionhandler.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annot.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annothandlermgr.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annotiteration.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_annotiterator.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_appstream.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_baannot.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_baannothandler.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_customaccess.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_fieldaction.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_filewriteadapter.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_formfillenvironment.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_helpers.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_interactiveform.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_pageview.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_pauseadapter.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_renderpage.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_widget.cpp \
    $$PWD/pdfium/fpdfsdk/cpdfsdk_widgethandler.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_annot.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_attachment.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_catalog.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_dataavail.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_doc.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_editimg.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_editpage.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_editpath.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_edittext.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_ext.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_flatten.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_formfill.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_javascript.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_ppo.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_progressive.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_save.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_searchex.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_signature.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_structtree.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_sysfontinfo.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_text.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_thumbnail.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_transformpage.cpp \
    $$PWD/pdfium/fpdfsdk/fpdf_view.cpp



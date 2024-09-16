// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_window.h"
#include "dsettingsoption.h"
#include "ddialog.h"
#include "qfileinfo.h"
#include "qfile.h"

int exec_ret = 1;
int QDialog_exec_stub()
{
    return exec_ret;
}

int intvalue=1;
int retintstub()
{
    return intvalue;
}

QString stringvalue="123";
QString retstringstub()
{
    return stringvalue;
}

bool isDraft=false;
bool EditWrapper_isDraftFile_stub()
{
    return isDraft;
}

bool isModified=false;
bool EditWrapper_isModified_stub()
{
    return isModified;
}

bool qfileinfo_exists= false;
bool QFileInfo_exists_stub()
{
    return qfileinfo_exists;
}

bool editwrapper_istemfile=false;
bool EditWrapper_isTemFile_stub()
{
    return editwrapper_istemfile;
}

bool editwrapper_saveFile = false;
bool EditWrapper_saveFile_stub()
{
    return editwrapper_saveFile;
}

bool editwrapper_saveAsFile = false;
bool EditWrapper_saveAsFile_stub(const QString &newFilePath, const QByteArray &encodeName)
{
    Q_UNUSED(newFilePath);
    Q_UNUSED(encodeName);
    return  editwrapper_saveAsFile;
}

EditWrapper* window_currentWrapper = nullptr;
EditWrapper* Window_currentWrapper_stub()
{
    return window_currentWrapper;
}

TextEdit* editwrapper_texteditor = nullptr;
TextEdit *EditWrapper_textEditor_stub()
{
    return editwrapper_texteditor;
}

bool window_saveasfile=false;
bool Window_saveAsFile_stub()
{
    return false;
}

int QFile_permissions_stub()
{
    return 0x2000;//The file is writable by the owner of the file.
}

void Window_updateSabeAsFileNameTemp_stub()
{
    return ;
}

StartManager::FileTabInfo StartManager_getFileTabInfo_stub()
{
    return StartManager::FileTabInfo{1,1};
}

QString utils_getkeyshortcut;
QString Utils_getKeyshortcut_stub()
{
    return utils_getkeyshortcut;
}


QString utils_getkeyshortcutfromkeymap;
QString Utils_getKeyshortcutFromKeymap_stub()
{
    return utils_getkeyshortcutfromkeymap;
}


int option_stub_para = 0;
QPointer<DSettingsOption> pDSettingsOption = nullptr;
QPointer<DSettingsOption> option_stub()
{
    if (pDSettingsOption == nullptr) {
        pDSettingsOption = new DSettingsOption;
    }
    if(option_stub_para == 0)
        pDSettingsOption->setValue("window_maximum");
    else if(option_stub_para ==1 )
        pDSettingsOption->setValue("fullscreen");
    else
        pDSettingsOption->setValue("normal");
    return pDSettingsOption;
}


UT_Window::UT_Window()
{

}

TEST(UT_Window_Window, UT_Window_Window_001)
{
    Window *pWindow = new Window;

    ASSERT_TRUE(pWindow->m_editorWidget != nullptr);
    pWindow->deleteLater();
}

TEST(UT_Window_showCenterWindow, UT_Window_showCenterWindow_001)
{
    Stub stub;
    stub.set(ADDR(DSettings,option),option_stub);

    Window * window1 = new Window();
    option_stub_para=0;
    window1->showCenterWindow(true);

    EXPECT_NE(window1->isVisible(),false);
    EXPECT_NE(window1,nullptr);

    window1->deleteLater();
    if (pDSettingsOption != nullptr) {
        pDSettingsOption->deleteLater();
    }
}

TEST(UT_Window_showCenterWindow, UT_Window_showCenterWindow_002)
{
    Window * window1 = new Window();

    Stub stub;
    stub.set(ADDR(DSettings,option),option_stub);

    option_stub_para=1;
    window1->showCenterWindow(true);

    EXPECT_NE(window1->isVisible(),false);
    EXPECT_NE(window1,nullptr);

    window1->deleteLater();
    if (pDSettingsOption != nullptr) {
        pDSettingsOption->deleteLater();
    }
}

TEST(UT_Window_showCenterWindow, UT_Window_showCenterWindow_003)
{
    Window * window1 = new Window();

    Stub stub;
    stub.set(ADDR(DSettings,option),option_stub);

    option_stub_para=2;
    window1->showCenterWindow(true);

    EXPECT_NE(window1->isVisible(),false);
    EXPECT_NE(window1,nullptr);

    window1->deleteLater();
    if (pDSettingsOption != nullptr) {
        pDSettingsOption->deleteLater();
    }
}


//initTitlebar
TEST(UT_Window_initTitlebar, UT_Window_initTitlebar)
{
    Window * window1 = new Window();
    window1->initTitlebar();

    EXPECT_NE(window1,nullptr);
    EXPECT_NE(window1->m_menu,nullptr);

    window1->deleteLater();



}
TEST(UT_Window_checkBlockShutdown, UT_Window_checkBlockShutdown)
{
    Window * window1 = new Window();
    EXPECT_NE(window1->checkBlockShutdown(),true);

    EXPECT_NE(window1,nullptr);

    window1->deleteLater();


}
//getTabIndex
TEST(UT_Window_getTabIndex, UT_Window_getTabIndex)
{
    Window * window1 = new Window();
    EXPECT_NE(window1->getTabIndex("aadd"),1);

    EXPECT_NE(window1,nullptr);

    window1->deleteLater();


}
int *stub1_exec()
{

    return nullptr;
}
//activeTab
TEST(UT_Window_activeTab, UT_Window_activeTab)
{
    Window * window1 = new Window();
    window1->activeTab(0);

    EXPECT_NE(window1->m_tabbar->currentIndex(),1);
    EXPECT_NE(window1,nullptr);

    window1->deleteLater();


}
//addTab
TEST(UT_Window_addTab, UT_Window_addTab)
{
    Window * window1 = new Window();
    window1->addTab("aabb",true);

    EXPECT_NE(window1->m_tabbar->currentIndex(),1);
    EXPECT_NE(window1,nullptr);

    window1->deleteLater();


}
//openFile
TEST(UT_Window_openFile, UT_Window_openFile_001)
{
    Window* window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub s1;
    s1.set(fptr,QDialog_exec_stub);

    exec_ret = 1;

    window->openFile();

    EXPECT_NE(window,nullptr);
    EXPECT_NE(a,nullptr);
    window->deleteLater();
    a->deleteLater();

}

TEST(UT_Window_openFile, UT_Window_openFile_002)
{
    Window* window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub s1;
    s1.set(fptr,QDialog_exec_stub);

    exec_ret = 0;

    window->openFile();

    EXPECT_NE(window,nullptr);
    EXPECT_NE(a,nullptr);
    window->deleteLater();
    a->deleteLater();

}

TEST(UT_Window_saveFile, UT_Window_saveFile_001)
{
    Window * window1 = new Window();

    isDraft=false;
    qfileinfo_exists=true;
    editwrapper_istemfile=false;//-
    editwrapper_saveFile=true;
    window_currentWrapper = new EditWrapper;

    Stub s0;s0.set(ADDR(Window,currentWrapper),Window_currentWrapper_stub);
    Stub s1;s1.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s2;s2.set((bool (QFileInfo::*)(void) const)ADDR(QFileInfo,exists),QFileInfo_exists_stub);
    Stub s3;s3.set(ADDR(EditWrapper,isTemFile),EditWrapper_isTemFile_stub);
    Stub s4;s4.set(ADDR(EditWrapper,saveFile),EditWrapper_saveFile_stub);
    Stub s5;s5.set(ADDR(Window,saveAsFile),Window_saveAsFile_stub);

    typedef QFileDevice::Permissions (*Fptr)(QFile*);
    Fptr fptr = (Fptr)((QFileDevice::Permissions(QFile::*)() const )&QFile::permissions);
    Stub s6;s6.set(fptr,QFile_permissions_stub);

    Stub s7;s7.set(ADDR(Window,updateSabeAsFileNameTemp),Window_updateSabeAsFileNameTemp_stub);

    window1->saveFile();


    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
   window_currentWrapper->deleteLater();
}


TEST(UT_Window_saveFile, UT_Window_saveFile_002)
{
    Window * window1 = new Window();

    isDraft=false;
    qfileinfo_exists=true;
    editwrapper_istemfile=true;//-
    editwrapper_saveFile=true;
    window_currentWrapper = new EditWrapper;

    Stub s0;s0.set(ADDR(Window,currentWrapper),Window_currentWrapper_stub);
    Stub s1;s1.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s2;s2.set((bool (QFileInfo::*)(void) const)ADDR(QFileInfo,exists),QFileInfo_exists_stub);
    Stub s3;s3.set(ADDR(EditWrapper,isTemFile),EditWrapper_isTemFile_stub);
    Stub s4;s4.set(ADDR(EditWrapper,saveFile),EditWrapper_saveFile_stub);
    Stub s5;s5.set(ADDR(Window,saveAsFile),Window_saveAsFile_stub);

    typedef QFileDevice::Permissions (*Fptr)(QFile*);
    Fptr fptr = (Fptr)((QFileDevice::Permissions(QFile::*)() const )&QFile::permissions);
    Stub s6;s6.set(fptr,QFile_permissions_stub);

    Stub s7;s7.set(ADDR(Window,updateSabeAsFileNameTemp),Window_updateSabeAsFileNameTemp_stub);

    window1->saveFile();


    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
    window_currentWrapper->deleteLater();
}


TEST(UT_Window_closeTab, UT_Window_closeTab_001)
{
    Window * window = new Window();
    window->addBlankTab("aabb");

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub stub;
    stub.set(fptr,QDialog_exec_stub);

    Stub s2;
    s2.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s3;
    s3.set(ADDR(EditWrapper,isModified),EditWrapper_isModified_stub);


    isDraft = true;
    isModified = true;
    exec_ret = 0;
    window->closeTab();


    EXPECT_NE(window,nullptr);

    window->deleteLater();

}

TEST(UT_Window_closeTab, UT_Window_closeTab_002)
{
    Window * window = new Window();
    window->addBlankTab("aabb");

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub stub;
    stub.set(fptr,QDialog_exec_stub);

    Stub s2;
    s2.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s3;
    s3.set(ADDR(EditWrapper,isModified),EditWrapper_isModified_stub);

    isDraft = true;
    isModified = true;
    exec_ret = 1;
    window->closeTab();

    EXPECT_NE(window,nullptr);

    window->deleteLater();
}


TEST(UT_Window_closeTab, UT_Window_closeTab_003)
{
    Window * window = new Window();
    window->addBlankTab("aabb");

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub stub;
    stub.set(fptr,QDialog_exec_stub);

    Stub s2;
    s2.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s3;
    s3.set(ADDR(EditWrapper,isModified),EditWrapper_isModified_stub);

    isDraft = true;
    isModified = true;
    exec_ret = 2;
    window->closeTab();

    EXPECT_NE(window,nullptr);

    window->deleteLater();
}

TEST(UT_Window_closeTab, UT_Window_closeTab_004)
{
    Window * window = new Window();
    window->addBlankTab("aabb");

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub stub;
    stub.set(fptr,QDialog_exec_stub);

    Stub s2;
    s2.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s3;
    s3.set(ADDR(EditWrapper,isModified),EditWrapper_isModified_stub);

    isDraft = false;
    isModified = true;
    exec_ret = 1;
    window->closeTab();

    EXPECT_NE(window,nullptr);
    window->deleteLater();
}


TEST(UT_Window_closeTab, UT_Window_closeTab_005)
{
    Window * window = new Window();
    window->addBlankTab("aabb");

    typedef int (*Fptr)(QDialog *);
    Fptr fptr = (Fptr)(&QDialog::exec);
    Stub stub;
    stub.set(fptr,QDialog_exec_stub);

    Stub s2;
    s2.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s3;
    s3.set(ADDR(EditWrapper,isModified),EditWrapper_isModified_stub);

    isDraft = false;
    isModified = true;
    exec_ret = 2;
    window->closeTab();

    EXPECT_NE(window,nullptr);
    window->deleteLater();
}

//saveAsFileToDisk
TEST(UT_Window_saveAsFileToDisk, UT_Window_saveAsFileToDisk_001)
{
    Window * window1 = new Window();

    window_currentWrapper = new EditWrapper;
    exec_ret=0;
    editwrapper_saveFile = true;
    isDraft=true;

    Stub s0;s0.set(ADDR(Window,currentWrapper),Window_currentWrapper_stub);
    typedef int (*Fptr)(QFileDialog *);
    Fptr fptr = (Fptr)(&QFileDialog::exec);
    Stub s1;
    s1.set(fptr,QDialog_exec_stub);
    Stub s2;s2.set(ADDR(EditWrapper,saveFile),EditWrapper_saveFile_stub);
    Stub s3;s3.set(ADDR(EditWrapper,updateSaveAsFileName),Window_updateSabeAsFileNameTemp_stub);
    Stub s4;s4.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);

    EXPECT_NE(window1->saveAsFileToDisk(),"1");

    EXPECT_NE(window1,nullptr);

    window1->deleteLater();
    window_currentWrapper->deleteLater();


}

TEST(UT_Window_saveAsFileToDisk, UT_Window_saveAsFileToDisk_002)
{
    Window * window1 = new Window();

    window_currentWrapper = new EditWrapper;
    exec_ret=1;
    editwrapper_saveFile = true;
    isDraft=false;
    editwrapper_saveAsFile = true;

    Stub s0;s0.set(ADDR(Window,currentWrapper),Window_currentWrapper_stub);
    typedef int (*Fptr)(QFileDialog *);
    Fptr fptr = (Fptr)(&QFileDialog::exec);
    Stub s1;s1.set(fptr,QDialog_exec_stub);
    Stub s2;s2.set(ADDR(EditWrapper,saveFile),EditWrapper_saveFile_stub);
    Stub s3;s3.set(ADDR(Window,updateSaveAsFileName),Window_updateSabeAsFileNameTemp_stub);
    Stub s4;s4.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);
    Stub s5;s5.set((bool(EditWrapper::*)(const QString &, const QByteArray&))ADDR(EditWrapper, saveAsFile), EditWrapper_saveAsFile_stub);

    EXPECT_NE(window1->saveAsFileToDisk(),"1");

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
    window_currentWrapper->deleteLater();
}

//saveBlankFileToDisk
TEST(UT_Window_saveBlankFileToDisk, UT_Window_saveBlankFileToDisk)
{
    Window * window1 = new Window();
    EXPECT_NE(window1->saveBlankFileToDisk(),"1");

    EXPECT_NE(window1,nullptr);

    window1->deleteLater();

}

//saveAsOtherTabFile
TEST(UT_Window_saveAsOtherTabFile, UT_Window_saveAsOtherTabFile_001)
{
    Window * window = StartManager::instance()->createWindow();
    window->addBlankTab("");
    window->addBlankTab("");
    window->addBlankTab("");
    EditWrapper *wrapper = window->currentWrapper();

    exec_ret=1;
    isDraft=false;

    typedef int (*Fptr)(QFileDialog *);
    Fptr fptr = (Fptr)(&QFileDialog::exec);
    Stub s1;s1.set(fptr,QDialog_exec_stub);
    Stub s4;s4.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);

    window->saveAsOtherTabFile(wrapper);


    EXPECT_NE(window,nullptr);
    window->deleteLater();

}

TEST(UT_Window_saveAsOtherTabFile, UT_Window_saveAsOtherTabFile_002)
{
    Window * window = StartManager::instance()->createWindow();
    window->addBlankTab("");
    window->addBlankTab("");
    window->addBlankTab("");
    EditWrapper *wrapper = window->currentWrapper();

    exec_ret=1;
    isDraft=true;

    typedef int (*Fptr)(QFileDialog *);
    Fptr fptr = (Fptr)(&QFileDialog::exec);
    Stub s1;s1.set(fptr,QDialog_exec_stub);
    Stub s4;s4.set(ADDR(EditWrapper,isDraftFile),EditWrapper_isDraftFile_stub);

    window->saveAsOtherTabFile(wrapper);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}

//changeSettingDialogComboxFontNumber
TEST(UT_Window_changeSettingDialogComboxFontNumber, UT_Window_changeSettingDialogComboxFontNumber)
{
    EditWrapper * e = new EditWrapper();
    Window * window1 = new Window();
    window1->changeSettingDialogComboxFontNumber(3);


    EXPECT_NE(window1,nullptr);
    EXPECT_NE(e,nullptr);
    window1->deleteLater();
    e->deleteLater();


}
//popupFindBar
TEST(UT_Window_popupFindBar, UT_Window_popupFindBar)
{
    Window * window1 = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window1->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window1->addBlankTab();
    window1->currentWrapper()->textEditor()->setPlainText(QString("12345"));
    window1->popupFindBar();

    window1->currentWrapper()->textEditor()->document()->clear();
    window1->popupFindBar();

    EXPECT_NE(window1->m_tabbar->count(),0);

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();



}

//popupReplaceBar
TEST(UT_Window_popupReplaceBar, UT_Window_popupReplaceBar)
{
    Window * window1 = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window1->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window1->addBlankTab();
    window1->currentWrapper()->textEditor()->setPlainText(QString("12345"));
    window1->popupReplaceBar();

    window1->currentWrapper()->textEditor()->document()->clear();
    window1->popupReplaceBar();

    EXPECT_NE(window1->m_replaceBar->isVisible(),false);


    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();
}

//popupJumpLineBar
TEST(UT_Window_popupJumpLineBar, UT_Window_popupJumpLineBar)
{
    Window * window1 = new Window();
    window1->popupJumpLineBar();

    EditWrapper *pEditWrapper = new EditWrapper();
    window1->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window1->addBlankTab();
    window1->currentWrapper()->textEditor()->setPlainText(QString("12345"));
    window1->popupJumpLineBar();

    window1->currentWrapper()->textEditor()->document()->clear();
    window1->popupJumpLineBar();

    EXPECT_NE(window1->m_jumpLineBar->isVisible(),false);


    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();


}

//updateJumpLineBar
TEST(UT_Window_updateJumpLineBar, UT_Window_updateJumpLineBar)
{
//    Window * window = new Window();
//    window->updateJumpLineBar();



}

//popupSettingsDialog
TEST(UT_Window_popupSettingsDialog, UT_Window_popupSettingsDialog)
{
    Window * window = new Window();

    exec_ret=1;

    typedef int (*Fptr)(DSettingsDialog *);
    Fptr fptr = (Fptr)(&DSettingsDialog::exec);
    Stub s1;s1.set(fptr,QDialog_exec_stub);

    window->popupSettingsDialog();


    ASSERT_NE(window,nullptr);
    window->deleteLater();

}
//popupPrintDialog

TEST(UT_Window_popupPrintDialog, UT_Window_popupPrintDialog)
{
//    Window * window = new Window();
//    window->popupPrintDialog();
//    delete window;
//    window = nullptr;


}
TEST(UT_Window_popupThemePanel, UT_Window_popupThemePanel)
{
    Window * window1 = new Window();
    window1->popupThemePanel();

    EXPECT_NE(window1->m_themePanel->isVisible(),false);

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();


}
//toggleFullscreen
TEST(UT_Window_toggleFullscreen, UT_Window_toggleFullscreen)
{
    Window * window1 = new Window();
    window1->toggleFullscreen();

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();


}
TEST(UT_Window_remberPositionSave, UT_Window_remberPositionSave)
{
//    Window * window = new Window();
//    window->remberPositionSave();
//    delete window;
//    window = nullptr;


}
//remberPositionRestore
TEST(UT_Window_remberPositionRestore, UT_Window_remberPositionRestore)
{
    Window * window1 = new Window();


    EditWrapper* wrapper = new EditWrapper;
    window1->m_remberPositionFilePath = "1234";
    window1->m_wrappers["1234"] = wrapper;


    window1->remberPositionRestore();

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();
    wrapper->deleteLater();

}
//displayShortcuts
TEST(UT_Window_displayShortcuts, UT_Window_displayShortcuts)
{
    Window * window1 = new Window();
    window1->displayShortcuts();

    EXPECT_NE(window1->m_shortcutViewProcess,nullptr);

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();


}
//setChildrenFocus
TEST(UT_Window_setChildrenFocus, UT_Window_setChildrenFocus)
{
    Window * window1 = new Window();
    window1->setChildrenFocus(false);
    window1->setChildrenFocus(true);

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();


}
//addBlankTab
TEST(UT_Window_addBlankTab, UT_Window_addBlankTab)
{
    Window * window1 = new Window();
    window1->addBlankTab("aabb");

    EXPECT_NE(window1->m_tabbar->count(),0);

    EXPECT_NE(window1,nullptr);
    window1->deleteLater();


}
//handleTabCloseRequested
TEST(UT_Window_addTabWithWrapper, UT_Window_addTabWithWrapper)
{
    EditWrapper* edit = new EditWrapper();
    Window *window = new Window();
    window->addTabWithWrapper(edit,"aabb","aabb","aabb");

    EXPECT_NE(window->m_tabbar->count(),0);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(edit,nullptr);
    edit->deleteLater();


}
TEST(UT_Window_handleTabCloseRequested, UT_Window_handleTabCloseRequested)
{
//    StartManager::instance()->createWindow()->addTabWithWrapper(edit,"aabb","aabb",0);
//    StartManager::instance()->createWindow()->handleTabCloseRequested(0);


}
//handleTabsClosed
TEST(UT_Window_handleTabsClosed, UT_Window_handleTabsClosed)
{
    QStringList aa;
    Window *window = new Window();
    window->handleTabsClosed(aa);

    EXPECT_NE(window->m_tabbar->count(),1);

    EXPECT_NE(window,nullptr);
    window->deleteLater();


}
//handleCurrentChanged
TEST(UT_Window_handleCurrentChanged, UT_Window_handleCurrentChanged)
{
    QStringList aa;
    Window *window = new Window();
    window->handleCurrentChanged(0);

    EXPECT_NE(window->m_findBar->isVisible(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
}
//slot_setTitleFocus
TEST(UT_Window_slot_setTitleFocus, UT_Window_slot_setTitleFocus)
{
//    QStringList aa;
    //window = new Window();
//    StartManager::instance()->createWindow()->slot_setTitleFocus();
    //window->slot_setTitleFocus();


}
//resizeEvent
TEST(UT_Window_resizeEvent, UT_Window_resizeEvent)
{
    QStringList aa;
    Window *window = new Window();
    QResizeEvent * eve= new QResizeEvent(QSize(100,100),QSize(50,50));
    window->resizeEvent(eve);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    delete eve;eve=nullptr;


}
//closeEvent
TEST(UT_Window_keyPressEvent, UT_Window_keyPressEvent_001)
{
//    QStringList aa;
//    Window *window = new Window();
//    window->m_settings = new Settings;
//    QKeyEvent * eve =nullptr;
//    utils_getkeyshortcut = Utils::getKeyshortcutFromKeymap(window->m_settings, "window", "decrementfontsize");
//    editwrapper_texteditor = new TextEdit;

//    Stub s1;s1.set(ADDR(Utils,getKeyshortcut),Utils_getKeyshortcut_stub);
//    Stub s2;s2.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

//    window->keyPressEvent(eve);

//    EXPECT_NE(window,nullptr);
//    window->deleteLater();
//    window->m_settings->deleteLater();
//    editwrapper_texteditor->deleteLater();
}


TEST(UT_Window_keyPressEvent, UT_Window_keyPressEvent_002)
{
//    QStringList aa;
//    Window *window = new Window();
//    window->m_settings = new Settings;
//    QKeyEvent * eve =nullptr;
//    utils_getkeyshortcut = "Alt+4";
//    editwrapper_texteditor = new TextEdit;

//    Stub s1;s1.set(ADDR(Utils,getKeyshortcut),Utils_getKeyshortcut_stub);
//    Stub s2;s2.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

//    window->keyPressEvent(eve);

//    EXPECT_NE(window,nullptr);
//    window->deleteLater();
//    window->m_settings->deleteLater();
//    editwrapper_texteditor->deleteLater();

}


TEST(UT_Window_keyPressEvent, UT_Window_keyPressEvent_003)
{
//    QStringList aa;
//    Window *window = new Window();
//    window->m_settings = new Settings;
//    QKeyEvent * eve =nullptr;
//    utils_getkeyshortcut = Utils::getKeyshortcutFromKeymap(window->m_settings, "window", "incrementfontsize");
//    editwrapper_texteditor = new TextEdit;

//    Stub s1;s1.set(ADDR(Utils,getKeyshortcut),Utils_getKeyshortcut_stub);
//    Stub s2;s2.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

//    window->keyPressEvent(eve);

//    EXPECT_NE(window,nullptr);
//    window->deleteLater();
//    window->m_settings->deleteLater();
//    editwrapper_texteditor->deleteLater();

}

TEST(UT_Window_keyPressEvent, UT_Window_keyPressEvent_004)
{
//    QStringList aa;
//    Window *window = new Window();
//    window->m_settings = new Settings;
//    QKeyEvent * eve =nullptr;
//    utils_getkeyshortcut = Utils::getKeyshortcutFromKeymap(window->m_settings, "window", "decrementfontsize");
//    editwrapper_texteditor = new TextEdit;

//    Stub s1;s1.set(ADDR(Utils,getKeyshortcut),Utils_getKeyshortcut_stub);
//    Stub s2;s2.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

//    window->keyPressEvent(eve);

//    EXPECT_NE(window,nullptr);
//    window->deleteLater();
//    window->m_settings->deleteLater();
//    editwrapper_texteditor->deleteLater();

}


TEST(UT_Window_keyPressEvent, UT_Window_keyPressEvent_005)
{
//    QStringList aa;
//    Window *window = new Window();
//    window->m_settings = new Settings;
//    QKeyEvent * eve =nullptr;
//    utils_getkeyshortcut = Utils::getKeyshortcutFromKeymap(window->m_settings, "window", "resetfontsize");
//    editwrapper_texteditor = new TextEdit;

//    Stub s1;s1.set(ADDR(Utils,getKeyshortcut),Utils_getKeyshortcut_stub);
//    Stub s2;s2.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

//    window->keyPressEvent(eve);

//    EXPECT_NE(window,nullptr);
//    window->deleteLater();
//    window->m_settings->deleteLater();
//    editwrapper_texteditor->deleteLater();

}

TEST(UT_Window_keyPressEvent, UT_Window_keyPressEvent_006)
{
//    QStringList aa;
//    Window *window = new Window();
//    window->m_settings = new Settings;
//    QKeyEvent * eve =nullptr;
//    utils_getkeyshortcut = Utils::getKeyshortcutFromKeymap(window->m_settings, "window", "togglefullscreen");
//    editwrapper_texteditor = new TextEdit;

//    Stub s1;s1.set(ADDR(Utils,getKeyshortcut),Utils_getKeyshortcut_stub);
//    Stub s2;s2.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);
//    Stub s3;s3.set(ADDR(Utils,getKeyshortcutFromKeymap),Utils_getKeyshortcutFromKeymap_stub);

//    window->keyPressEvent(eve);

//    EXPECT_NE(window,nullptr);
//    window->deleteLater();
//    window->m_settings->deleteLater();
//    editwrapper_texteditor->deleteLater();
}


//hideEvent
TEST(UT_Window_hideEvent, UT_Window_hideEvent)
{
    QStringList aa;
    Window *window = new Window();
    QHideEvent * eve;
    window->hideEvent(eve);

    EXPECT_NE(window->m_replaceBar->isVisible(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
}

//TextEdit
//void backupFile();
TEST(UT_Window_backupFile, UT_Window_backupFile)
{
    Window *window = new Window();

    EditWrapper* e1 = new EditWrapper;
    EditWrapper* e2 = new EditWrapper;
    window->m_wrappers["1"]=e1;
    window->m_wrappers["2"]=e2;

    Stub s1;s1.set(ADDR(StartManager,getFileTabInfo),StartManager_getFileTabInfo_stub);

    window->backupFile();


    EXPECT_NE(window,nullptr);
    window->deleteLater();


}

//void closeAllFiles();
TEST(UT_Window_closeAllFiles, UT_Window_closeAllFiles)
{
    Window *window = new Window();
    EXPECT_NE(window->closeAllFiles(),false);

    EXPECT_NE(window->m_tabbar->count(),1);

    EXPECT_NE(window,nullptr);
    window->deleteLater();


}

//void addTemFileTab(QString qstrPath,QString qstrName,QString qstrTruePath,bool bIsTemFile = false);
TEST(UT_Window_addTemFileTab, UT_Window_addTemFileTab)
{
    Window *window = new Window();
    window->addTemFileTab("aa","bb","cc","");

    EXPECT_NE(window->m_tabbar->count(),1);

    EXPECT_NE(window,nullptr);
    window->deleteLater();


}
//Window(DMainWindow *parent = nullptr);
//~Window() override;

////跟新文件修改状态
//void updateModifyStatus(const QString &path, bool isModified);
TEST(UT_Window_updateModifyStatus, UT_Window_updateModifyStatus)
{
    Window *window = new Window();
    window->updateModifyStatus("aa",false);
    window->updateModifyStatus("aa",true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();


}
////跟新tab文件名称
//void updateSaveAsFileName(QString strOldFilePath, QString strNewFilePath);
TEST(UT_Window_updateSaveAsFileName, UT_Window_updateSaveAsFileName)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->updateSaveAsFileName("aa","bb");

    EXPECT_NE(window->m_tabbar->indexOf("aa"),1);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}

//int getTabIndex(const QString &file);
//void activeTab(int index);

//Tabbar* getTabbar();
TEST(UT_Window_getTabbar, UT_Window_getTabbar)
{
    Window *window = new Window();
    EXPECT_NE(window->getTabbar(),nullptr);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}

//void addTab(const QString &filepath, bool activeTab = false);
//void addTabWithWrapper(EditWrapper *wrapper, const QString &filepath, const QString &qstrTruePath,
//                       const QString &tabName, int index = -1);
//bool closeTab();
//void restoreTab();
TEST(UT_Window_restoreTab, UT_Window_restoreTab)
{
    Window *window = new Window();
    window->restoreTab();
    EXPECT_NE(window->m_closeFileHistory.isEmpty(),false);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}

//void clearBlack();

//EditWrapper* createEditor();
TEST(UT_Window_createEditor, UT_Window_createEditor)
{
    Window *window = new Window();
    EXPECT_NE(window->createEditor(),nullptr);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}
//EditWrapper* currentWrapper();
TEST(UT_Window_currentWrapper, UT_Window_currentWrapper)
{
    Window *window = new Window();
    EXPECT_EQ(window->currentWrapper(),nullptr);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}
//EditWrapper* wrapper(const QString &filePath);
//TextEdit* getTextEditor(const QString &filepath);
TEST(UT_Window_getTextEditor, UT_Window_getTextEditor)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    EXPECT_NE(window->getTextEditor("aa"),nullptr);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void focusActiveEditor();
TEST(UT_Window_focusActiveEditor, UT_Window_focusActiveEditor)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->focusActiveEditor();

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void removeWrapper(const QString &filePath, bool isDelete = false);
TEST(UT_Window_removeWrapper, UT_Window_removeWrapper)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->removeWrapper("aa",true);
    window->removeWrapper("bb",false);

    EXPECT_EQ(window->m_wrappers.value("aa"),nullptr);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();


}

TEST(UT_Window_decrementFontSize, UT_Window_decrementFontSize)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->m_settings =Settings::instance();
    window->m_fontSize = 12;
    qreal oldFontSize = window->m_fontSize;
    window->decrementFontSize();
    EXPECT_GT(oldFontSize, window->m_fontSize);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void incrementFontSize();
TEST(UT_Window_incrementFontSize, UT_Window_incrementFontSize)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->m_settings =Settings::instance();
    qreal oldFontSize = window->m_fontSize;
    window->incrementFontSize();
    EXPECT_LT(oldFontSize, window->m_fontSize);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void resetFontSize();
TEST(UT_Window_resetFontSize, UT_Window_resetFontSize)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->m_settings =Settings::instance();
    window->resetFontSize();

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void setFontSizeWithConfig(EditWrapper *editor);
TEST(UT_Window_setFontSizeWithConfig, UT_Window_setFontSizeWithConfig)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->m_settings =Settings::instance();
    window->setFontSizeWithConfig(a);

    EXPECT_NE(window->m_fontSize,10);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}



//public slots:
//void addBlankTab();
//void addBlankTab(const QString &blankFile);
//void handleTabCloseRequested(int index);
//void handleTabsClosed(QStringList tabList);
//void handleCurrentChanged(const int &index);

//void handleJumpLineBarExit();
TEST(UT_Window_handleJumpLineBarExit, UT_Window_handleJumpLineBarExit)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->m_settings =Settings::instance();
    window->handleJumpLineBarExit();

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();



}
//void handleJumpLineBarJumpToLine(const QString &filepath, int line, bool focusEditor);
TEST(UT_Window_handleJumpLineBarJumpToLine, UT_Window_handleJumpLineBarJumpToLine)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleJumpLineBarJumpToLine("aa",1,true);
    window->handleJumpLineBarJumpToLine("aa",1,false);


    EXPECT_NE(window->m_wrappers.contains("aa"),false);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();


}

//void handleBackToPosition(const QString &file, int row, int column, int scrollOffset);
TEST(UT_Window_handleBackToPosition, UT_Window_handleBackToPosition)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleBackToPosition("aa",1,1,1);

    EXPECT_NE(window->m_wrappers.contains("bb"),false);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}

//void handleFindNext();
TEST(UT_Window_handleFindNextSearchKeyword, UT_Window_handleFindNextSearchKeyword)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleFindNextSearchKeyword("");

    EXPECT_NE(window->m_wrappers.contains("aadd"),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void handleFindPrev();
TEST(UT_Window_handleFindPrevSearchKeyword, UT_Window_handleFindPrevSearchKeyword)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleFindPrevSearchKeyword("");

    EXPECT_NE(window->m_wrappers.contains("aad"),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();


}
//void slotFindbarClose();
TEST(UT_Window_slotFindbarClose, UT_Window_slotFindbarClose)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->slotFindbarClose();

    EXPECT_NE(window->m_findBar->isVisible(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();


}
//void slotReplacebarClose();
TEST(UT_Window_slotReplacebarClose, UT_Window_slotReplacebarClose)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->slotReplacebarClose();

    EXPECT_NE(window->m_replaceBar->isVisible(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();


}

//void handleReplaceAll(const QString &replaceText, const QString &withText);
TEST(UT_Window_handleReplaceAll, UT_Window_handleReplaceAll)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleReplaceAll("","");


    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void handleReplaceNext(const QString &replaceText, const QString &withText);
TEST(UT_Window_handleReplaceNext, UT_Window_handleReplaceNext)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleReplaceNext("a", "", "");

    EXPECT_EQ(window->m_keywordForSearch,"");

    EXPECT_NE(window->m_findBar->isVisible(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void handleReplaceRest(const QString &replaceText, const QString &withText);
TEST(UT_Window_handleReplaceRest, UT_Window_handleReplaceRest)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleReplaceRest("","");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void handleReplaceSkip();
TEST(UT_Window_handleReplaceSkip, UT_Window_handleReplaceSkip)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleReplaceSkip("aa", "");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}

//void handleRemoveSearchKeyword();
TEST(UT_Window_handleRemoveSearchKeyword, UT_Window_handleRemoveSearchKeyword)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleRemoveSearchKeyword();


    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void handleUpdateSearchKeyword(QWidget *widget, const QString &file, const QString &keyword);
TEST(UT_Window_handleUpdateSearchKeyword, UT_Window_handleUpdateSearchKeyword)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleUpdateSearchKeyword(window,"aa","");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}


//void addBottomWidget(QWidget *widget);
//void removeBottomWidget();

//void loadTheme(const QString &path);
TEST(UT_Window_loadTheme, UT_Window_loadTheme)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->loadTheme("window");

    EXPECT_NE(window->m_themePath,"1");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}


//void showNewEditor(EditWrapper *wrapper);
TEST(UT_Window_showNewEditor, UT_Window_showNewEditor)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->showNewEditor(a);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void showNotify(const QString &message);
TEST(UT_Window_showNotify, UT_Window_showNotify)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->showNotify("ffffkkkk");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//int getBlankFileIndex();
TEST(UT_Window_getBlankFileIndex, UT_Window_getBlankFileIndex)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    EXPECT_NE(window->getBlankFileIndex(),0);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}

//DDialog *createDialog(const QString &title, const QString &content);
TEST(UT_Window_createDialog, UT_Window_createDialog)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    EXPECT_NE(window->createDialog("dd","ddd"),nullptr);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}

//void slotLoadContentTheme(DGuiApplicationHelper::ColorType themeType);
TEST(UT_Window_slotLoadContentTheme, UT_Window_slotLoadContentTheme)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->slotLoadContentTheme(DGuiApplicationHelper::ColorType::DarkType);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}

//void slotSettingResetTheme(const QString &path);
TEST(UT_Window_slotSettingResetTheme, UT_Window_slotSettingResetTheme)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->slotSettingResetTheme("DGuiApplicationHelper::ColorType::DarkType");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}

//void slotSigThemeChanged(const QString &path);
TEST(UT_Window_slotSigThemeChanged, UT_Window_slotSigThemeChanged)
{
    Window *pWindow = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    pWindow->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    pWindow->addBlankTab();
    pWindow->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);
    pWindow->m_settings = Settings::instance();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::LightType);
    pWindow->slotSigThemeChanged(DEEPIN_THEME);
    pWindow->slotSigThemeChanged(DEEPIN_DARK_THEME);

    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
    //pWindow->slotSigThemeChanged(DEEPIN_THEME);
    //pWindow->slotSigThemeChanged(DEEPIN_DARK_THEME);


    EXPECT_NE(pWindow,nullptr);
    pWindow->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();


}

//void slot_saveReadingPath();
TEST(UT_Window_slot_saveReadingPath, UT_Window_slot_saveReadingPath)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->slot_saveReadingPath();

    EXPECT_NE(window->m_reading_list.isEmpty(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void slot_beforeReplace(QString _);
TEST(UT_Window_slot_beforeReplace, UT_Window_slot_beforeReplace)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->slot_beforeReplace("d");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();

}
//void slot_setTitleFocus();

//private:
//void handleFocusWindowChanged(QWindow *w);
TEST(UT_Window_handleFocusWindowChanged, UT_Window_handleFocusWindowChanged)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    QWindow * q = new QWindow();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->handleFocusWindowChanged(q);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();
    EXPECT_NE(q,nullptr);
    q->deleteLater();

}
//void updateThemePanelGeomerty();
TEST(UT_Window_updateThemePanelGeomerty, UT_Window_updateThemePanelGeomerty)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    QWindow * q = new QWindow();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->updateThemePanelGeomerty();

    EXPECT_NE(window->m_themePanel->geometry().x(),0);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();
    EXPECT_NE(q,nullptr);
    q->deleteLater();


}

//void checkTabbarForReload();
TEST(UT_Window_checkTabbarForReload, UT_Window_checkTabbarForReload)
{
    Window *window = new Window();
    EditWrapper * a = new EditWrapper();
    QWindow * q = new QWindow();
    window->addTabWithWrapper(a,"aa","aad","aadd",0);
    window->addBlankTab();
    window->addTabWithWrapper(a,"bb","aad","aadd",1);
    window->m_settings =Settings::instance();
    window->checkTabbarForReload();

    EXPECT_NE(window->m_tabbar->currentName(),"aa");

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(a,nullptr);
    a->deleteLater();
    EXPECT_NE(q,nullptr);
    q->deleteLater();


}

//void slotSigAdjustFont();
TEST(UT_Window_slotSigAdjustFont, UT_Window_slotSigAdjustFont)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper,"bb", "aad", "aadd", 1);
    window->slotSigAdjustFont(QString());

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();


}

//void slotSigAdjustFontSize();
TEST(UT_Window_slotSigAdjustFontSize, UT_Window_slotSigAdjustFontSize)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);
    window->slotSigAdjustFontSize(14);

    EXPECT_EQ(window->m_fontSize,14);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();


}

//void slotSigAdjustTabSpaceNumber();
TEST(UT_Window_slotSigAdjustTabSpaceNumber, UT_Window_slotSigAdjustTabSpaceNumber)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);

    window->slotSigAdjustTabSpaceNumber(14);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();

}

//void slotSigAdjustWordWrap();
TEST(UT_Window_slotSigAdjustWordWrap, UT_Window_slotSigAdjustWordWrap)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);

    window->slotSigAdjustWordWrap(true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();

}

//void slotSigAdjustBookmark();
TEST(UT_Window_slotSigAdjustBookmark, UT_Window_slotSigAdjustBookmark)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);

    window->slotSigAdjustBookmark(true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();

}

//void slotSigShowBlankCharacter();
TEST(UT_Window_slotSigShowBlankCharacter, UT_Window_slotSigShowBlankCharacter)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);

    window->slotSigShowBlankCharacter(true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();

}

//void slotSigHightLightCurrentLine();
TEST(UT_Window_slotSigHightLightCurrentLine, UT_Window_slotSigHightLightCurrentLine)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);

    window->slotSigHightLightCurrentLine(true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();

}

//void slotSigShowCodeFlodFlag();
TEST(UT_Window_slotSigShowCodeFlodFlag, UT_Window_slotSigShowCodeFlodFlag)
{
    Window *window = new Window();
    EditWrapper *pEditWrapper = new EditWrapper();
    window->addTabWithWrapper(pEditWrapper, "aa", "aad", "aadd", 0);
    window->addBlankTab();
    window->addTabWithWrapper(pEditWrapper, "bb", "aad", "aadd", 1);

    window->slotSigShowCodeFlodFlag(true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();
    EXPECT_NE(pEditWrapper,nullptr);
    pEditWrapper->deleteLater();

}

//void slotSigShowCodeFlodFlag();
TEST(UT_Window_slotSigChangeWindowSize, UT_Window_slotSigChangeWindowSize)
{
    Window *window = new Window();
    window->slotSigChangeWindowSize(QString());
    window->slotSigChangeWindowSize(QString("fullscreen"));
    window->slotSigChangeWindowSize(QString("window_maximum"));

    EXPECT_EQ(window->isVisible(),true);

    EXPECT_NE(window,nullptr);
    window->deleteLater();

}



TEST(UT_Window_printPage, UT_Window_printPage)
{
    Window *window = new Window();
    QPainter* painter = new QPainter;
    QTextDocument* doc = new QTextDocument;
    QRectF body,pageContBox;
    Window::printPage(0,painter,doc,body,pageContBox);

    EXPECT_NE(window,nullptr);
    EXPECT_NE(window,nullptr);
    delete painter;painter=nullptr;
    doc->deleteLater();

}

TEST(UT_Window_delete_window, UT_Window_delete_window)
{
    Window* w = new Window();
    w->m_shortcutViewProcess = new QProcess;

    EXPECT_NE(w,nullptr);
    w->deleteLater();
    //w->m_shortcutViewProcess will be deleted in ~Window;
}

TEST(UT_Window_doprint, UT_Window_doprint)
{
    Window* w = new Window();
    DPrinter* p = new DPrinter;
    w->m_printDoc = new QTextDocument;
    w->m_pPreview = new DPrintPreviewDialog;

    editwrapper_texteditor = new TextEdit;
    Stub s1;s1.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

    QVector<int> pages{1,2,3,4,5};
#if (DTK_VERSION_MAJOR > 5 \
 || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR > 4) \
 || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR == 4 && DTK_VERSION_PATCH >= 10))
    w->doPrint(p,pages);
#endif

    EXPECT_NE(w, nullptr);
    w->deleteLater();
    editwrapper_texteditor->deleteLater();
    w->m_printDoc->deleteLater();
    w->m_pPreview->deleteLater();
    delete p;
    p = nullptr;
}

TEST(UT_Window_doprint, UT_Window_doPrintWithLargeDoc)
{
    Window* w = new Window();
    DPrinter* p = new DPrinter;
    w->m_pPreview = new DPrintPreviewDialog;
    Window::PrintInfo info;
    info.doc = new QTextDocument;
    w->m_printDocList.append(info);

    editwrapper_texteditor = new TextEdit;
    Stub s1;s1.set(ADDR(EditWrapper,textEditor),EditWrapper_textEditor_stub);

    QVector<int> pages{1,2,3,4,5};
#if (DTK_VERSION_MAJOR > 5 \
 || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR > 4) \
 || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR == 4 && DTK_VERSION_PATCH >= 10))
    w->doPrintWithLargeDoc(p, pages);
#endif

    EXPECT_NE(w, nullptr);
    w->deleteLater();
    editwrapper_texteditor->deleteLater();
    w->m_printDoc->deleteLater();
    w->m_pPreview->deleteLater();
    delete p;
    p = nullptr;
}

void stubProcessEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents)
{
    Q_UNUSED(flags);
}

TEST(UT_Window_doprint, UT_Window_cloneLargeDocument)
{
    Window* w = new Window();
    EditWrapper* wra = new EditWrapper(w);
    QString text = "123";
    wra->textEditor()->document()->setPlainText(text);

    // 不在ut中使用Eventloop
    typedef void (*Func)(QEventLoop::ProcessEventsFlags);
    Stub s1;
    s1.set((Func)ADDR(QCoreApplication, processEvents), stubProcessEvents);

    // 拷贝数据
    w->cloneLargeDocument(wra);
    EXPECT_FALSE(w->m_printDocList.isEmpty());
    if (!w->m_printDocList.isEmpty()) {
        EXPECT_NE(w->m_printDocList.first().doc, nullptr);
    }

    // 清空数据
    w->clearPrintTextDocument();
    EXPECT_TRUE(w->m_printDocList.isEmpty());

    wra->deleteLater();
    w->deleteLater();
}


TEST(UT_Window_slotClearDoubleCharaterEncode, UT_Window_slotClearDoubleCharaterEncode)
{
    Window* w = new Window();

    Stub s1;s1.set(ADDR(Window,handleReplaceAll),Window_updateSabeAsFileNameTemp_stub);

    w->slotClearDoubleCharaterEncode();

    EXPECT_NE(w,nullptr);
    w->deleteLater();
}



TEST(UT_Window_dropEvent, UT_Window_dropEvent)
{
    Window* w = new Window();
    QMimeData* data = new QMimeData;
    QList<QUrl> urls = {QUrl("http://")};
    data->setUrls(urls);
    QDropEvent* d = new QDropEvent(QPointF(100,100), Qt::MoveAction,data ,Qt::LeftButton, Qt::ShiftModifier);

    w->dropEvent(d);

    EXPECT_NE(w,nullptr);
    w->deleteLater();
    data->deleteLater();
    delete d; d=nullptr;
}

TEST(UT_Window_setPrintEnabled, UT_Window_setPrintEnabled)
{
    Window* w = new Window();
    w->m_menu = new DMenu;
    w->setPrintEnabled(true);

    EXPECT_NE(w,nullptr);
    w->deleteLater();
}

TEST(UT_Window_getStackedWgt, UT_Window_getStackedWgt)
{
    Window* w = new Window();
    w->getStackedWgt();

    EXPECT_NE(w,nullptr);
    w->deleteLater();
}


TEST(UT_Window_asynPrint, UT_Window_asynPrint)
{
    Window* w = new Window();
    QPainter* painter = new QPainter(w);
    DPrinter* printer = new DPrinter();
    w->m_printDoc = new QTextDocument;
    QVector<int> pr{1,2,3};

    Stub stub;
    stub.set(ADDR(Window,printPage),retintstub);
    //bool newPage() override;
    typedef bool (*fptr)(DPrinter*);
    fptr A_foo = (fptr)(&DPrinter::newPage);
    stub.set(A_foo,retintstub);

    w->asynPrint(*painter,printer,pr);

    EXPECT_NE(w,nullptr);
    w->deleteLater();
    delete painter;
    painter=nullptr;
    delete printer;
    printer=nullptr;
    w->m_printDoc->deleteLater();
}

TEST(UT_Window_slot_setTitleFocus, UT_Window_slot_setTitleFocus_002)
{
    Window* w = new Window();
    EditWrapper* wra = new EditWrapper(w);
    w->m_wrappers["a"] = wra;

    Stub stub;
    stub.set(ADDR(Tabbar,currentPath),retstringstub);
    //bool newPage() override;
//    typedef bool (*fptr)(DPrinter*);
//    fptr A_foo = (fptr)(&DPrinter::newPage);
//    stub.set(A_foo,retintstub);

    stringvalue = "a";
    w->slot_setTitleFocus();

    EXPECT_NE(w,nullptr);
    w->deleteLater();
}

TEST(UT_Window_rehighlightPrintDoc, rehighlightPrintDoc_HighlightCpp_pass)
{
    Window* w = new Window;
    w->addBlankTab();
    w->m_printWrapper = w->currentWrapper();
    QTextDocument *doc = new QTextDocument;
    doc->setPlainText("#include <iostream>;\n"
                      "int main(int argc, char *argv[]) { }");

    CSyntaxHighlighter *highlighter = new CSyntaxHighlighter(doc);
    KSyntaxHighlighting::Repository repository;
    highlighter->setDefinition(repository.definitionForName("C++"));
    highlighter->setTheme(repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

    w->rehighlightPrintDoc(doc, highlighter);
    EXPECT_FALSE(doc->firstBlock().textFormats().isEmpty());

    doc->deleteLater();
    w->deleteLater();
}



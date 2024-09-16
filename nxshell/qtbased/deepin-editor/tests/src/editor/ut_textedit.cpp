// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_textedit.h"
#include "stub.h"
#include "../../src/widgets/window.h"
#include <QUndoStack>
#include "QDBusReply"
#include "QDBusConnection"


namespace texteditstub {

bool rettruestub()
{
    return true;
}

bool retfalsestub()
{
    return false;
}

int intvalue = 1;
int retintstub()
{
    return intvalue;
}

int intvalue2=1;
int retintstub2()
{
    return intvalue2;
}

QString string1="1";
QString retstring1()
{
    return string1;
}

QString string2="2";
QString retstring2()
{
    return string2;
}

QDBusConnection sessionBusstub()
{
    return QDBusConnection("123");
}
}

using namespace texteditstub;

test_textedit::test_textedit()
{
    QString text = QString("#include \"window.h\"\n"
                           "#include \"urlinfo.h\"\n"
                           "int main(int argc, char *argv[])\n"
                           "{\n"
                           "using namespace Dtk::Core;\n"
                           "PerformanceMonitor::initializeAppStart();\n"
                           "return 0;\n"
                           "}");
    QFile f("1.cpp");
    if (f.open(QFile::WriteOnly)) {
        f.write(text.toUtf8());
        f.close();
    }
}

void test_textedit::forstub(QPoint q)
{
}

TEST_F(test_textedit, setWrapper)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_wrapper != nullptr);

    pWindow->deleteLater();
}

//getWrapper
TEST_F(test_textedit, getWrapper)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    EditWrapper *pEditWrapper = pWindow->currentWrapper()->textEditor()->getWrapper();
    ASSERT_TRUE(pEditWrapper != nullptr);

    pWindow->deleteLater();
}

void showCenterWindow_stub()
{
    return;
}

//inline QString getFilePath() { return m_sFilePath;};
TEST_F(test_textedit, getFilePath)
{
    Stub stub_showCenterWindow;
    stub_showCenterWindow.set(ADDR(Window, showCenterWindow), showCenterWindow_stub);
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QString strRet(pWindow->currentWrapper()->textEditor()->getFilePath());
    ASSERT_TRUE(strRet != nullptr);
    pWindow->deleteLater();
}

//inline void setFilePath(QString file) { m_sFilePath = file;}
TEST_F(test_textedit, setFilePath)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    startManager->setFilePath("a");

    ASSERT_TRUE(startManager->m_pLeftAreaWidget != nullptr);
    startManager->deleteLater();
    p->deleteLater();
}

//getLeftAreaWidget
TEST_F(test_textedit, getLeftAreaWidget)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    LeftAreaTextEdit *pLeftAreaTextEdit = pWindow->currentWrapper()->textEditor()->getLeftAreaWidget();
    ASSERT_TRUE(pLeftAreaTextEdit != nullptr);
    pWindow->deleteLater();
}

//isUndoRedoOpt
TEST_F(test_textedit, isUndoRedoOpt)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    bool bRet = pWindow->currentWrapper()->textEditor()->isUndoRedoOpt();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//getModified
TEST_F(test_textedit, getModified)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    bool bRet = pWindow->currentWrapper()->textEditor()->getModified();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//TextEdit(QWidget *parent = nullptr);
TEST_F(test_textedit, TextEdit)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->verticalScrollBarPolicy()   == Qt::ScrollBarAsNeeded);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->horizontalScrollBarPolicy() == Qt::ScrollBarAsNeeded);

    pWindow->deleteLater();
}

//insertTextEx
TEST_F(test_textedit, insertTextEx)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(!strRet.isEmpty());

    pWindow->deleteLater();
}

//deleteTextEx
TEST_F(test_textedit, deleteTextEx)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->deleteTextEx(pWindow->currentWrapper()->textEditor()->textCursor());
    bool bRet = pWindow->currentWrapper()->textEditor()->m_pUndoStack->canUndo();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//insertSelectTextEx
TEST_F(test_textedit, insertSelectTextEx)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->insertSelectTextEx(pWindow->currentWrapper()->textEditor()->textCursor(), QString("Holle world."));
    bool bRet = pWindow->currentWrapper()->textEditor()->m_pUndoStack->canUndo();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//insertColumnEditTextEx
TEST_F(test_textedit, insertColumnEditTextEx)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->insertColumnEditTextEx(QString("Holle world."));
    bool bRet = pWindow->currentWrapper()->textEditor()->m_pUndoStack->canUndo();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//deleteSelectTextEx
TEST_F(test_textedit, deleteSelectTextEx)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->deleteSelectTextEx(textCursor);
    QString strAfter(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(strBefore.compare(strAfter));

    pWindow->deleteLater();
}

//deleteSelectTextEx(QTextCursor,QString text,bool currLine)
TEST_F(test_textedit, deleteSelectTextEx_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->deleteSelectTextEx(textCursor, QString("Holle world."), true);
    QString strAfter(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(strBefore.compare(strAfter));

    pWindow->deleteLater();
}

//slotSelectionChanged 001
TEST_F(test_textedit, slotSelectionChanged_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotSelectionChanged();
    int iRet = QApplication::cursorFlashTime();
    ASSERT_TRUE(iRet == 0);

    pWindow->deleteLater();
}

//slotSelectionChanged 002
TEST_F(test_textedit, slotSelectionChanged_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->slotSelectionChanged();
    int iRet = QApplication::cursorFlashTime();
    ASSERT_TRUE(iRet != 0);

    pWindow->deleteLater();
}

//slotCanRedoChanged
TEST_F(test_textedit, slotCanRedoChanged)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->slotCanRedoChanged(true);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pUndoStack != nullptr);

    pWindow->deleteLater();
}

//slotCanUndoChanged
TEST_F(test_textedit, slotCanUndoChanged)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->slotCanUndoChanged(true);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pUndoStack != nullptr);

    pWindow->deleteLater();
}

//slotValueChanged
TEST_F(test_textedit, slotValueChanged)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMassege("Holle world.Holle world.Holle world.Holle world.Holle world.Holle world.Holle world.Holle world.Holle world.");
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(), strMassege);
    pWindow->currentWrapper()->textEditor()->setLineWrapMode(QPlainTextEdit::NoWrap);
    int iRetBefore = pWindow->currentWrapper()->textEditor()->horizontalScrollBar()->value();
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->slotValueChanged(true);
    int iRetAfter = pWindow->currentWrapper()->textEditor()->horizontalScrollBar()->value();
    ASSERT_TRUE(iRetBefore != iRetAfter);

    pWindow->deleteLater();
}

//getCurrentLine
TEST_F(test_textedit, getCurrentLine)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\n Create operating system innovation ecosystem!"));
    int iRet = pWindow->currentWrapper()->textEditor()->getCurrentLine();
    ASSERT_TRUE(iRet == 2);

    pWindow->deleteLater();
}

//getCurrentColumn
TEST_F(test_textedit, getCurrentColumn)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    int iRet = pWindow->currentWrapper()->textEditor()->getCurrentColumn();
    ASSERT_TRUE(iRet == QString("Holle world.").length());

    pWindow->deleteLater();
}

//getPosition
TEST_F(test_textedit, getPosition)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    int iRet = pWindow->currentWrapper()->textEditor()->getPosition();
    ASSERT_TRUE(iRet == QString("Holle world.").length());

    pWindow->deleteLater();
}

//getScrollOffset
TEST_F(test_textedit, getScrollOffset)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw"
                                                                  "\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H"
                                                                  "\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no"
                                                                  "\nr\nl\nd\n."));
    int iRet = pWindow->currentWrapper()->textEditor()->getScrollOffset();
    ASSERT_TRUE(iRet > 0);

    pWindow->deleteLater();
}

//DMenu *getHighlightMenu();
TEST_F(test_textedit, getHighlightMenu)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));

    DMenu *pDMenu = pWindow->currentWrapper()->textEditor()->getHighlightMenu();
    ASSERT_TRUE(pDMenu == nullptr);
    pWindow->deleteLater();
}

//forwardChar 001
TEST_F(test_textedit, forwardChar_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->forwardChar();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 1) && (!strRet.compare(QString("H"))));

    pWindow->deleteLater();
}

//forwardChar 002
TEST_F(test_textedit, forwardChar_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->forwardChar();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 1) && strRet.isEmpty());

    pWindow->deleteLater();
}

//backwardChar 001
TEST_F(test_textedit, backwardChar_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->backwardChar();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 10) && (!strRet.compare(QString("d"))));

    pWindow->deleteLater();
}

//backwardChar 002
TEST_F(test_textedit, backwardChar_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->backwardChar();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 10) && strRet.isEmpty());

    pWindow->deleteLater();
}

//forwardWord 001
TEST_F(test_textedit, forwardWord_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->forwardWord();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 6) && (!strRet.compare(QString("Holle "))));

    pWindow->deleteLater();
}

//forwardWord 002
TEST_F(test_textedit, forwardWord_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->forwardWord();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 6) && strRet.isEmpty());

    pWindow->deleteLater();
}

//backwardWord 001
TEST_F(test_textedit, backwardWord_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->backwardWord();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 6) && (!strRet.compare(QString("world"))));

    pWindow->deleteLater();
}

//backwardWord 002
TEST_F(test_textedit, backwardWord_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->backwardWord();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE((iRet == 6) && strRet.isEmpty());

    pWindow->deleteLater();
}

bool forwardPair_001_find_stub()
{
    return true;
}

//forwardPair 001
TEST_F(test_textedit, forwardPair_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    #if 0 //保留打桩方法，后续完善覆盖率
    typedef int (*fptr)(QDialog *);
    fptr qDialogExec = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(qDialogExec, forwardPair_001_find_stub);

    Stub find_stub;
    find_stub.set((bool(QPlainTextEdit::*)(const QRegExp &))ADDR(QPlainTextEdit, QPlainTextEdit::find), forwardPair_001_find_stub);
    #endif
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->forwardPair();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//backwardPair 001
TEST_F(test_textedit, backwardPair_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    #if 0 //保留打桩方法，后续完善覆盖率
    typedef int (*fptr)(QDialog *);
    fptr qDialogExec = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(qDialogExec, forwardPair_001_find_stub);

    Stub find_stub;
    find_stub.set((bool(QPlainTextEdit::*)(const QRegExp &))ADDR(QPlainTextEdit, QPlainTextEdit::find), forwardPair_001_find_stub);
    #endif
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->backwardPair();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//blockCount
TEST_F(test_textedit, blockCount)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    int iRet = pWindow->currentWrapper()->textEditor()->blockCount();
    ASSERT_TRUE(iRet == 1);

    pWindow->deleteLater();
}

//int characterCount() const;
TEST_F(test_textedit, characterCount)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    int iRet = pWindow->currentWrapper()->textEditor()->characterCount();
    ASSERT_TRUE(iRet == 12);
    pWindow->deleteLater();
}

//firstVisibleBlock()
TEST_F(test_textedit, firstVisibleBlock)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    QTextBlock textBlock = pWindow->currentWrapper()->textEditor()->firstVisibleBlock();
    QString strRet(textBlock.text());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));
    pWindow->deleteLater();
}

//moveToStart 001
TEST_F(test_textedit, moveToStart_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->moveToStart();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    pWindow->deleteLater();
}

//moveToStart 002
TEST_F(test_textedit, moveToStart_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->moveToStart();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//moveToEnd 001
TEST_F(test_textedit, moveToEnd_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->moveToEnd();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    pWindow->deleteLater();
}

//moveToEnd 002
TEST_F(test_textedit, moveToEnd_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->moveToEnd();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//moveToStartOfLine 001
TEST_F(test_textedit, moveToStartOfLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->moveToStartOfLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(!strRet.isEmpty());

    pWindow->deleteLater();
}

//moveToStartOfLine 002
TEST_F(test_textedit, moveToStartOfLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->moveToStartOfLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//moveToEndOfLine 001
TEST_F(test_textedit, moveToEndOfLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->moveToEndOfLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    pWindow->deleteLater();
}

//moveToEndOfLine 002
TEST_F(test_textedit, moveToEndOfLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->moveToEndOfLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//moveToLineIndentation 001
TEST_F(test_textedit, moveToLineIndentation_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world"));
    pWindow->currentWrapper()->textEditor()->moveToLineIndentation();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    ASSERT_TRUE(strRet.isEmpty() && iRet == 0);

    pWindow->deleteLater();
}

//moveToLineIndentation 002
TEST_F(test_textedit, moveToLineIndentation_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->moveToLineIndentation();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().position();
    ASSERT_TRUE(strRet.isEmpty() && iRet == 12);

    pWindow->deleteLater();
}

//nextLine 001
TEST_F(test_textedit, nextLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->nextLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(!strRet.isEmpty());

    pWindow->deleteLater();
}

//nextLine 002
TEST_F(test_textedit, nextLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->nextLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//prevLine 001
TEST_F(test_textedit, prevLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->prevLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(!strRet.isEmpty());

    pWindow->deleteLater();
}

//prevLine 002
TEST_F(test_textedit, prevLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->prevLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());
    ASSERT_TRUE(strRet.isEmpty());

    pWindow->deleteLater();
}

//jumpToLine
TEST_F(test_textedit, jumpToLine)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->jumpToLine(1, true);
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().blockNumber() + 1;
    ASSERT_TRUE(iRet == 1);

    pWindow->deleteLater();
}

//void moveCursorNoBlink(QTextCursor::MoveOperation operation,
//                       QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
TEST_F(test_textedit, moveCursorNoBlink)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->moveCursorNoBlink(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//newline
TEST_F(test_textedit, newline)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->newline();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().blockNumber() + 1;
    ASSERT_TRUE(iRet == 3);

    pWindow->deleteLater();
}

//openNewlineAbove
TEST_F(test_textedit, openNewlineAbove)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->openNewlineAbove();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().blockNumber() + 1;
    ASSERT_TRUE(iRet == 2);

    pWindow->deleteLater();
}

//openNewlineBelow
TEST_F(test_textedit, openNewlineBelow)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    pWindow->currentWrapper()->textEditor()->openNewlineBelow();
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().blockNumber() + 1;
    ASSERT_TRUE(iRet == 3);

    pWindow->deleteLater();
}

//moveLineDownUp 001
TEST_F(test_textedit, moveLineDownUp_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->moveLineDownUp(true);
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().blockNumber() + 1;
    ASSERT_TRUE(iRet == 1);

    pWindow->deleteLater();
}

//moveLineDownUp 002
TEST_F(test_textedit, moveLineDownUp_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->moveLineDownUp(false);
    int iRet = pWindow->currentWrapper()->textEditor()->textCursor().blockNumber() + 1;
    ASSERT_TRUE(iRet == 3);

    pWindow->deleteLater();
}

//scrollLineUp
TEST_F(test_textedit, scrollLineUp)
{
//    Window *pWindow = new Window();
//    pWindow->addBlankTab(QString());
//    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
//                                                          QString("H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw"
//                                                                  "\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H"
//                                                                  "\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no"
//                                                                  "\nr\nl\nd\n."));
//    pWindow->currentWrapper()->textEditor()->scrollLineUp();
//    int iRet = pWindow->currentWrapper()->textEditor()->verticalScrollBar()->value();
//    ASSERT_TRUE(iRet == 26);

//    pWindow->deleteLater();
}

TEST_F(test_textedit, scrollLineDown)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw"
                                                                  "\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H"
                                                                  "\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no"
                                                                  "\nr\nl\nd\n."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->scrollLineDown();
    int iRet = pWindow->currentWrapper()->textEditor()->verticalScrollBar()->value();
    ASSERT_TRUE(iRet == 1);

    pWindow->deleteLater();
}

TEST_F(test_textedit, scrollUp)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw"
                                                                  "\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H"
                                                                  "\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no"
                                                                  "\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\n"));
    int iScrollBarMaxmun = pWindow->currentWrapper()->textEditor()->verticalScrollBar()->maximum();
    pWindow->currentWrapper()->textEditor()->verticalScrollBar()->setValue(iScrollBarMaxmun);
    pWindow->currentWrapper()->textEditor()->scrollUp();
    int iRet = pWindow->currentWrapper()->textEditor()->verticalScrollBar()->value();
    ASSERT_TRUE(iScrollBarMaxmun != iRet);

    pWindow->deleteLater();
}

TEST_F(test_textedit, scrollDown)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw"
                                                                  "\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H"
                                                                  "\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\nl\nl\ne\n\nw\no"
                                                                  "\nr\nl\nd\n.H\nl\nl\ne\n\nw\no\nr\nl\nd\n.H\n"));
    pWindow->currentWrapper()->textEditor()->verticalScrollBar()->setValue(0);
    pWindow->currentWrapper()->textEditor()->scrollDown();
    int iRet = pWindow->currentWrapper()->textEditor()->verticalScrollBar()->value();
    ASSERT_TRUE(iRet != 0);

    pWindow->deleteLater();
}

//duplicateLine 001
TEST_F(test_textedit, duplicateLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iDuplicateLineBeFor = pWindow->currentWrapper()->textEditor()->blockCount();
    pWindow->currentWrapper()->textEditor()->duplicateLine();
    int iDuplicateLineAfter = pWindow->currentWrapper()->textEditor()->blockCount();

    ASSERT_TRUE(iDuplicateLineAfter == iDuplicateLineBeFor + 1);

    pWindow->deleteLater();
}

//duplicateLine 002
TEST_F(test_textedit, duplicateLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    textCursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iDuplicateLineBeFor = pWindow->currentWrapper()->textEditor()->blockCount();
    pWindow->currentWrapper()->textEditor()->duplicateLine();
    int iDuplicateLineAfter = pWindow->currentWrapper()->textEditor()->blockCount();

    ASSERT_TRUE(iDuplicateLineAfter == iDuplicateLineBeFor + 1);

    pWindow->deleteLater();
}

//duplicateLine 003
TEST_F(test_textedit, duplicateLine_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iDuplicateLineBeFor = pWindow->currentWrapper()->textEditor()->blockCount();
    pWindow->currentWrapper()->textEditor()->duplicateLine();
    int iDuplicateLineAfter = pWindow->currentWrapper()->textEditor()->blockCount();

    ASSERT_TRUE(iDuplicateLineAfter == iDuplicateLineBeFor + 1);

    pWindow->deleteLater();
}

//void copyLines 001
TEST_F(test_textedit, copyLines_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->copyLines();
    QString strRet(pClipboard->text());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    pWindow->deleteLater();
}

//void copyLines 002
TEST_F(test_textedit, copyLines_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world\nHolle world\nHolle world"));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->copyLines();
    QString strRet(pClipboard->text());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    pWindow->deleteLater();
}

void stub_push(QUndoCommand *cmd)
{
    Q_UNUSED(cmd);
}

void stub_updateModifyStatus(const QString &path, bool isModified)
{
    Q_UNUSED(path);
    Q_UNUSED(isModified);
}

bool popRightMenu_001_canRedo_stub()
{
    return true;
}

void popRightMenu_001_UpdateBottomBarWordCnt_stub()
{
    return;
}

void popRightMenu_001_writeHistoryRecord_stub()
{
    return;
}

bool popRightMenu_001_isRegisteredFflytekAiassistant_stub()
{
    return false;
}

//popRightMenu 001
TEST_F(test_textedit, popRightMenu_001)
{
#if 0
    EditWrapper *pEditWrapper = new EditWrapper();
    //TextEdit *pTextEdit = new TextEdit();
    Stub isRegisteredFflytekAiassistant_stub;
    isRegisteredFflytekAiassistant_stub.set(ADDR(Window, isRegisteredFflytekAiassistant), popRightMenu_001_isRegisteredFflytekAiassistant_stub);
    Stub UpdateBottomBarWordCnt_stub;
    UpdateBottomBarWordCnt_stub.set(ADDR(EditWrapper, UpdateBottomBarWordCnt), popRightMenu_001_UpdateBottomBarWordCnt_stub);
    Stub writeHistoryRecord_stub;
    writeHistoryRecord_stub.set(ADDR(TextEdit, writeHistoryRecord), popRightMenu_001_writeHistoryRecord_stub);
    //typedef int (*fptr)(QMenu *);
    //fptr fileDialogExec = (fptr)(&QMenu::exec);
    //Stub stub;
    //stub.set((QAction *)(QPoint, QAction *)ADDR(QMenu, exec), popRightMenu_001_writeHistoryRecord_stub);

    pEditWrapper->textEditor()->popRightMenu();
    //pTextEdit->popRightMenu();


//    Window *pWindow = new Window();
//    pWindow->addBlankTab(QString());
//    //pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
//    //                                                      QString("Holle world\nHolle world\nHolle world"));
//    Stub canRedo_stub;
//    canRedo_stub.set(ADDR(QUndoStack, canRedo), popRightMenu_001_canRedo_stub);
//    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    Stub UpdateBottomBarWordCnt_stub;
//    UpdateBottomBarWordCnt_stub.set(ADDR(EditWrapper, UpdateBottomBarWordCnt), popRightMenu_001_UpdateBottomBarWordCnt_stub);
//    Stub writeHistoryRecord_stub;
//    writeHistoryRecord_stub.set(ADDR(TextEdit, writeHistoryRecord), popRightMenu_001_writeHistoryRecord_stub);
//    Stub isRegisteredFflytekAiassistant_stub;
//    isRegisteredFflytekAiassistant_stub.set(ADDR(Window, isRegisteredFflytekAiassistant), popRightMenu_001_isRegisteredFflytekAiassistant_stub);
//    Stub handleTabCloseRequested_stub;
//    handleTabCloseRequested_stub.set(ADDR(Window, handleTabCloseRequested), popRightMenu_001_writeHistoryRecord_stub);
//    Stub indexOf_stub;
//    indexOf_stub.set(ADDR(Tabbar, indexOf), popRightMenu_001_writeHistoryRecord_stub);
//    Stub itextAt_stub;
//    itextAt_stub.set(ADDR(Tabbar, textAt), popRightMenu_001_writeHistoryRecord_stub);
//    Stub slotCanRedoChanged_stub;
//    slotCanRedoChanged_stub.set(ADDR(TextEdit, slotCanRedoChanged), popRightMenu_001_writeHistoryRecord_stub);
//    Stub slotCanUndoChanged_stub;
//    slotCanUndoChanged_stub.set(ADDR(TextEdit, slotCanUndoChanged), popRightMenu_001_writeHistoryRecord_stub);

//    pWindow->currentWrapper()->textEditor()->popRightMenu(QPoint(10, 10));
    //ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    //pEditWrapper->deleteLater();
#endif
}

//void cutlines 001
TEST_F(test_textedit, cutlines_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(), strMsg);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->cutlines();
    QString strRet(pClipboard->text());
    ASSERT_TRUE(strRet.length() == strMsg.length());

    pWindow->deleteLater();
}

//void cutlines 002
TEST_F(test_textedit, cutlines_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(), strMsg);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->cutlines();
    QString strRet(pClipboard->text());
    ASSERT_TRUE(!strRet.compare(QString("Holle world")));

    pWindow->deleteLater();
}

//joinLines 001
TEST_F(test_textedit, joinLines_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRetBefor = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    pWindow->currentWrapper()->textEditor()->joinLines();
    int iRetAfter = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    ASSERT_TRUE(iRetAfter == iRetBefor);

    pWindow->deleteLater();
}

//joinLines 002
TEST_F(test_textedit, joinLines_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRetBefor = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    pWindow->currentWrapper()->textEditor()->joinLines();
    int iRetAfter = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    ASSERT_TRUE(iRetAfter == iRetBefor - 1);

    pWindow->deleteLater();
}

//joinLines 003
TEST_F(test_textedit, joinLines_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRetBefor = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    pWindow->currentWrapper()->textEditor()->joinLines();
    int iRetAfter = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    ASSERT_TRUE(iRetAfter == iRetBefor - 1);

    pWindow->deleteLater();
}

//killLine 001
TEST_F(test_textedit, killLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->killLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRet.compare(strMsg));
    pWindow->deleteLater();
}

//killLine 002
TEST_F(test_textedit, killLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->killLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(strRet.isEmpty());
    pWindow->deleteLater();
}

//killLine 003
TEST_F(test_textedit, killLine_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->killLine();
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(strRetAfter.compare(strRetBefore));
    pWindow->deleteLater();
}

//killCurrentLine 001
TEST_F(test_textedit, killCurrentLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->killCurrentLine();
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRet.compare(strMsg));
    pWindow->deleteLater();
}

//killCurrentLine 002
TEST_F(test_textedit, killCurrentLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    int iRetBefore = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    pWindow->currentWrapper()->textEditor()->killCurrentLine();
    int iRetAfter = pWindow->currentWrapper()->textEditor()->document()->blockCount();

    ASSERT_TRUE(iRetAfter == iRetBefore);
    pWindow->deleteLater();
}

//killBackwardWord 001
TEST_F(test_textedit, killBackwardWord_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->killBackwardWord();
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRet.compare(strMsg));
    pWindow->deleteLater();
}

//killBackwardWord 002
TEST_F(test_textedit, killBackwardWord_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->killBackwardWord();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRet.compare(QString("Holle ")));
    pWindow->deleteLater();
}

//killForwardWord 001
TEST_F(test_textedit, killForwardWord_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->killForwardWord();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRet.compare(strRetBefore));
    pWindow->deleteLater();
}

//killForwardWord 002
TEST_F(test_textedit, killForwardWord_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->killForwardWord();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRet.compare(QString("world")));
    pWindow->deleteLater();
}

//indentText 001
TEST_F(test_textedit, indentText_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRetBefore = pWindow->currentWrapper()->textEditor()->textCursor().position();
    pWindow->currentWrapper()->textEditor()->indentText();
    int iRetAfter = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(iRetAfter != iRetBefore);
    pWindow->deleteLater();
}

//indentText 002
TEST_F(test_textedit, indentText_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRetBefore = pWindow->currentWrapper()->textEditor()->textCursor().position();
    pWindow->currentWrapper()->textEditor()->indentText();
    int iRetAfter = pWindow->currentWrapper()->textEditor()->textCursor().position();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    EXPECT_EQ(iRetAfter , iRetBefore);
    pWindow->deleteLater();
}


//setTabSpaceNumber
TEST_F(test_textedit, setTabSpaceNumber)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);
    pWindow->currentWrapper()->textEditor()->setTabSpaceNumber(5);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_tabSpaceNumber == 5);
    pWindow->deleteLater();
}

//convertWordCase 001
TEST(UT_test_textedit_convertWordCase, UT_test_textedit_convertWordCase_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->convertWordCase(UPPER);
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRet.compare(strMsg.toUpper()));
    pWindow->deleteLater();
}

//convertWordCase 002
TEST(UT_test_textedit_convertWordCase, UT_test_textedit_convertWordCase_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->convertWordCase(LOWER);
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRet.compare(strMsg.toLower()));
    pWindow->deleteLater();
}

//convertWordCase 003
TEST(UT_test_textedit_convertWordCase, UT_test_textedit_convertWordCase_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    pWindow->currentWrapper()->textEditor()->convertWordCase(CAPITALIZE);
    QString strRet(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(strRet.compare(strMsg));
    pWindow->deleteLater();
}

//convertWordCase 004
TEST(UT_test_textedit_convertWordCase, UT_test_textedit_convertWordCase_004)
{
//    Window *pWindow = new Window();
//    pWindow->addBlankTab(QString());
//    QString strMsg("Holle world\nHolle world\nHolle world");
//    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

//    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    pWindow->currentWrapper()->textEditor()->convertWordCase(UPPER);
//    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());

//    ASSERT_TRUE(!strRet.compare(QString("WORLD")));
//    pWindow->deleteLater();
}

//convertWordCase 005
TEST(UT_test_textedit_convertWordCase, UT_test_textedit_convertWordCase_005)
{
//    Window *pWindow = new Window();
//    pWindow->addBlankTab(QString());
//    QString strMsg("Holle world\nHolle world\nHolle WORLD");
//    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

//    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    pWindow->currentWrapper()->textEditor()->convertWordCase(LOWER);
//    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());

//    ASSERT_TRUE(!strRet.compare(QString("world")));
//    pWindow->deleteLater();
}

//convertWordCase 006
TEST(UT_test_textedit_convertWordCase, UT_test_textedit_convertWordCase_006)
{
//    Window *pWindow = new Window();
//    pWindow->addBlankTab(QString());
//    QString strMsg("Holle world\nHolle world\nHolle world");
//    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

//    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    pWindow->currentWrapper()->textEditor()->convertWordCase(CAPITALIZE);
//    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
//    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
//    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
//    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().selectedText());

//    ASSERT_TRUE(!strRet.compare(QString("World")));
//    pWindow->deleteLater();
}

//QString capitalizeText(QString text);
TEST_F(test_textedit, capitalizeText)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strRet(pWindow->currentWrapper()->textEditor()->capitalizeText(QString("world")));

    ASSERT_TRUE(!strRet.compare(QString("World")));
    pWindow->deleteLater();
}

//void keepCurrentLineAtCenter();
TEST_F(test_textedit, keepCurrentLineAtCenter)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("H\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\n"
                   "H\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\n"
                   "H\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->keepCurrentLineAtCenter();
    QScrollBar *pScrollBar = pWindow->currentWrapper()->textEditor()->verticalScrollBar();
    int iRet = pScrollBar->value();

    ASSERT_TRUE(iRet != 0);
    pWindow->deleteLater();
}

//scrollToLine
TEST_F(test_textedit, scrollToLine)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("H\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\n"
                   "H\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\n"
                   "H\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\nH\no\nl\nl\ne\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);
    pWindow->currentWrapper()->textEditor()->scrollToLine(10, 5, 5);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_restoreRow == 5);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_restoreColumn == 5);
    pWindow->deleteLater();
}

//void setLineWrapMode(bool enable);
TEST(UT_test_textedit_setLineWrapMode, UT_test_textedit_setLineWrapMode_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setLineWrapMode(QPlainTextEdit::NoWrap);
    pWindow->currentWrapper()->textEditor()->setLineWrapMode(true);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->lineWrapMode() == QPlainTextEdit::WidgetWidth);
    pWindow->deleteLater();
}

//void setFontFamily(QString fontName);
TEST(UT_test_textedit_setFontFamily, UT_test_textedit_setFontFamily_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setFontFamily(QString("MT Extra"));
    QFont font(pWindow->currentWrapper()->textEditor()->font());
    QString strRet(font.family());
    ASSERT_TRUE(!strRet.compare(QString("MT Extra")));
    pWindow->deleteLater();
}

//void setFontSize(int fontSize);
TEST(UT_test_textedit_setFontSize, UT_test_textedit_setFontSize_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setFontSize(15);
    QFont font(pWindow->currentWrapper()->textEditor()->font());
    int iRet = font.pointSize();
    ASSERT_TRUE(iRet == 15);
    pWindow->deleteLater();
}

//void updateFont();
TEST(UT_test_textedit_updateFont, UT_test_textedit_updateFont_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->updateFont();
    QFont font(pWindow->currentWrapper()->textEditor()->document()->defaultFont());
    bool bRet = font.fixedPitch();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//replaceAll 001
TEST(UT_test_textedit_replaceAll, UT_test_textedit_replaceAll_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Helle");
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    pWindow->currentWrapper()->textEditor()->replaceAll(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));
    pWindow->deleteLater();
}

//replaceAll 002
TEST(UT_test_textedit_replaceAll, UT_test_textedit_replaceAll_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Helle");
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->replaceAll(QString(), strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));

    pWindow->deleteLater();
}

//replaceAll 003
TEST(UT_test_textedit_replaceAll, UT_test_textedit_replaceAll_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Holle");
    pWindow->currentWrapper()->textEditor()->replaceAll(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRetAfter.compare(QString("Holle Holle\nHolle Holle")));

    pWindow->currentWrapper()->textEditor()->replaceAll(QString("holle"), QString("World"));
    strRetAfter = QString(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(!strRetAfter.compare(QString("World World\nWorld World")));

    pWindow->deleteLater();
}

//replaceAll check case sensitive
TEST(UT_test_textedit_replaceAll, UT_test_textedit_replaceAll_CaseSensitive)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    auto *textEdit = pWindow->currentWrapper()->textEditor();

    // NotChange
    textEdit->replaceAll(QString("hello"), QString("world"), Qt::CaseSensitive);
    ASSERT_TRUE(!textEdit->toPlainText().compare(strMsg));

    textEdit->replaceAll(QString("hello"), QString("World"), Qt::CaseInsensitive);
    ASSERT_TRUE(!textEdit->toPlainText().compare(QString("World world\nWorld world")));

    textEdit->replaceAll(QString("world"), QString("World"), Qt::CaseInsensitive);
    ASSERT_TRUE(!textEdit->toPlainText().compare(QString("World World\nWorld World")));

    pWindow->deleteLater();
}

//replaceNext 001
TEST(UT_test_textedit_replaceNext, UT_test_textedit_replaceNext_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Holle");
    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->replaceNext(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));
    pWindow->deleteLater();
}

//replaceNext 002
TEST(UT_test_textedit_replaceNext, UT_test_textedit_replaceNext_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Holle");
    pWindow->currentWrapper()->textEditor()->m_isSelectAll = true;
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->replaceNext(QString(), strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->toPlainText());

    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));
    pWindow->deleteLater();
}

//replaceNext 003
TEST(UT_test_textedit_replaceNext, UT_test_textedit_replaceNext_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strReplaceText("world");
    QString strWithText("Holle");
    pWindow->currentWrapper()->textEditor()->m_findHighlightSelection.cursor = textCursor;
    pWindow->currentWrapper()->textEditor()->m_cursorStart = 1;
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->replaceNext(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRetAfter.compare(QString("Holle world")));
    pWindow->deleteLater();
}

//replaceNext 004
TEST(UT_test_textedit_replaceNext, UT_test_textedit_replaceNext_004)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strReplaceText("world");
    QString strWithText("Holle");
    pWindow->currentWrapper()->textEditor()->m_findHighlightSelection.cursor = textCursor;
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->replaceNext(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRetAfter.compare(QString("Holle world")));
    pWindow->deleteLater();
}

//replaceNext check sensitive
TEST(UT_test_textedit_replaceNext, UT_test_textedit_replaceNext_CaseSensitive)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    auto *textEdit = pWindow->currentWrapper()->textEditor();
    QTextCursor textCursor = textEdit->textCursor();
    textEdit->insertTextEx(textCursor, strMsg);

    textCursor = textEdit->textCursor();
    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    textEdit->setTextCursor(textCursor);
    textEdit->m_findHighlightSelection.cursor = textCursor;

    QString strReplaceText("hello");
    QString strWithText("World");
    QString strRetBefore(textEdit->textCursor().block().text());

    textEdit->replaceNext(strReplaceText, strWithText, Qt::CaseSensitive);
    QString strRetAfter(textEdit->textCursor().block().text());
    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));

    textEdit->setTextCursor(textCursor);
    textEdit->m_findHighlightSelection.cursor = textCursor;
    textEdit->replaceNext(strReplaceText, strWithText, Qt::CaseInsensitive);
    ASSERT_TRUE(!textEdit->toPlainText().compare("World world\nHello world"));

    pWindow->deleteLater();
}

//replaceRest 001
TEST(UT_test_textedit_replaceRest, UT_test_textedit_replaceRest_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world Holle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Holle");
    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->replaceRest(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRetAfter.compare(QString("Holle world Holle world")));
    pWindow->deleteLater();
}

//replaceRest 002
TEST(UT_test_textedit_replaceRest, UT_test_textedit_replaceRest_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Holle world\nHolle world Holle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strReplaceText("world");
    QString strWithText("Holle");
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->replaceRest(QString(), strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRetAfter.compare(QString("Holle world Holle world")));
    pWindow->deleteLater();
}

//replaceRest 003
TEST(UT_test_textedit_replaceRest, UT_test_textedit_replaceRest_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world Helle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strReplaceText("world");
    QString strWithText("Helle");
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->replaceRest(strReplaceText, strWithText);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRetAfter.compare(QString("Helle Helle Helle Helle")));
    pWindow->deleteLater();
}

//replaceRest check case sensitive
TEST(UT_test_textedit_replaceRest, UT_test_textedit_replaceRest_CaseSensitive)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world Hello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strReplaceText("World");
    QString strWithText("Hello");
    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->replaceRest(strReplaceText, strWithText, Qt::CaseSensitive);
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));

    pWindow->currentWrapper()->textEditor()->replaceRest(strReplaceText, strWithText, Qt::CaseInsensitive);
    strRetAfter = QString(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    ASSERT_TRUE(!strRetAfter.compare("Hello Hello Hello Hello"));

    pWindow->deleteLater();
}


//beforeReplace
TEST(UT_test_textedit_beforeReplace, UT_test_textedit_beforeReplace_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strRetBefore(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    pWindow->currentWrapper()->textEditor()->beforeReplace(QString());
    QString strRetAfter(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRetAfter.compare(strRetBefore));
    pWindow->deleteLater();
}

//findKeywordForward 001
TEST(UT_test_textedit_findKeywordForward, UT_test_textedit_findKeywordForward_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    bool bRet = pWindow->currentWrapper()->textEditor()->findKeywordForward(QString("Helle"));

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//findKeywordForward 002
TEST(UT_test_textedit_findKeywordForward, UT_test_textedit_findKeywordForward_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->findKeywordForward(QString("Helle"));

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//removeKeywords
TEST(UT_test_textedit_removeKeywords, UT_test_textedit_removeKeywords_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->removeKeywords();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_findMatchSelections.isEmpty());
    pWindow->deleteLater();
}

//highlightKeyword
TEST(UT_test_textedit_highlightKeyword, UT_test_textedit_highlightKeyword_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->highlightKeyword(QString("world"), 0);

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//highlightKeyword case sensitive
TEST(UT_test_textedit_highlightKeyword, UT_test_textedit_highlightKeyword_CaseSensitive)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->highlightKeyword(QString("World"), 0);
    ASSERT_TRUE(bRet);

    bRet = pWindow->currentWrapper()->textEditor()->highlightKeyword(QString("hello"), 0);
    ASSERT_TRUE(bRet);

    bRet = pWindow->currentWrapper()->textEditor()->highlightKeyword(QString("World"), 0, Qt::CaseSensitive);
    ASSERT_FALSE(bRet);

    pWindow->deleteLater();
}

//highlightKeywordInView
TEST(UT_test_textedit_highlightKeywordInView, UT_test_textedit_highlightKeywordInView_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->highlightKeywordInView(QString("world"));

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//clearFindMatchSelections
TEST(UT_test_textedit_clearFindMatchSelections, UT_test_textedit_clearFindMatchSelections_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->clearFindMatchSelections();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_findMatchSelections.isEmpty());
    pWindow->deleteLater();
}

//updateCursorKeywordSelection
TEST(UT_test_textedit_updateCursorKeywordSelection, UT_test_textedit_updateCursorKeywordSelection_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->updateCursorKeywordSelection(QString(), true);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_findMatchSelections.isEmpty());
    pWindow->deleteLater();
}

//updateHighlightLineSelection
TEST(UT_test_textedit_updateHighlightLineSelection, UT_test_textedit_updateHighlightLineSelection_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_gestureAction = TextEdit::GA_slide;
    pWindow->currentWrapper()->textEditor()->updateHighlightLineSelection();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_findMatchSelections.isEmpty());
    pWindow->deleteLater();
}

//updateKeywordSelections 001
TEST(UT_test_textedit_updateKeywordSelections, UT_test_textedit_updateKeywordSelections_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelections(QString("smile"), charFormat, listExtraSelection);
    ASSERT_TRUE(bRet == false);

    pWindow->currentWrapper()->textEditor()->defaultCaseSensitive = Qt::CaseSensitive;
    bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelections(QString("World"), charFormat, listExtraSelection);
    ASSERT_TRUE(bRet == false);

    pWindow->deleteLater();
}

//updateKeywordSelections 002
TEST(UT_test_textedit_updateKeywordSelections, UT_test_textedit_updateKeywordSelections_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelections(QString("world"), charFormat, listExtraSelection);
    ASSERT_TRUE(bRet == true);

    bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelections(QString("World"), charFormat, listExtraSelection);
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//updateKeywordSelections 003
TEST(UT_test_textedit_updateKeywordSelections, UT_test_textedit_updateKeywordSelections_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelections(QString(), charFormat, listExtraSelection);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//updateKeywordSelectionsInView 001
TEST(UT_test_textedit_updateKeywordSelectionsInView, UT_test_textedit_updateKeywordSelectionsInView_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelectionsInView(QString("smile"), charFormat, &listExtraSelection);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//updateKeywordSelectionsInView 002
TEST(UT_test_textedit_updateKeywordSelectionsInView, UT_test_textedit_updateKeywordSelectionsInView_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelectionsInView(QString("smile"), charFormat, &listExtraSelection);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//updateKeywordSelectionsInView 003
TEST(UT_test_textedit_updateKeywordSelectionsInView, UT_test_textedit_updateKeywordSelectionsInView_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelectionsInView(QString("n"), charFormat, &listExtraSelection);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//updateKeywordSelectionsInView 004
TEST(UT_test_textedit_updateKeywordSelectionsInView, UT_test_textedit_updateKeywordSelectionsInView_004)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.format = charFormat;
    QList<QTextEdit::ExtraSelection> listExtraSelection;
    listExtraSelection.append(extraSelection);
    bool bRet = pWindow->currentWrapper()->textEditor()->updateKeywordSelectionsInView(QString(), charFormat, &listExtraSelection);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//searchKeywordSeletion 001
TEST(UT_test_textedit_searchKeywordSeletion, UT_test_textedit_searchKeywordSeletion_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    bool bRet = pWindow->currentWrapper()->textEditor()->searchKeywordSeletion(QString(), textCursor, true);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//searchKeywordSeletion 002
TEST(UT_test_textedit_searchKeywordSeletion, UT_test_textedit_searchKeywordSeletion_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    bool bRet = pWindow->currentWrapper()->textEditor()->searchKeywordSeletion(QString("world"), textCursor, true);

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//searchKeywordSeletion 003
TEST(UT_test_textedit_searchKeywordSeletion, UT_test_textedit_searchKeywordSeletion_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->searchKeywordSeletion(QString("world"), textCursor, false);

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//renderAllSelections
TEST(UT_test_textedit_renderAllSelections, UT_test_textedit_renderAllSelections_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"));
    pWindow->currentWrapper()->textEditor()->markKeywordInView(QString("world"), QString("red"));
    pWindow->currentWrapper()->textEditor()->m_HightlightYes = true;
    pWindow->currentWrapper()->textEditor()->renderAllSelections();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_mapKeywordMarkSelections.isEmpty());
    pWindow->deleteLater();
}

//clearMarkOperationForCursor
TEST(UT_test_textedit_clearMarkOperationForCursor, UT_test_textedit_clearMarkOperationForCursor_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkAllLine(true, QString("red"));
    QTextCursor textCursorTemp = pWindow->currentWrapper()->textEditor()->m_markOperations.at(0).first.cursor;
    bool bRet = pWindow->currentWrapper()->textEditor()->clearMarkOperationForCursor(textCursorTemp);

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//clearMarksForTextCursor 001
TEST(UT_test_textedit_clearMarksForTextCursor, UT_test_textedit_clearMarksForTextCursor_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"));
    bool bRet = pWindow->currentWrapper()->textEditor()->clearMarksForTextCursor();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//clearMarksForTextCursor 002
TEST(UT_test_textedit_clearMarksForTextCursor, UT_test_textedit_clearMarksForTextCursor_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"));
    bool bRet = pWindow->currentWrapper()->textEditor()->clearMarksForTextCursor();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//markAllKeywordInView 001
TEST(UT_test_textedit_clearMarksForTextCursor, UT_test_textedit_markAllKeywordInView_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->markAllKeywordInView();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty());
    pWindow->deleteLater();
}

//markAllKeywordInView 002
TEST(UT_test_textedit_clearMarksForTextCursor, UT_test_textedit_markAllKeywordInView_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"));
    pWindow->currentWrapper()->textEditor()->m_markOperations.begin()->first.type = TextEdit::MarkAllMatch;
    pWindow->currentWrapper()->textEditor()->markAllKeywordInView();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty());
    pWindow->deleteLater();
}

//markAllKeywordInView 003
TEST(UT_test_textedit_clearMarksForTextCursor, UT_test_textedit_markAllKeywordInView_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"));
    pWindow->currentWrapper()->textEditor()->m_markOperations.begin()->first.type = TextEdit::MarkAll;
    pWindow->currentWrapper()->textEditor()->markAllKeywordInView();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty());
    pWindow->deleteLater();
}

//markKeywordInView 001
TEST(UT_test_textedit_markKeywordInView, UT_test_textedit_markKeywordInView_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->markKeywordInView(QString(), QString("red"));

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//markKeywordInView 002
TEST(UT_test_textedit_markKeywordInView, UT_test_textedit_markKeywordInView_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->markKeywordInView(QString("world"), QString("red"));

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//markAllInView 001
TEST(UT_test_textedit_markAllInView, UT_test_textedit_markAllInView_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->markAllInView(QString("red"));

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_mapKeywordMarkSelections.isEmpty());
    pWindow->deleteLater();
}

//toggleMarkSelections 001
TEST(UT_test_textedit_toggleMarkSelections, UT_test_textedit_toggleMarkSelections_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->toggleMarkSelections();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty());
    pWindow->deleteLater();
}

//updateMarkAllSelectColor 001
TEST(UT_test_textedit_updateMarkAllSelectColor, UT_test_textedit_updateMarkAllSelectColor_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->updateMarkAllSelectColor();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_mapKeywordMarkSelections.isEmpty());
    pWindow->deleteLater();
}

// lineNumberAreaPaintEvent 001
TEST(UT_test_textedit_lineNumberAreaPaintEvent, UT_test_textedit_lineNumberAreaPaintEvent_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n"
                   "H\ne\nl\nl\ne\n w\no\nr\nl\nd\nH\ne\nl\nl\ne\n w\no\nr\nl\nd\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::DarkType);
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->lineNumberAreaPaintEvent(pPaintEvent);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_lineNumbersColor.alphaF() == 0.2);
    pWindow->deleteLater();
}

//lineNumberAreaPaintEvent 002
TEST(UT_test_textedit_lineNumberAreaPaintEvent, UT_test_textedit_lineNumberAreaPaintEvent_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::LightType);
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->lineNumberAreaPaintEvent(pPaintEvent);

    EXPECT_NE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pLineNumberArea, nullptr);
    pWindow->deleteLater();
}

//codeFLodAreaPaintEvent 001
TEST(UT_test_textedit_codeFLodAreaPaintEvent, UT_test_textedit_codeFLodAreaPaintEvent_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\n{\n}\n}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::DarkType);
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->codeFLodAreaPaintEvent(pPaintEvent);

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listFlodIconPos.isEmpty());
    pWindow->deleteLater();
}

//codeFLodAreaPaintEvent 002
TEST(UT_test_textedit_codeFLodAreaPaintEvent, UT_test_textedit_codeFLodAreaPaintEvent_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\n{\n}\n}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::LightType);
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->codeFLodAreaPaintEvent(pPaintEvent);

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listFlodIconPos.isEmpty());
    pWindow->deleteLater();
}

//setBookmarkFlagVisable 001
TEST(UT_test_textedit_setBookmarkFlagVisable, UT_test_textedit_setBookmarkFlagVisable_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setBookmarkFlagVisable(true, false);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pIsShowBookmarkArea == true);
    pWindow->deleteLater();
}

//setBookmarkFlagVisable 002
TEST(UT_test_textedit_setBookmarkFlagVisable, UT_test_textedit_setBookmarkFlagVisable_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setBookmarkFlagVisable(false, false);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pIsShowBookmarkArea == false);
    pWindow->deleteLater();
}

//setCodeFlodFlagVisable 001
TEST(UT_test_textedit_setCodeFlodFlagVisable, UT_test_textedit_setCodeFlodFlagVisable_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setCodeFlodFlagVisable(true, false);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pIsShowCodeFoldArea == true);
    pWindow->deleteLater();
}

//setCodeFlodFlagVisable 002
TEST(UT_test_textedit_setCodeFlodFlagVisable, UT_test_textedit_setCodeFlodFlagVisable_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setCodeFlodFlagVisable(false, false);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pIsShowCodeFoldArea == false);
    pWindow->deleteLater();
}

//setTheme 001
TEST(UT_test_textedit_setTheme, UT_test_textedit_setTheme_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strThemeFilePath("./../themes/deepin.theme");
    pWindow->currentWrapper()->textEditor()->setTheme(strThemeFilePath);
    QString strBgColorName(pWindow->currentWrapper()->textEditor()->m_backgroundColor.name());

    ASSERT_TRUE(!strBgColorName.compare(QString("#000000")));
    pWindow->deleteLater();
}

//void removeHighlightWordUnderCursor() 001
TEST(UT_test_textedit_removeHighlightWordUnderCursor, UT_test_textedit_removeHighlightWordUnderCursor_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->removeHighlightWordUnderCursor();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_nBookMarkHoverLine == -1);
    pWindow->deleteLater();
}

//void setSettings(Settings *settings) 001
TEST(UT_test_textedit_setSettings, UT_test_textedit_setSettings_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setSettings(pWindow->currentWrapper()->textEditor()->m_settings);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_settings != nullptr);
    pWindow->deleteLater();
}

//void copySelectedText() 001
TEST(UT_test_textedit_copySelectedText, UT_test_textedit_copySelectedText_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QTextEdit::ExtraSelection extraSelection;
    extraSelection.cursor = textCursor;
    pWindow->currentWrapper()->textEditor()->m_altModSelections << extraSelection;
    pWindow->currentWrapper()->textEditor()->m_bIsAltMod = true;
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->copySelectedText();

    QString strRet(pClipboard->text());
    ASSERT_TRUE(!strRet.compare(QString("Helle world")));
    pWindow->deleteLater();
}

//void copySelectedText() 002
TEST(UT_test_textedit_copySelectedText, UT_test_textedit_copySelectedText_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->copySelectedText();

    QString strRet(pClipboard->text());
    ASSERT_TRUE(!strRet.compare(QString("Helle world")));
    pWindow->deleteLater();
}

//void copySelectedText() 003
TEST(UT_test_textedit_copySelectedText, UT_test_textedit_copySelectedText_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->copySelectedText();

    QString strRet(pClipboard->text());
    ASSERT_TRUE(strRet.isEmpty());
    pWindow->deleteLater();
}

//void cutSelectedText() 001
TEST(UT_test_textedit_cutSelectedText, UT_test_textedit_cutSelectedText_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->cutSelectedText();

    QString strRet1(pClipboard->text());
    QString strRet2(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(!strRet1.compare(QString("Helle world")) && !strRet2.compare(QString("Helle world\n")));
    pWindow->deleteLater();
}

TEST(UT_test_textedit_cutSelectedText, cutSelectedText_withMultiByteText_passed)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    // 多字节字符编码 CJK
    // 𢝐𢝑𢝒𢝓𢝔𢝕𢝖𢝗𢝘𢝙\n𢝚𢝛𢝜𢝝𢝞𢝟𢝠𢝡𢝢𢝣
    QString strMsg("\360\242\235\220\360\242\235\221\360\242\235\222\360\242\235\223\360\242\235\224"
                   "\360\242\235\225\360\242\235\226\360\242\235\227\360\242\235\230\360\242\235\231\n"
                   "\360\242\235\232\360\242\235\233\360\242\235\234\360\242\235\235\360\242\235\236"
                   "\360\242\235\237\360\242\235\240\360\242\235\241\360\242\235\242\360\242\235\243");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.setPosition(0);
    textCursor.setPosition(6, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->cutSelectedText();

    QString strRet1(pClipboard->text());
    QString strRet2(pWindow->currentWrapper()->textEditor()->toPlainText());
    EXPECT_EQ(strRet1, strMsg.left(6));
    EXPECT_EQ(strRet2, strMsg.mid(6));
    pWindow->deleteLater();
}

//void pasteText() 001
TEST(UT_test_textedit_pasteText, UT_test_textedit_pasteText_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->copySelectedText();
    textCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->pasteText();
    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());

    ASSERT_TRUE(!strRet.compare(QString("Helle worldHelle world")));
    pWindow->deleteLater();
}

//void setMark() 001
TEST(UT_test_textedit_setMark, UT_test_textedit_setMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->setMark();
    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void setMark() 002
TEST(UT_test_textedit_setMark, UT_test_textedit_setMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->setMark();

    bool bRet = pWindow->currentWrapper()->textEditor()->m_cursorMark;
    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void setMark() 003
TEST(UT_test_textedit_setMark, UT_test_textedit_setMark_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    pWindow->currentWrapper()->textEditor()->setMark();

    bool bRet = pWindow->currentWrapper()->textEditor()->m_cursorMark;
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void unsetMark() 001
TEST(UT_test_textedit_unsetMark, UT_test_textedit_unsetMark_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    pWindow->currentWrapper()->textEditor()->unsetMark();

    bool bRet = pWindow->currentWrapper()->textEditor()->m_cursorMark;
    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//bool tryUnsetMark() 001
TEST(UT_test_textedit_tryUnsetMark, UT_test_textedit_tryUnsetMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = true;
    bool bRet = pWindow->currentWrapper()->textEditor()->tryUnsetMark();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//bool tryUnsetMark() 002
TEST(UT_test_textedit_tryUnsetMark, UT_test_textedit_tryUnsetMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMark = false;
    bool bRet = pWindow->currentWrapper()->textEditor()->tryUnsetMark();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void exchangeMark() 001
TEST(UT_test_textedit_exchangeMark, UT_test_textedit_exchangeMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRetBefore = textCursor.position();
    int iSelectStar = textCursor.selectionStart();
    int iSelectEnd = textCursor.selectionEnd();
    pWindow->currentWrapper()->textEditor()->exchangeMark();
    int iRetAfter = textCursor.position();

    ASSERT_TRUE(iRetAfter == iRetBefore);
    pWindow->deleteLater();
}

//void exchangeMark() 002
TEST(UT_test_textedit_exchangeMark, UT_test_textedit_exchangeMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    textCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRetBefore = textCursor.position();
    pWindow->currentWrapper()->textEditor()->exchangeMark();
    int iRetAfter = textCursor.position();

    ASSERT_TRUE(iRetAfter == iRetBefore);
    pWindow->deleteLater();
}

//void saveMarkStatus() 001
TEST(UT_test_textedit_saveMarkStatus, UT_test_textedit_saveMarkStatus_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet1 = pWindow->currentWrapper()->textEditor()->m_cursorMarkPosition;
    pWindow->currentWrapper()->textEditor()->setMark();
    pWindow->currentWrapper()->textEditor()->saveMarkStatus();

    int iRet = pWindow->currentWrapper()->textEditor()->m_cursorMarkPosition;
    ASSERT_TRUE(iRet == 23);
    pWindow->deleteLater();
}

//void restoreMarkStatus() 001
TEST(UT_test_textedit_restoreMarkStatus, UT_test_textedit_restoreMarkStatus_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_cursorMarkStatus = true;
    pWindow->currentWrapper()->textEditor()->restoreMarkStatus();

    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

QString UT_test_textedit_completionWord_001_stub()
{
    return QString("wo");
}

//void completionWord(QString word) 001
TEST(UT_test_textedit_completionWord, UT_test_textedit_completionWord_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_getWordAtCursor;
    stub_getWordAtCursor.set(ADDR(TextEdit, getWordAtCursor), UT_test_textedit_completionWord_001_stub);
    pWindow->currentWrapper()->textEditor()->completionWord("world");

    QString strRet(pWindow->currentWrapper()->textEditor()->textCursor().block().text());
    ASSERT_TRUE(!strRet.compare(QString("Helle worldrld")));
    pWindow->deleteLater();
}

int UT_test_textedit_getWordAtMouse_001_stub()
{
    return 0;
}

//QString getWordAtMouse() 001
TEST(UT_test_textedit_getWordAtMouse, UT_test_textedit_getWordAtMouse_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_characterCount;
    stub_characterCount.set(ADDR(TextEdit, characterCount), UT_test_textedit_getWordAtMouse_001_stub);
    QString strRet = pWindow->currentWrapper()->textEditor()->getWordAtMouse();

    ASSERT_TRUE(strRet.isEmpty());
    pWindow->deleteLater();
}

//QString getWordAtMouse() 002
TEST(UT_test_textedit_getWordAtMouse, UT_test_textedit_getWordAtMouse_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strRet = pWindow->currentWrapper()->textEditor()->getWordAtMouse();

    ASSERT_TRUE(strRet.isEmpty());
    pWindow->deleteLater();
}

//QString getWordAtCursor() 001
TEST(UT_test_textedit_getWordAtCursore, UT_test_textedit_getWordAtCursor_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_characterCount;
    stub_characterCount.set(ADDR(TextEdit, characterCount), UT_test_textedit_getWordAtMouse_001_stub);
    QString strRet = pWindow->currentWrapper()->textEditor()->getWordAtCursor();

    ASSERT_TRUE(strRet.isEmpty());
    pWindow->deleteLater();
}

//QString getWordAtCursor() 002
TEST(UT_test_textedit_getWordAtCursore, UT_test_textedit_getWordAtCursor_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle wor-ld");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QString strRet = pWindow->currentWrapper()->textEditor()->getWordAtCursor();

    ASSERT_TRUE(!strRet.compare(QString("ld")));
    pWindow->deleteLater();
}

//void toggleReadOnlyMode() 001
TEST(UT_test_textedit_toggleReadOnlyMode, UT_test_textedit_toggleReadOnlyMode_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    pWindow->currentWrapper()->textEditor()->m_cursorMode = TextEdit::Overwrite;
    pWindow->currentWrapper()->textEditor()->toggleReadOnlyMode();

    bool bRet = pWindow->currentWrapper()->textEditor()->isReadOnly();
    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void toggleReadOnlyMode() 002
TEST(UT_test_textedit_toggleReadOnlyMode, UT_test_textedit_toggleReadOnlyMode_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    pWindow->currentWrapper()->textEditor()->m_cursorMode = TextEdit::Readonly;
    pWindow->currentWrapper()->textEditor()->toggleReadOnlyMode();

    bool bRet = pWindow->currentWrapper()->textEditor()->isReadOnly();
    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void toggleReadOnlyMode() 003
TEST(UT_test_textedit_toggleReadOnlyMode, UT_test_textedit_toggleReadOnlyMode_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = false;
    pWindow->currentWrapper()->textEditor()->toggleReadOnlyMode();

    bool bRet = pWindow->currentWrapper()->textEditor()->isReadOnly();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void toggleComment(bool bValue) 001
TEST(UT_test_textedit_toggleComment, UT_test_textedit_toggleComment_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    pWindow->currentWrapper()->textEditor()->toggleComment(true);

    bool bRet = pWindow->currentWrapper()->textEditor()->m_readOnlyMode;
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

bool UT_test_textedit_toggleComment_002_stub()
{
    return true;
}

//void toggleComment(bool bValue) 002
TEST(UT_test_textedit_toggleComment, UT_test_textedit_toggleComment_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_isValid;
    stub_isValid.set(ADDR(Comment::CommentDefinition, isValid), UT_test_textedit_toggleComment_002_stub);
    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = false;
    pWindow->currentWrapper()->textEditor()->toggleComment(true);

    bool bRet = pWindow->currentWrapper()->textEditor()->m_commentDefinition.isValid();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

bool UT_test_textedit_toggleComment_003_stub()
{
    return false;
}

void UT_test_textedit_toggleComment_003_setComment_stub()
{
    return;
}

//void toggleComment(bool bValue) 003
TEST(UT_test_textedit_toggleComment, UT_test_textedit_toggleComment_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_isValid;
    stub_isValid.set(ADDR(Comment::CommentDefinition, isValid), UT_test_textedit_toggleComment_002_stub);
    Stub stub_isEmpty;
    stub_isEmpty.set(ADDR(QString, isEmpty), UT_test_textedit_toggleComment_003_stub);
    Stub stub_setComment;
    stub_setComment.set(ADDR(TextEdit, setComment), UT_test_textedit_toggleComment_003_setComment_stub);
    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = false;
    pWindow->currentWrapper()->textEditor()->m_commentDefinition.multiLineStart = QString("Helle  \tworld\nHelle world");
    pWindow->currentWrapper()->textEditor()->m_commentDefinition.singleLine = QString("Helle  \tworld\nHelle world");
    pWindow->currentWrapper()->textEditor()->toggleComment(true);

    bool bRet = pWindow->currentWrapper()->textEditor()->m_commentDefinition.isValid();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

void UT_test_textedit_toggleComment_004_removeComment_stub()
{
    return;
}

//void toggleComment(bool bValue) 004
TEST(UT_test_textedit_toggleComment, UT_test_textedit_toggleComment_004)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_isValid;
    stub_isValid.set(ADDR(Comment::CommentDefinition, isValid), UT_test_textedit_toggleComment_002_stub);
    Stub stub_isEmpty;
    stub_isEmpty.set(ADDR(QString, isEmpty), UT_test_textedit_toggleComment_003_stub);
    Stub stub_removeComment;
    stub_removeComment.set(ADDR(TextEdit, removeComment), UT_test_textedit_toggleComment_004_removeComment_stub);
    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = false;
    pWindow->currentWrapper()->textEditor()->m_commentDefinition.multiLineStart = QString("Helle  \tworld\nHelle world");
    pWindow->currentWrapper()->textEditor()->m_commentDefinition.singleLine = QString("Helle  \tworld\nHelle world");
    pWindow->currentWrapper()->textEditor()->toggleComment(false);

    bool bRet = pWindow->currentWrapper()->textEditor()->m_commentDefinition.isValid();
    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

int UT_test_textedit_getNextWordPosition_001_characterCount_stub()
{
    return 0;
}

//getNextWordPosition 001
TEST(UT_test_textedit_getNextWordPosition, UT_test_textedit_getNextWordPosition_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_characterCount;
    stub_characterCount.set(ADDR(TextEdit, characterCount), UT_test_textedit_getNextWordPosition_001_characterCount_stub);
    int iRet = pWindow->currentWrapper()->textEditor()->getNextWordPosition(textCursor, QTextCursor::MoveMode::MoveAnchor);

    ASSERT_TRUE(iRet == 0);
    pWindow->deleteLater();
}

//getNextWordPosition 002
TEST(UT_test_textedit_getNextWordPosition, UT_test_textedit_getNextWordPosition_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    int iRet = pWindow->currentWrapper()->textEditor()->getNextWordPosition(textCursor, QTextCursor::MoveMode::MoveAnchor);

    ASSERT_TRUE(iRet == 17);
    pWindow->deleteLater();
}

int UT_test_textedit_getPrevWordPosition_001_characterCount_stub()
{
    return 0;
}

//getPrevWordPosition 001
TEST(UT_test_textedit_getPrevWordPosition, UT_test_textedit_getPrevWordPosition_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_characterCount;
    stub_characterCount.set(ADDR(TextEdit, characterCount), UT_test_textedit_getPrevWordPosition_001_characterCount_stub);
    int iRet = pWindow->currentWrapper()->textEditor()->getPrevWordPosition(textCursor, QTextCursor::MoveMode::MoveAnchor);

    ASSERT_TRUE(iRet == 0);
    pWindow->deleteLater();
}

//getPrevWordPosition 002
TEST(UT_test_textedit_getPrevWordPosition, UT_test_textedit_getPrevWordPosition_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->getPrevWordPosition(textCursor, QTextCursor::MoveMode::MoveAnchor);

    ASSERT_TRUE(iRet == 17);
    pWindow->deleteLater();
}

//bool atWordSeparator(int position);
TEST(UT_test_textedit_atWordSeparator, UT_test_textedit_atWordSeparator_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->atWordSeparator(10);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void showCursorBlink();
TEST(UT_test_textedit_showCursorBlink, UT_test_textedit_showCursorBlink_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->showCursorBlink();
    int iRet = QApplication::cursorFlashTime();

    ASSERT_TRUE(iRet == 1000);
    pWindow->deleteLater();
}

//void hideCursorBlink();
TEST(UT_test_textedit_hideCursorBlink, UT_test_textedit_hideCursorBlink_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->hideCursorBlink();
    int iRet = QApplication::cursorFlashTime();

    ASSERT_TRUE(iRet == 0);
    pWindow->deleteLater();
}

//void setReadOnlyPermission(bool permission);
TEST(UT_test_textedit_setReadOnlyPermission, UT_test_textedit_setReadOnlyPermission_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setReadOnlyPermission(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->isReadOnly();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void setReadOnlyPermission(bool permission);
TEST(UT_test_textedit_setReadOnlyPermission, UT_test_textedit_setReadOnlyPermission_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = false;
    pWindow->currentWrapper()->textEditor()->setReadOnlyPermission(false);
    bool bRet = pWindow->currentWrapper()->textEditor()->isReadOnly();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void setReadOnlyPermission(bool permission);
TEST(UT_test_textedit_setReadOnlyPermission, UT_test_textedit_setReadOnlyPermission_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_readOnlyMode = true;
    pWindow->currentWrapper()->textEditor()->setReadOnlyPermission(false);
    bool bRet = pWindow->currentWrapper()->textEditor()->isReadOnly();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//bool getReadOnlyPermission();
TEST(UT_test_textedit_getReadOnlyPermission, UT_test_textedit_getReadOnlyPermission_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->getReadOnlyPermission();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//bool getReadOnlyMode();
TEST(UT_test_textedit_getReadOnlyMode, UT_test_textedit_getReadOnlyMode_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->getReadOnlyMode();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void hideRightMenu();
TEST(UT_test_textedit_hideRightMenu, UT_test_textedit_hideRightMenu_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Helle world\nHelle world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->hideRightMenu();
    bool bRet = pWindow->currentWrapper()->textEditor()->m_rightMenu->isHidden();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void flodOrUnflodAllLevel(bool isFlod);
TEST(UT_test_textedit_flodOrUnflodAllLevel, UT_test_textedit_flodOrUnflodAllLevel_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\n{\n}\n}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->flodOrUnflodAllLevel(true);
    QTextOption::WrapMode eRet = pWindow->currentWrapper()->textEditor()->wordWrapMode();

    ASSERT_TRUE(eRet == QTextOption::WrapMode::WrapAnywhere);
    pWindow->deleteLater();
}

//void flodOrUnflodAllLevel(bool isFlod);
TEST(UT_test_textedit_flodOrUnflodAllLevel, UT_test_textedit_flodOrUnflodAllLevel_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\n{\n}\n}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->flodOrUnflodAllLevel(true);
    pWindow->currentWrapper()->textEditor()->flodOrUnflodAllLevel(false);
    QTextOption::WrapMode eRet = pWindow->currentWrapper()->textEditor()->wordWrapMode();

    ASSERT_TRUE(eRet == QTextOption::WrapMode::WrapAnywhere);
    pWindow->deleteLater();
}

//void flodOrUnflodCurrentLevel(bool isFlod);
TEST(UT_test_textedit_flodOrUnflodCurrentLevel, UT_test_textedit_flodOrUnflodCurrentLevel_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\n{\n}\n}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->flodOrUnflodCurrentLevel(true);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget != nullptr);
    pWindow->deleteLater();
}

//void getHideRowContent
TEST(UT_test_textedit_getHideRowContent, UT_test_textedit_getHideRowContent_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\nHello world\n{\nHello world\n}\n}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->getHideRowContent(0);
    QString strRet(pWindow->currentWrapper()->textEditor()->m_foldCodeShow->m_pContentEdit->toPlainText());

    ASSERT_TRUE(strRet.compare(QString("{\nHello world\n{\nHello world\n}\n}")));
    pWindow->deleteLater();
}

//void isNeedShowFoldIcon
TEST(UT_test_textedit_getHideRowContent, UT_test_textedit_isNeedShowFoldIcon_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{Hello world}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QTextBlock textBlock = pWindow->currentWrapper()->textEditor()->textCursor().block();
    bool bRet = pWindow->currentWrapper()->textEditor()->isNeedShowFoldIcon(textBlock);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void isNeedShowFoldIcon
TEST(UT_test_textedit_getHideRowContent, UT_test_textedit_isNeedShowFoldIcon_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\nHello world}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QTextBlock textBlock = pWindow->currentWrapper()->textEditor()->textCursor().block();
    bool bRet = pWindow->currentWrapper()->textEditor()->isNeedShowFoldIcon(textBlock);

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//int  getHighLightRowContentLineNum
TEST(UT_test_textedit_getHighLightRowContentLineNum, UT_test_textedit_getHighLightRowContentLineNum_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{Hello world}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->getHighLightRowContentLineNum(0);

    ASSERT_TRUE(iRet == 0);
    pWindow->deleteLater();
}

//int  getHighLightRowContentLineNum
TEST(UT_test_textedit_getHighLightRowContentLineNum, UT_test_textedit_getHighLightRowContentLineNum_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\nHello world}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->getHighLightRowContentLineNum(1);

    ASSERT_TRUE(iRet == 1);
    pWindow->deleteLater();
}

//getHighLightRowContentLineNum
TEST(UT_test_textedit_getHighLightRowContentLineNum, UT_test_textedit_getHighLightRowContentLineNum_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\nHello world}}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->getHighLightRowContentLineNum(1);

    ASSERT_TRUE(iRet == 1);
    pWindow->deleteLater();
}

//paintCodeFlod
TEST(UT_test_textedit_paintCodeFlod, UT_test_textedit_paintCodeFlod_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\nHello world}}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QPainter painter(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pFlodArea);
    QRect rect(0, pWindow->currentWrapper()->textEditor()->cursorRect(textCursor).y(), 15, 15);
    pWindow->currentWrapper()->textEditor()->paintCodeFlod(&painter, rect, true);
    QPainter::RenderHints eRet = painter.renderHints();
    bool bRet = eRet.testFlag(QPainter::RenderHint::Antialiasing);

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//getBackColor
TEST(UT_test_textedit_getBackColor, UT_test_textedit_getBackColor_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("{\nHello world}}");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    QColor colorRet(pWindow->currentWrapper()->textEditor()->getBackColor());

    ASSERT_TRUE(!colorRet.name().isEmpty());
    pWindow->deleteLater();
}

//updateLeftWidgetWidth
TEST(UT_test_textedit_updateLeftWidgetWidth, UT_test_textedit_updateLeftWidgetWidth_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->updateLeftWidgetWidth(20);
    int iRet = pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pBookMarkArea->width();

    ASSERT_TRUE(iRet == 20);
    pWindow->deleteLater();
}

//lineNumberAreaWidth
TEST(UT_test_textedit_lineNumberAreaWidth, UT_test_textedit_lineNumberAreaWidth_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("H\ne\nl\nl\no\n w\no\nr\nl\nd\nH\ne\nl\nl\no\n w\no\nr\nl\nd\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->lineNumberAreaWidth();

    QFontMetrics fm(pWindow->currentWrapper()->textEditor()->m_fontLineNumberArea);
    int calcWitdh = fm.horizontalAdvance(QLatin1Char('9')) * 2;

    EXPECT_EQ(iRet, calcWitdh);
    pWindow->deleteLater();
}

//int  getLinePosYByLineNum(int iLine);
TEST(UT_test_textedit_getLinePosYByLineNum, UT_test_textedit_getLinePosYByLineNum_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->getLinePosYByLineNum(0);

    ASSERT_TRUE(iRet == 4);
    pWindow->deleteLater();
}

//bool ifHasHighlight();
TEST(UT_test_textedit_ifHasHighlight, UT_test_textedit_ifHasHighlight_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QTextCharFormat charFormat;
    charFormat.setBackground(QColor("red"));
    QTextEdit::ExtraSelection findExtraSelection;
    findExtraSelection.cursor = textCursor;
    findExtraSelection.format = charFormat;
    pWindow->currentWrapper()->textEditor()->m_findHighlightSelection = findExtraSelection;
    bool bRet = pWindow->currentWrapper()->textEditor()->ifHasHighlight();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//bool ifHasHighlight();
TEST(UT_test_textedit_ifHasHighlight, UT_test_textedit_ifHasHighlight_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    bool bRet = pWindow->currentWrapper()->textEditor()->ifHasHighlight();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//bookMarkAreaPaintEvent
TEST(UT_test_textedit_bookMarkAreaPaintEvent, UT_test_textedit_bookMarkAreaPaintEvent_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::DarkType);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->m_nBookMarkHoverLine = 2;
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->bookMarkAreaPaintEvent(pPaintEvent);
    int iRet = pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pBookMarkArea->width();

    ASSERT_TRUE(iRet == 15);
    pWindow->deleteLater();
}

//bookMarkAreaPaintEvent
TEST(UT_test_textedit_bookMarkAreaPaintEvent, UT_test_textedit_bookMarkAreaPaintEvent_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::LightType);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->m_nBookMarkHoverLine = 2;
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->bookMarkAreaPaintEvent(pPaintEvent);
    int iRet = pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pBookMarkArea->width();

    ASSERT_TRUE(iRet == 15);
    pWindow->deleteLater();
}

//bookMarkAreaPaintEvent
TEST(UT_test_textedit_bookMarkAreaPaintEvent, UT_test_textedit_bookMarkAreaPaintEvent_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    DApplicationHelper::instance()->setThemeType(DApplicationHelper::ColorType::LightType);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->m_nBookMarkHoverLine = 1;
    QPaintEvent *pPaintEvent;
    pWindow->currentWrapper()->textEditor()->bookMarkAreaPaintEvent(pPaintEvent);
    int iRet = pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget->m_pBookMarkArea->width();

    ASSERT_TRUE(iRet == 15);
    pWindow->deleteLater();
}

//int getLineFromPoint(const QPoint &point);
TEST(UT_test_textedit_getLineFromPoint, UT_test_textedit_getLineFromPoint_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    int iRet = pWindow->currentWrapper()->textEditor()->getLineFromPoint(QPoint(0, 13));

    ASSERT_TRUE(iRet == 1);
    pWindow->deleteLater();
}

//void addOrDeleteBookMark();
TEST(UT_test_textedit_addOrDeleteBookMark, UT_test_textedit_addOrDeleteBookMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_bIsShortCut = true;
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->addOrDeleteBookMark();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget != nullptr);
    pWindow->deleteLater();
}

//void addOrDeleteBookMark();
TEST(UT_test_textedit_addOrDeleteBookMark, UT_test_textedit_addOrDeleteBookMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_bIsShortCut = false;
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->addOrDeleteBookMark();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget != nullptr);
    pWindow->deleteLater();
}

//void addOrDeleteBookMark();
TEST(UT_test_textedit_addOrDeleteBookMark, UT_test_textedit_addOrDeleteBookMark_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_bIsShortCut = true;
    pWindow->currentWrapper()->textEditor()->addOrDeleteBookMark();

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget != nullptr);
    pWindow->deleteLater();
}

//void moveToPreviousBookMark();
TEST(UT_test_textedit_moveToPreviousBookMark, UT_test_textedit_moveToPreviousBookMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->moveToPreviousBookMark();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void moveToPreviousBookMark();
TEST(UT_test_textedit_moveToPreviousBookMark, UT_test_textedit_moveToPreviousBookMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->moveToPreviousBookMark();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void moveToPreviousBookMark();
TEST(UT_test_textedit_moveToPreviousBookMark, UT_test_textedit_moveToPreviousBookMark_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(3);
    pWindow->currentWrapper()->textEditor()->moveToPreviousBookMark();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void moveToNextBookMark();
TEST(UT_test_textedit_moveToNextBookMark, UT_test_textedit_moveToNextBookMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->moveToNextBookMark();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void moveToNextBookMark();
TEST(UT_test_textedit_moveToNextBookMark, UT_test_textedit_moveToNextBookMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->moveToNextBookMark();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void moveToNextBookMark();
TEST(UT_test_textedit_moveToNextBookMark, UT_test_textedit_moveToNextBookMarkk_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(3);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->moveToNextBookMark();

    ASSERT_TRUE(!pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void checkBookmarkLineMove(int from, int charsRemoved, int charsAdded);
TEST(UT_test_textedit_checkBookmarkLineMove, UT_test_textedit_checkBookmarkLineMove_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_bIsFileOpen = true;
    pWindow->currentWrapper()->textEditor()->checkBookmarkLineMove(3, 0, 0);

    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty());
    pWindow->deleteLater();
}

//void checkBookmarkLineMove(int from, int charsRemoved, int charsAdded);
TEST(UT_test_textedit_checkBookmarkLineMove, UT_test_textedit_checkBookmarkLineMove_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_nLines = 4;
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    pWindow->currentWrapper()->textEditor()->checkBookmarkLineMove(3, 0, 0);
    int iRet1 = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    int iRet2 = pWindow->currentWrapper()->textEditor()->m_nLines;

    ASSERT_TRUE(iRet1 == iRet2);
    pWindow->deleteLater();
}

//void checkBookmarkLineMove(int from, int charsRemoved, int charsAdded);
TEST(UT_test_textedit_checkBookmarkLineMove, UT_test_textedit_checkBookmarkLineMove_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_nLines = 4;
    pWindow->currentWrapper()->textEditor()->m_nSelectEndLine = -1;
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(3);
    pWindow->currentWrapper()->textEditor()->checkBookmarkLineMove(3, 0, 0);
    int iRet1 = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    int iRet2 = pWindow->currentWrapper()->textEditor()->m_nLines;

    ASSERT_TRUE(iRet1 == iRet2);
    pWindow->deleteLater();
}

//void checkBookmarkLineMove(int from, int charsRemoved, int charsAdded);
TEST(UT_test_textedit_checkBookmarkLineMove, UT_test_textedit_checkBookmarkLineMove_004)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_nLines = 3;
    pWindow->currentWrapper()->textEditor()->m_nSelectEndLine = -1;
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(3);
    pWindow->currentWrapper()->textEditor()->checkBookmarkLineMove(1, 0, 0);
    int iRet1 = pWindow->currentWrapper()->textEditor()->document()->blockCount();
    int iRet2 = pWindow->currentWrapper()->textEditor()->m_nLines;

    ASSERT_TRUE(iRet1 == iRet2);
    pWindow->deleteLater();
}

//void setIsFileOpen();
TEST(UT_test_textedit_setIsFileOpen, UT_test_textedit_setIsFileOpen_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world\n");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->setIsFileOpen();
    bool  bRet = pWindow->currentWrapper()->textEditor()->m_bIsFileOpen;

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void setTextFinished();
TEST(UT_test_textedit_setTextFinished, UT_test_textedit_setTextFinished_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->setTextFinished();
    bool  bRet = pWindow->currentWrapper()->textEditor()->m_bIsFileOpen;

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

QStringList UT_test_textedit_setTextFinished_002_readHistoryRecordofBookmark_stub()
{
    QStringList bookmarkList;
    bookmarkList << "*(2,*1,*2,*1,*4,*1,*1,*1)*";
    bookmarkList << "*(2,*1,*2,*1,*4,*1,*1)*";
    bookmarkList << "*(5,*6,*7,*1,*1,*1,*1)*";

    return bookmarkList;
}

QString m_sFilePath {QString()};
QStringList UT_test_textedit_setTextFinished_002_readHistoryRecordofFilePath_stub()
{
    QStringList filePathList;
    filePathList << "~/Desktop/1.txt";
    filePathList << "~/.local/share/deepin/deepin-editor/blank-files/blank_file_2021-09-28_08-44-42-244";
    filePathList << "~/.local/share/deepin/deepin-editor/blank-files/blank_file_2021-09-26_11-37-20-780";
    filePathList << m_sFilePath;

    return filePathList;
}

//void setTextFinished();
TEST(UT_test_textedit_setTextFinished, UT_test_textedit_setTextFinished_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    //pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    Stub stub_readHistoryRecordofBookmark;
    stub_readHistoryRecordofBookmark.set(ADDR(TextEdit, readHistoryRecordofBookmark), UT_test_textedit_setTextFinished_002_readHistoryRecordofBookmark_stub);
    m_sFilePath = pWindow->currentWrapper()->textEditor()->m_sFilePath;
    Stub stub_readHistoryRecordofFilePath;
    stub_readHistoryRecordofFilePath.set(ADDR(TextEdit, readHistoryRecordofFilePath), UT_test_textedit_setTextFinished_002_readHistoryRecordofFilePath_stub);
    pWindow->currentWrapper()->textEditor()->setTextFinished();
    bool  bRet = pWindow->currentWrapper()->textEditor()->m_bIsFileOpen;

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

QVariant UT_test_textedit_readHistoryRecord_001_option_stub()
{
    return QVariant("*{\"cursorPosition\":\"13\",\"modify\":false}*");
}

//QStringList readHistoryRecord(QString key);
TEST(UT_test_textedit_readHistoryRecord, UT_test_textedit_readHistoryRecord_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_option;
    stub_option.set(ADDR(DSettingsOption, value), UT_test_textedit_readHistoryRecord_001_option_stub);
    QStringList stringListRet = pWindow->currentWrapper()->textEditor()->readHistoryRecord(QString("advance.editor.browsing_history_file"));

    ASSERT_TRUE(stringListRet == QStringList("*{\"cursorPosition\":\"13\",\"modify\":false}*"));
    pWindow->deleteLater();
}

QVariant UT_test_textedit_readHistoryRecordofBookmark_001_option_stub()
{
    return QVariant("*(\"cursorPosition\":\"13\",\"modify\":false)*");
}

//QStringList readHistoryRecordofBookmark();
TEST(UT_test_textedit_readHistoryRecordofBookmark, UT_test_textedit_readHistoryRecordofBookmark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_option;
    stub_option.set(ADDR(DSettingsOption, value), UT_test_textedit_readHistoryRecordofBookmark_001_option_stub);
    QStringList stringListRet = pWindow->currentWrapper()->textEditor()->readHistoryRecordofBookmark();

    ASSERT_TRUE(stringListRet == QStringList("*(\"cursorPosition\":\"13\",\"modify\":false)*"));
    pWindow->deleteLater();
}

QVariant UT_test_textedit_readHistoryRecordofFilePath_001_option_stub()
{
    return QVariant("*[\"cursorPosition\":\"13\",\"modify\":false]*");
}

//QStringList readHistoryRecordofFilePath();
TEST(UT_test_textedit_readHistoryRecordofFilePath, UT_test_textedit_readHistoryRecordofFilePath_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    Stub stub_option;
    stub_option.set(ADDR(DSettingsOption, value), UT_test_textedit_readHistoryRecordofFilePath_001_option_stub);
    QStringList stringListRet = pWindow->currentWrapper()->textEditor()->readHistoryRecordofFilePath(QString());

    ASSERT_TRUE(stringListRet != QStringList("*[\"cursorPosition\":\"13\",\"modify\":false]*"));
    pWindow->deleteLater();
}

//void isMarkCurrentLine();
TEST(UT_test_textedit_isMarkCurrentLine, UT_test_textedit_isMarkCurrentLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"), -1);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void isMarkCurrentLine();
TEST(UT_test_textedit_isMarkCurrentLine, UT_test_textedit_isMarkCurrentLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"), -1);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void isMarkCurrentLine();
TEST(UT_test_textedit_isMarkCurrentLine, UT_test_textedit_isMarkCurrentLine_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(false, QString("red"), -1);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void isMarkAllLine(bool isMark, QString strColor = "");
TEST(UT_test_textedit_isMarkAllLine, UT_test_textedit_isMarkAllLine_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world one");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkAllLine(true, QString("red"));
    bool bRet = pWindow->currentWrapper()->textEditor()->m_mapKeywordMarkSelections.isEmpty();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void isMarkAllLine(bool isMark, QString strColor = "");
TEST(UT_test_textedit_isMarkAllLine, UT_test_textedit_isMarkAllLine_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkAllLine(true, QString("red"));
    bool bRet = pWindow->currentWrapper()->textEditor()->m_mapKeywordMarkSelections.isEmpty();

    ASSERT_TRUE(bRet == false);
    pWindow->deleteLater();
}

//void isMarkAllLine(bool isMark, QString strColor = "");
TEST(UT_test_textedit_isMarkAllLine, UT_test_textedit_isMarkAllLine_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->isMarkAllLine(false, QString("red"));
    QString strRet = pWindow->currentWrapper()->textEditor()->m_markAllSelection.format.background().color().name();

    ASSERT_TRUE(!strRet.compare(QString("#ff0000")));
    pWindow->deleteLater();
}

//void cancelLastMark();
TEST(UT_test_textedit_cancelLastMark, UT_test_textedit_cancelLastMark_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    pWindow->currentWrapper()->textEditor()->m_markOperations.clear();
    pWindow->currentWrapper()->textEditor()->cancelLastMark();
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void cancelLastMark();
TEST(UT_test_textedit_cancelLastMark, UT_test_textedit_cancelLastMark_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"), -1);
    //pWindow->currentWrapper()->textEditor()->isMarkAllLine(true, QString("red"));
    pWindow->currentWrapper()->textEditor()->m_markOperations.last().first.type = TextEdit::MarkOperationType::MarkLine;
    pWindow->currentWrapper()->textEditor()->cancelLastMark();
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void cancelLastMark();
TEST(UT_test_textedit_cancelLastMark, UT_test_textedit_cancelLastMark_003)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"), -1);
    pWindow->currentWrapper()->textEditor()->markAllInView(QString("red"), -1);
    pWindow->currentWrapper()->textEditor()->m_markOperations.last().first.type = TextEdit::MarkOperationType::MarkAllMatch;
    pWindow->currentWrapper()->textEditor()->cancelLastMark();
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void cancelLastMark();
TEST(UT_test_textedit_cancelLastMark, UT_test_textedit_cancelLastMark_004)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("Hello world\nHello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->isMarkCurrentLine(true, QString("red"), -1);
    pWindow->currentWrapper()->textEditor()->markAllInView(QString("red"), -1);
    pWindow->currentWrapper()->textEditor()->m_markOperations.last().first.type = TextEdit::MarkOperationType::MarkAll;
    pWindow->currentWrapper()->textEditor()->m_mapKeywordMarkSelections.insert(QString(TEXT_EIDT_MARK_ALL),QList<QPair<QTextEdit::ExtraSelection, qint64>>());
    pWindow->currentWrapper()->textEditor()->cancelLastMark();
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();

    ASSERT_TRUE(bRet == true);
    pWindow->deleteLater();
}

//void setThemeWithPath(const QString &path);
TEST_F(test_textedit, setThemeWithPath)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

//void setIsFileOpen();
TEST_F(test_textedit, setIsFileOpen)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    startManager->setWrapper(ee);
    startManager->setIsFileOpen();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

//void isMarkCurrentLine(bool isMark, QString strColor = "");
TEST_F(test_textedit, isMarkCurrentLine)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->isMarkCurrentLine(true, "red");
    startManager->isMarkCurrentLine(false, "red");

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}

//void markSelectWord();
TEST_F(test_textedit, markSelectWord)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->markSelectWord();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}

//void updateMark(int from, int charsRemoved, int charsAdded);
TEST_F(test_textedit, updateMark)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->updateMark(1, 2, 3);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}

//bool containsExtraSelection(QList<QTextEdit::ExtraSelection> listSelections, QTextEdit::ExtraSelection selection);
TEST_F(test_textedit, containsExtraSelection)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->containsExtraSelection(listSelection, selectio);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}

//void appendExtraSelection(QList<QTextEdit::ExtraSelection> wordMarkSelections, QTextEdit::ExtraSelection selection
//                          , QString strColor, QList<QTextEdit::ExtraSelection> *listSelections);
TEST_F(test_textedit, appendExtraSelection)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    selectio.cursor = startManager->textCursor();
    selectio.format.setBackground(QColor(Qt::red));
    QList<QTextEdit::ExtraSelection> listSelectionsd;
    listSelectionsd.append(selectio);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->appendExtraSelection(listSelection, selectio, "#000000", &listSelectionsd);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}
/// void setCursorStart(int _);
TEST_F(test_textedit, setCursorStart)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->setCursorStart(2);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}
//void writeEncodeHistoryRecord();
TEST_F(test_textedit, writeEncodeHistoryRecord)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->writeEncodeHistoryRecord();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}
//QStringList readEncodeHistoryRecord();
TEST_F(test_textedit, readEncodeHistoryRecord)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->readEncodeHistoryRecord();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}

//clickFindAction
TEST_F(test_textedit, clickFindAction)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->clickFindAction();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}
// void tellFindBarClose();
TEST_F(test_textedit, tellFindBarClose)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->tellFindBarClose();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
    s->deleteLater();
}

// void setEditPalette(const QString &activeColor, const QString &inactiveColor);
TEST_F(test_textedit, setEditPalette)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->setEditPalette("aa", "aa");

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void setCodeFoldWidgetHide(bool isHidden);
TEST_F(test_textedit, setCodeFoldWidgetHide)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->setCodeFoldWidgetHide(true);
    startManager->setCodeFoldWidgetHide(false);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}
// void updateLeftAreaWidget();
TEST_F(test_textedit, updateLeftAreaWidget)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->updateLeftAreaWidget();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void updateLineNumber();
TEST_F(test_textedit, updateLineNumber)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->updateLeftAreaWidget();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}
// void handleScrollFinish();
TEST_F(test_textedit, handleScrollFinish)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->handleScrollFinish();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void slot_translate();
TEST_F(test_textedit, slot_translate)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->slot_translate();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

#if 0 //gerrit上段错误，暂且屏蔽
// void upcaseWord();
 TEST_F(test_textedit, upcaseWord)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper * ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->upcaseWord();

    
}
#endif

#if 0 //gerrit上段错误，暂且屏蔽
// void downcaseWord();
 TEST_F(test_textedit, downcaseWord)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();TextEdit *startManager = new TextEdit();startManager->setVerticalScrollBar(p);
    EditWrapper * ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->downcaseWord();

    
}
#endif

// void capitalizeWord();
// TEST_F(test_textedit, capitalizeWord)
//{
//    QList<QTextEdit::ExtraSelection> listSelection;
//    QTextEdit::ExtraSelection selectio;
//    QScrollBar *p = new QScrollBar();TextEdit *startManager = new TextEdit();startManager->setVerticalScrollBar(p);
//    EditWrapper * ee = new EditWrapper();
//    Settings *s = new Settings();
//    startManager->setSettings(s);
//    startManager->setWrapper(ee);
//    startManager->capitalizeWord();

//    
//}
// void transposeChar();

TEST_F(test_textedit, transposeChar)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->transposeChar();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void handleCursorMarkChanged(bool mark, QTextCursor cursor);
TEST_F(test_textedit, handleCursorMarkChanged)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->handleCursorMarkChanged(true, QTextCursor());
    startManager->handleCursorMarkChanged(false, QTextCursor());

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void adjustScrollbarMargins();
TEST_F(test_textedit, adjustScrollbarMargins)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->adjustScrollbarMargins();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void onSelectionArea();
TEST_F(test_textedit, onSelectionArea)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->onSelectionArea();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}
// void fingerZoom(QString name, QString direction, int fingers);
TEST_F(test_textedit, fingerZoom)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    startManager->fingerZoom("aa", "aa", 3);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// bool event(QEvent* evt) override;   //触摸屏event事件
TEST_F(test_textedit, event)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QEvent *e = new QEvent(QEvent::Type::MouseButtonPress);
    startManager->event(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    delete e;
    e = nullptr;
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}


//TEST(UT_TextEdit_inputMethodEvent, UT_TextEdit_inputMethodEvent)

// void mousePressEvent(QMouseEvent *e) override;
TEST_F(test_textedit, mousePressEvent)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPoint a(1, 2);
    QPointF b(a);
    QMouseEvent *e = new QMouseEvent(QMouseEvent::Type::Enter, b, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    startManager->mousePressEvent(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

TEST(UT_TextEdit_mousePressEvent, UT_TextEdit_mousePressEvent_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonPress,QPointF(20.0,20.0),Qt::RightButton,Qt::RightButton,Qt::AltModifier);

    Stub s1;
    s1.set(ADDR(QMouseEvent,source),retintstub);

    intvalue=2;
    edit->m_bIsAltMod=true;
    edit->m_bIsFindClose=true;
    edit->mousePressEvent(e);


    EXPECT_NE(edit,nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

// void mouseMoveEvent(QMouseEvent *e) override;
TEST_F(test_textedit, mouseMoveEvent)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPoint a(1, 2);
    QPointF b(a);
    QMouseEvent *e = new QMouseEvent(QMouseEvent::Type::Enter, b, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    startManager->mouseMoveEvent(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

TEST(UT_TextEdit_mouseMoveEvent, UT_TextEdit_mouseMoveEvent_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseMove,QPointF(20.0,20.0),Qt::LeftButton,Qt::LeftButton,Qt::AltModifier);

    Stub s1;
    s1.set(ADDR(QMouseEvent,source),retintstub);

    intvalue=2;
    edit->m_bIsAltMod=true;
    edit->m_gestureAction == TextEdit::GA_slide;
    edit->mouseMoveEvent(e);


    EXPECT_NE(edit,nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}
// void mouseReleaseEvent(QMouseEvent *e) override;
TEST_F(test_textedit, mouseReleaseEvent)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPoint a(1, 2);
    QPointF b(a);
    QMouseEvent *e = new QMouseEvent(QMouseEvent::Type::Enter, b, Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    startManager->mouseReleaseEvent(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

TEST(UT_TextEdit_mouseReleaseEvent, UT_TextEdit_mouseReleaseEvent_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonRelease,QPointF(20.0,20.0),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(QMouseEvent,source),retintstub);

    intvalue = 2;
    edit->mouseReleaseEvent(e);


    EXPECT_NE(edit,nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

// void wheelEvent(QWheelEvent *e) override;
TEST_F(test_textedit, wheelEvent)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPointF pos;
    QWheelEvent *e = new QWheelEvent(pos, 4, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::AltModifier);

    //    QWheelEvent(const QPointF &pos, int delta,
    //    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
    //    Qt::Orientation orient = Qt::Vertical);
    startManager->wheelEvent(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

QAction *stub_exec(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos)
    Q_UNUSED(at)

    return nullptr;
}
// void contextMenuEvent(QContextMenuEvent *event) override;

TEST_F(test_textedit, contextMenuEvent)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    QPoint b(0, 0);
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    Stub stub;
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), stub_exec);
    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Reason::Keyboard, b);
    //startManager->contextMenuEvent(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void paintEvent(QPaintEvent *e) override;
TEST_F(test_textedit, paintEvent)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QRect a(1, 2, 3, 4);
    QPaintEvent *e = new QPaintEvent(a);
    startManager->paintEvent(e);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

TEST(UT_TextEdit_paintEvent, UT_TextEdit_paintEvent_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QPaintEvent* e = new QPaintEvent(QRect(20,20,20,20));

    QTextEdit::ExtraSelection s1,s2;
    edit->m_altModSelections.push_back(s1);
    edit->m_altModSelections.push_back(s2);
    edit->m_bIsAltMod = true;

    edit->paintEvent(e);

    EXPECT_NE(edit,nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

// bool blockContainStrBrackets(int line);
TEST_F(test_textedit, blockContainStrBrackets)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPaintEvent *e;
    startManager->blockContainStrBrackets(2);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

TEST(UT_TextEdit_blockContainStrBrackets, UT_TextEdit_blockContainStrBrackets_002)
{
   TextEdit* edit = new TextEdit;
   Stub s1;
   s1.set((bool (QString::*) (QRegExp &) const )ADDR(QString,contains),rettruestub);

   edit->blockContainStrBrackets(1);

   EXPECT_NE(edit,nullptr);
   edit->deleteLater();
}

TEST(UT_TextEdit_blockContainStrBrackets, UT_TextEdit_blockContainStrBrackets_003)
{
   TextEdit* edit = new TextEdit;
   Stub s1;
   s1.set((bool (QString::*) (QRegExp &) const )ADDR(QString,contains),retfalsestub);

   edit->blockContainStrBrackets(1);

   EXPECT_NE(edit,nullptr);
   edit->deleteLater();
}

// bool setCursorKeywordSeletoin(int position, bool findNext);
TEST_F(test_textedit, setCursorKeywordSeletoin)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPaintEvent *e;
    startManager->setCursorKeywordSeletoin(2, true);
    startManager->setCursorKeywordSeletoin(2, false);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void cursorPositionChanged();
TEST_F(test_textedit, cursorPositionChanged)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPaintEvent *e;
    startManager->cursorPositionChanged();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// void cut();
TEST_F(test_textedit, cut_normal_passed)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    QString strMsg("hello world\n"
                   "hello world");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->cut();

    QString strRet1(pClipboard->text());
    QString strRet2(pWindow->currentWrapper()->textEditor()->toPlainText());
    EXPECT_EQ(strRet1, QString("hello world"));
    EXPECT_EQ(strRet2, QString("hello world\n"));
    pWindow->currentWrapper()->textEditor()->undo_();
    QString strRet3(pWindow->currentWrapper()->textEditor()->toPlainText());
    EXPECT_EQ(strRet3, strMsg);

    pWindow->deleteLater();
}

TEST_F(test_textedit, cut_withMultiByteText_passed)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    // 多字节字符编码 CJK
    // 𢝐𢝑𢝒𢝓𢝔𢝕𢝖𢝗𢝘𢝙\n𢝚𢝛𢝜𢝝𢝞𢝟𢝠𢝡𢝢𢝣
    QString strMsg("\360\242\235\220\360\242\235\221\360\242\235\222\360\242\235\223\360\242\235\224"
                   "\360\242\235\225\360\242\235\226\360\242\235\227\360\242\235\230\360\242\235\231\n"
                   "\360\242\235\232\360\242\235\233\360\242\235\234\360\242\235\235\360\242\235\236"
                   "\360\242\235\237\360\242\235\240\360\242\235\241\360\242\235\242\360\242\235\243");
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    pWindow->currentWrapper()->textEditor()->insertTextEx(textCursor, strMsg);

    textCursor.setPosition(0);
    textCursor.setPosition(6, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QClipboard *pClipboard = QApplication::clipboard();
    pClipboard->clear();
    pWindow->currentWrapper()->textEditor()->cut();

    QString strRet1(pClipboard->text());
    QString strRet2(pWindow->currentWrapper()->textEditor()->toPlainText());
    EXPECT_EQ(strRet1, strMsg.leftRef(6));
    EXPECT_EQ(strRet2, strMsg.midRef(6));
    pWindow->currentWrapper()->textEditor()->undo_();
    QString strRet3(pWindow->currentWrapper()->textEditor()->toPlainText());
    EXPECT_EQ(strRet3, strMsg);

    pWindow->deleteLater();
}

// void updateHighlightBrackets(const QChar &openChar, const QChar &closeChar);
TEST_F(test_textedit, updateHighlightBrackets)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    QChar a = ' ';
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPaintEvent *e;
    startManager->updateHighlightBrackets(a, a);

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

// int getFirstVisibleBlockId() const;
TEST_F(test_textedit, getFirstVisibleBlockId)
{
    QList<QTextEdit::ExtraSelection> listSelection;
    QTextEdit::ExtraSelection selectio;
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    EditWrapper *ee = new EditWrapper();
    Settings *s = new Settings();
    startManager->setSettings(s);
    startManager->setWrapper(ee);
    QPaintEvent *e;
    //startManager->getFirstVisibleBlockId();

    EXPECT_NE(ee->m_pTextEdit , nullptr);
    s->deleteLater();
    ee->deleteLater();
    startManager->deleteLater();
    p->deleteLater();
}

//void setTruePath(QString qstrTruePath);
TEST_F(test_textedit, setBackupPath)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    startManager->setTruePath("aa");

    ASSERT_TRUE(startManager->m_pLeftAreaWidget != nullptr);
    startManager->deleteLater();
    p->deleteLater();
}

// QString getTruePath();
TEST_F(test_textedit, getTruePath)
{
    QScrollBar *p = new QScrollBar();
    TextEdit *startManager = new TextEdit();
    startManager->setVerticalScrollBar(p);
    startManager->getTruePath();

    ASSERT_TRUE(startManager->m_pLeftAreaWidget != nullptr);
    startManager->deleteLater();
    p->deleteLater();
}

////初始化右键菜单
//void initRightClickedMenu();
TEST_F(test_textedit, initRightClickedMenu)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->initRightClickedMenu();
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_canUndo == false);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_canRedo == false);

    pWindow->deleteLater();
}

//slotSigColorSelected
TEST_F(test_textedit, slotSigColorSelected)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->slotSigColorSelected(true, QColor("red"));
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_rightMenu != nullptr);

    pWindow->deleteLater();
}

//slotSigColorAllSelected
TEST_F(test_textedit, slotSigColorAllSelected)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->slotSigColorAllSelected(true, QColor("red"));
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_rightMenu != nullptr);

    pWindow->deleteLater();
}

//slotCutAction
TEST_F(test_textedit, slotCutAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    QString strBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->slotCutAction(true);
    QString strAfter(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(strBefore.compare(strAfter));

    pWindow->deleteLater();
}

//slotCopyAction
TEST_F(test_textedit, slotCopyAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotCopyAction(true);
    QClipboard *pClipboard = QApplication::clipboard();
    QString strRet(pClipboard->text());
    ASSERT_TRUE(!strRet.compare(QString("Holle world.")));

    pWindow->deleteLater();
}

//slotPasteAction
TEST_F(test_textedit, slotPasteAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotCopyAction(true);
    QString strBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotPasteAction(true);
    QString strAfter(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(strBefore.compare(strAfter));

    pWindow->deleteLater();
}

//slotDeleteAction 001
TEST_F(test_textedit, slotDeleteAction_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotSelectAllAction(true);
    QString strBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->slotDeleteAction(true);
    QString strAfter(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(strBefore.compare(strAfter));

    pWindow->deleteLater();
}

//slotDeleteAction 002
TEST_F(test_textedit, slotDeleteAction_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QString strBefore(pWindow->currentWrapper()->textEditor()->toPlainText());
    pWindow->currentWrapper()->textEditor()->slotDeleteAction(true);
    QString strAfter(pWindow->currentWrapper()->textEditor()->toPlainText());
    ASSERT_TRUE(!strBefore.compare(strAfter));

    pWindow->deleteLater();
}

//slotSelectAllAction
TEST_F(test_textedit, slotSelectAllAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotSelectAllAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_isSelectAll;
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotOpenInFileManagerAction
TEST_F(test_textedit, slotOpenInFileManagerAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QString strRet(pWindow->currentWrapper()->textEditor()->getTruePath());
    ASSERT_TRUE(!strRet.isEmpty());

    pWindow->deleteLater();
}

//slotAddComment
TEST_F(test_textedit, slotAddComment)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotAddComment(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_commentDefinition.isValid();
    ASSERT_TRUE(bRet == false);

    pWindow->deleteLater();
}

//slotCancelComment
TEST_F(test_textedit, slotCancelComment)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotCancelComment(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_commentDefinition.isValid();
    ASSERT_TRUE(bRet == false);

    pWindow->deleteLater();
}

//slotVoiceReadingAction
TEST_F(test_textedit, slotVoiceReadingAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotVoiceReadingAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotStopReadingAction
TEST_F(test_textedit, slotStopReadingAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotStopReadingAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotdictationAction
TEST_F(test_textedit, slotdictationAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotdictationAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotColumnEditAction
TEST_F(test_textedit, slotColumnEditAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    QTextCursor textCursor = pWindow->currentWrapper()->textEditor()->textCursor();
    textCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    pWindow->currentWrapper()->textEditor()->setTextCursor(textCursor);
    pWindow->currentWrapper()->textEditor()->slotColumnEditAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->textCursor().hasSelection();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotPreBookMarkAction 001
TEST_F(test_textedit, slotPreBookMarkAction_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->m_mouseClickPos = QPoint(0, 0);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(1);
    pWindow->currentWrapper()->textEditor()->slotPreBookMarkAction(true);
    int iRet = pWindow->currentWrapper()->textEditor()->m_listBookmark.indexOf(1);
    ASSERT_TRUE(iRet == 0);

    pWindow->deleteLater();
}

//slotPreBookMarkAction 002
TEST_F(test_textedit, slotPreBookMarkAction_002)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world."));
    pWindow->currentWrapper()->textEditor()->m_mouseClickPos = QPoint(0, 0);
    pWindow->currentWrapper()->textEditor()->slotPreBookMarkAction(true);
    int iRet = pWindow->currentWrapper()->textEditor()->m_listBookmark.indexOf(1);
    ASSERT_TRUE(iRet != 0);

    pWindow->deleteLater();
}

void slotNextBookMarkAction_001_jumpToLine_stub()
{
    return;
}

//slotNextBookMarkAction 001
TEST_F(test_textedit, slotNextBookMarkAction_001)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->m_mouseClickPos = QPoint(0, 0);
    pWindow->currentWrapper()->textEditor()->m_listBookmark.append(2);
    Stub jumpToLine_stub;
    jumpToLine_stub.set(ADDR(TextEdit, jumpToLine), slotNextBookMarkAction_001_jumpToLine_stub);
    pWindow->currentWrapper()->textEditor()->slotNextBookMarkAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty();
    ASSERT_TRUE(bRet == false);

    pWindow->deleteLater();
}

//slotClearBookMarkAction
TEST_F(test_textedit, slotClearBookMarkAction)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotClearBookMarkAction(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_listBookmark.isEmpty();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotFlodAllLevel
TEST_F(test_textedit, slotFlodAllLevel)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotFlodAllLevel(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_listMainFlodAllPos.isEmpty();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotUnflodAllLevel
TEST_F(test_textedit, slotUnflodAllLevel)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotUnflodAllLevel(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_listMainFlodAllPos.isEmpty();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotFlodCurrentLevel
TEST_F(test_textedit, slotFlodCurrentLevel)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotFlodCurrentLevel(true);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget != nullptr);

    pWindow->deleteLater();
}

//slotUnflodCurrentLevel
TEST_F(test_textedit, slotUnflodCurrentLevel)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotUnflodCurrentLevel(true);
    ASSERT_TRUE(pWindow->currentWrapper()->textEditor()->m_pLeftAreaWidget != nullptr);

    pWindow->deleteLater();
}

//slotCancleMarkAllLine
TEST_F(test_textedit, slotCancleMarkAllLine)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotCancleMarkAllLine(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotCancleLastMark
TEST_F(test_textedit, slotCancleLastMark)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotCancleLastMark(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_markOperations.isEmpty();
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotUndoAvailable
TEST_F(test_textedit, slotUndoAvailable)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotUndoAvailable(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_canUndo;
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

//slotRedoAvailable
TEST_F(test_textedit, slotRedoAvailable)
{
    Window *pWindow = new Window();
    pWindow->addBlankTab(QString());
    pWindow->currentWrapper()->textEditor()->insertTextEx(pWindow->currentWrapper()->textEditor()->textCursor(),
                                                          QString("Holle world.\nHolle world."));
    pWindow->currentWrapper()->textEditor()->slotRedoAvailable(true);
    bool bRet = pWindow->currentWrapper()->textEditor()->m_canRedo;
    ASSERT_TRUE(bRet == true);

    pWindow->deleteLater();
}

/*
    void unCommentSelection();
    void setComment();
    void removeComment();
*/

TEST_F(test_textedit, unCommentSelection)
{
    EditWrapper *editWrapper = new EditWrapper;
    editWrapper->openFile("1.cpp", "1.cpp");
    QTextCursor cursor = editWrapper->m_pTextEdit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    editWrapper->m_pTextEdit->setTextCursor(cursor);
    editWrapper->m_pTextEdit->unCommentSelection();

    EXPECT_NE(editWrapper->m_pTextEdit , nullptr);
    editWrapper->deleteLater();
}

TEST(UT_TextEdit_unCommentSelection, UT_TextEdit_unCommentSelection_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;


    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,hasSelection),rettruestub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,hasMultiLineStyle),rettruestub);
    Stub s4;
    s4.set(ADDR(QString,length),retintstub);
    Stub s5;
    s5.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s6;
    s6.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s7;
    s7.set(ADDR(TextEdit,insertTextEx),rettruestub);


    intvalue = -1000;
    edit->unCommentSelection();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_TextEdit_unCommentSelection, UT_TextEdit_unCommentSelection_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;


    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,hasSelection),retfalsestub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,hasSingleLineStyle),retfalsestub);
    Stub s4;
    s4.set(ADDR(QString,length),retintstub);
    Stub s5;
    s5.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s6;
    s6.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),rettruestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,endsWith),rettruestub);
    Stub s8;
    s8.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s9;
    s9.set(ADDR(TextEdit,insertTextEx),rettruestub);

    intvalue = -1000;
    edit->unCommentSelection();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_TextEdit_unCommentSelection, UT_TextEdit_unCommentSelection_004)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;


    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,hasSelection),retfalsestub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,hasSingleLineStyle),retfalsestub);
    Stub s4;
    s4.set(ADDR(QString,length),retintstub);
    Stub s5;
    s5.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s6;
    s6.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),retfalsestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,endsWith),retfalsestub);
    Stub s8;
    s8.set(ADDR(QString,isEmpty),retfalsestub);
    Stub s9;
    s9.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s10;
    s10.set(ADDR(TextEdit,insertTextEx),rettruestub);

    intvalue = -1000;
    edit->unCommentSelection();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_TextEdit_unCommentSelection, UT_TextEdit_unCommentSelection_005)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;


    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,hasSelection),retfalsestub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,hasSingleLineStyle),retfalsestub);
    Stub s4;
    s4.set(ADDR(QString,length),retintstub);
    Stub s5;
    s5.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s6;
    s6.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),retfalsestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,endsWith),retfalsestub);
    Stub s8;
    s8.set(ADDR(QString,isEmpty),rettruestub);
    Stub s9;
    s9.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s10;
    s10.set(ADDR(TextEdit,insertTextEx),rettruestub);

    intvalue = -1000;
    edit->unCommentSelection();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}


TEST_F(test_textedit, setComment)
{
    EditWrapper *editWrapper = new EditWrapper;
    editWrapper->openFile("1.cpp", "1.cpp");
    QTextCursor cursor = editWrapper->m_pTextEdit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    editWrapper->m_pTextEdit->setTextCursor(cursor);
    editWrapper->m_pTextEdit->setComment();

    EXPECT_NE(editWrapper->m_pTextEdit , nullptr);
    editWrapper->deleteLater();
}

TEST(UT_TextEdit_setComment, UT_TextEdit_setComment_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,isNull),retfalsestub);
    Stub s3;
    s3.set(ADDR(QTextCursor,hasSelection),rettruestub);
    Stub s4;
    s4.set(ADDR(Comment::CommentDefinition,hasMultiLineStyle),rettruestub);
    Stub s5;
    s5.set(ADDR(QString,length),retintstub);
    Stub s6;
    s6.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s7;
    s7.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s8;
    s8.set(ADDR(TextEdit,insertTextEx),rettruestub);


    intvalue = -1000;
    edit->setComment();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_TextEdit_setComment, UT_TextEdit_setComment_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,isNull),retfalsestub);
    Stub s3;
    s3.set(ADDR(QTextCursor,hasSelection),retfalsestub);
    Stub s4;
    s4.set(ADDR(Comment::CommentDefinition,hasSingleLineStyle),retfalsestub);
    Stub s5;
    s5.set(ADDR(QString,length),retintstub);
    Stub s6;
    s6.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),rettruestub);
    Stub s8;
    s8.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,endsWith),rettruestub);
    Stub s9;
    s9.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s10;
    s10.set(ADDR(TextEdit,insertTextEx),rettruestub);


    intvalue = -1000;
    edit->setComment();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_TextEdit_setComment, UT_TextEdit_setComment_004)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,isNull),retfalsestub);
    Stub s3;
    s3.set(ADDR(QTextCursor,hasSelection),retfalsestub);
    Stub s4;
    s4.set(ADDR(Comment::CommentDefinition,hasSingleLineStyle),retfalsestub);
    Stub s5;
    s5.set(ADDR(QString,length),retintstub);
    Stub s6;
    s6.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),retfalsestub);
    Stub s8;
    s8.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,endsWith),retfalsestub);
    Stub s9;
    s9.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s10;
    s10.set(ADDR(TextEdit,insertTextEx),rettruestub);


    intvalue = -1000;
    edit->setComment();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST_F(test_textedit, removeComment)
{
    EditWrapper *editWrapper = new EditWrapper;
    editWrapper->openFile("1.cpp", "1.cpp");
    QTextCursor cursor = editWrapper->m_pTextEdit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    editWrapper->m_pTextEdit->setTextCursor(cursor);
    editWrapper->m_pTextEdit->removeComment();

    EXPECT_NE(editWrapper->m_pTextEdit , nullptr);
    editWrapper->deleteLater();
}

TEST(UT_Textedit_removeComment, UT_Textedit_removeComment_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,hasSelection),rettruestub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,hasMultiLineStyle),rettruestub);
    Stub s4;
    s4.set(ADDR(QString,length),retintstub);
    Stub s5;
    s5.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s6;
    s6.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s7;
    s7.set(ADDR(TextEdit,insertTextEx),rettruestub);

    intvalue = -1000;
    edit->removeComment();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_removeComment, UT_Textedit_removeComment_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    Stub s1;
    s1.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s2;
    s2.set(ADDR(QTextCursor,hasSelection),retfalsestub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,hasSingleLineStyle),retfalsestub);
    Stub s4;
    s4.set(ADDR(QString,length),retintstub);
    Stub s5;
    s5.set(ADDR(TextEdit,isComment),rettruestub);
    Stub s6;
    s6.set(ADDR(TextEdit,deleteTextEx),rettruestub);
    Stub s7;
    s7.set(ADDR(TextEdit,insertTextEx),rettruestub);

    intvalue = -1000;
    edit->removeComment();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST_F(test_textedit, keyPressEvent)
{
    Window *window = new Window;
    EditWrapper *editWrapper = window->createEditor();
    editWrapper->openFile("1.cpp", "1.cpp");
    QTextCursor cursor = editWrapper->m_pTextEdit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    editWrapper->m_pTextEdit->setTextCursor(cursor);
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Insert, Qt::NoModifier);
    editWrapper->m_pTextEdit->keyPressEvent(&keyEvent);

    EXPECT_NE(editWrapper->m_pTextEdit , nullptr);
    editWrapper->deleteLater();
    window->deleteLater();
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,copy),rettruestub);

    string1 = "e";
    string2 = "e";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,nextLine),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "J";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_004)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,prevLine),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "K";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_005)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,moveToEnd),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = ",";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_006)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,moveToStart),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = ".";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_007)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,backwardChar),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "H";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_008)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,forwardChar),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "L";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_009)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,scrollUp),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "space";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_010)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,scrollDown),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "V";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_011)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,forwardWord),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "F";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_012)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,backwardWord),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "B";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_013)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,moveToStartOfLine),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "A";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_014)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,moveToEndOfLine),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "E";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_015)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,moveToLineIndentation),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "M";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_016)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,toggleReadOnlyMode),rettruestub);

    edit->m_readOnlyMode=true;
    edit->m_bReadOnlyPermission=false;
    string1 = "Q";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_017)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,scrollLineUp),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "Shfit+J";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_018)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,scrollLineDown),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "Shfit+K";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_019)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,forwardPair),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "P";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_020)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, backwardPair),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "N";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_021)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, copyLines),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "Shift+:";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_022)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_H,Qt::ControlModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, copyLines),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "Shift+/";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_023)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_Control,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, copyLines),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_024)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_F11,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, copyLines),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_025)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_F1,Qt::NoModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, popupNotify),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_026)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_F1,Qt::GroupSwitchModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, popupNotify),rettruestub);

    edit->m_readOnlyMode=true;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_027)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_F1,Qt::GroupSwitchModifier);

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, isReadOnly),rettruestub);



    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_028)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_ydiaeresis,Qt::NoModifier,"123");


    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);


    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_029)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress,Qt::Key_9,Qt::KeypadModifier,"123");


    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);


    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_030)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Tab,Qt::NoModifier,"123");

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

//if (modifiers == Qt::NoModifier && (e->key() == Qt::Key_Backspace))
TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_031)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Backspace,Qt::NoModifier,"123");

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_032)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Delete,Qt::NoModifier,"123");

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);
    s4.set(ADDR(QUndoStack,push),rettruestub);

    QTextEdit::ExtraSelection e1,e2;
    edit->m_altModSelections.push_back(e1);
    edit->m_altModSelections.push_back(e2);
    edit->m_bIsAltMod=true;
    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_033)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Delete,Qt::NoModifier,"123");

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);

    QTextEdit::ExtraSelection e1,e2;
    edit->m_altModSelections.push_back(e1);
    edit->m_altModSelections.push_back(e2);
    edit->m_bIsAltMod=false;
    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_034)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Shift,Qt::ShiftModifier,"123");

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    Stub s2;
    s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_035)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);
    s4.set(ADDR(TextEdit,undo_),retfalsestub);
    s4.set(ADDR(TextEdit,redo_),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "undo");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_036)
{
//    TextEdit* edit = new TextEdit;
//    EditWrapper* wra = new EditWrapper;
//    edit->m_wrapper = wra;
//    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
//    edit->m_settings = Settings::instance();

//    Stub s1;
//    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
//    //Stub s2;
//    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
//    Stub s3;
//    s3.set(ADDR(TextEdit, insertColumnEditTextEx),retfalsestub);
//    Stub s4;
//    s4.set(ADDR(TextEdit, insertSelectTextEx),retfalsestub);

//    edit->m_readOnlyMode = false;
//    edit->m_bReadOnlyPermission=false;
//    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "redo");
//    //string2 = "789";
//    edit->keyPressEvent(e);

//    EXPECT_NE(edit , nullptr);
//    edit->deleteLater();
//    wra->deleteLater();
//    delete e;
//    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_037)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, cut),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "cut");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_038)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, paste),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "paste");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_039)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, scrollUp),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "scrollup");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_040)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, scrollDown),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "scrolldown");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_041)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, copyLines),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "copylines");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_042)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, cutlines),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "cutlines");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_043)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    //s3.set(ADDR(TextEdit, indentline),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "indentline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_044)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, unindentText),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "backindentline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_045)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, forwardChar),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "forwardchar");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}


TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_046)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, backwardWord),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "backwardword");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_047)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, nextLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "nextline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_048)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, prevLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "prevline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_049)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, newline),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "newline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_050)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, openNewlineAbove),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "opennewlineabove");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_051)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, openNewlineBelow),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "opennewlinebelow");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_052)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, duplicateLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "duplicateline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_053)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, killLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "killline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_054)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,  killCurrentLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "killcurrentline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_055)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,  moveLineDownUp),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "swaplineup");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_056)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,  moveLineDownUp),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "swaplinedown");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_057)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit,  scrollLineUp),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "scrolllineup");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_058)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, scrollLineDown),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "scrolllinedown");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_059)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, scrollUp),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "scrollup");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_060)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, scrollDown),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "scrolldown");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_061)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, moveToEndOfLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetoendofline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_062)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, moveToStartOfLine),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetostartofline");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_063)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, moveToStart),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetostart");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_064)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, moveToEnd),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetoend");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_065)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, moveToLineIndentation),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetolineindentation");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_066)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, upcaseWord),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "upcaseword");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_067)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, downcaseWord),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "downcaseword");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_068)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, capitalizeWord),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "capitalizeword");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_069)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, killBackwardWord),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "killbackwardword");
    //string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_070)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_ydiaeresis + 3,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;

    Stub s3;
    s3.set(ADDR(TextEdit, killForwardWord),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "killforwardword");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, forwardPair),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "forwardpair");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, backwardPair),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "backwardpair");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, transposeChar),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "transposechar");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, setMark),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "setmark");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, exchangeMark),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "exchangemark");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, joinLines),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "joinlines");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, toggleReadOnlyMode),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "togglereadonlymode");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, toggleReadOnlyMode),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "togglereadonlymode");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, toggleComment),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "togglecomment");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, addOrDeleteBookMark),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "switchbookmark");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, moveToPreviousBookMark),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetoprebookmark");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, moveToNextBookMark),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "movetonextbookmark");
    edit->keyPressEvent(e);

    s3.set(ADDR(TextEdit, toggleMarkSelections),retfalsestub);
    string1 = Utils::getKeyshortcutFromKeymap(edit->m_settings, "editor", "mark");
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_071)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Insert,Qt::NoModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, overwriteMode),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "987";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}

TEST(UT_TextEdit_KeyPressEvent, UT_TextEdit_KeyPressEvent_072)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Insert,Qt::GroupSwitchModifier,"123");
    edit->m_settings = Settings::instance();

    Stub s1;
    s1.set(ADDR(Utils,getKeyshortcut),retstring1);
    //Stub s2;
    //s2.set(ADDR(Utils,getKeyshortcutFromKeymap),retstring2);
    Stub s3;
    s3.set(ADDR(TextEdit, overwriteMode),retfalsestub);

    edit->m_readOnlyMode = false;
    edit->m_bReadOnlyPermission=false;
    string1 = "Shift+Ins";
    string2 = "789";
    edit->keyPressEvent(e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e=nullptr;
}


TEST(UT_Textedit_resizeEvent, UT_Textedit_resizeEvent)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QResizeEvent* r = new QResizeEvent(QSize(30,30),QSize(20,20));


    edit->m_isSelectAll = true;
    edit->resizeEvent(r);

    edit->m_isSelectAll = false;
    edit->resizeEvent(r);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete r;
    r = nullptr;
}

TEST(UT_Textedit_dragMoveEvent, UT_Textedit_dragMoveEvent)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMimeData* data = new QMimeData();
    data->setText("ddd");
    QDragMoveEvent* r = new QDragMoveEvent(QPoint(20,20),Qt::ActionMask,data,Qt::LeftButton,Qt::NoModifier);


    QList<QUrl> urls = {{"123"},{"456"}};
    data->setUrls(urls);
    edit->m_readOnlyMode = false;
    edit->dragMoveEvent(r);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    data->deleteLater();
    delete r;
    r = nullptr;
}


TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_001)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QTouchEvent* e = new QTouchEvent(QEvent::TouchBegin);


    edit->eventFilter(wra,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    delete e;
    e = nullptr;
}

TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonPress,QPointF(20.0,20.0),Qt::RightButton,Qt::RightButton,Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    intvalue=1;
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pBookMarkArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}

TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonPress,QPointF(20.0,20.0),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);
    Stub s2;
    s2.set(ADDR(TextEdit,renderAllSelections),retintstub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s4;
    s4.set(ADDR(QTextBlock,isVisible),rettruestub);
    Stub s5;
    //inline bool contains(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    s5.set((bool (QString::*)(const QStringRef &, Qt::CaseSensitivity) const) ADDR(QString,contains),rettruestub);
    Stub s6;
    //s6.set(ADDR(QString,isEmpty),retfalsestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),rettruestub);
    Stub s8;
    s8.set((bool (QString::*)(const QString &, Qt::CaseSensitivity) const) ADDR(QString,contains),rettruestub);


    intvalue=1;
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pFlodArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}

TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_004)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonPress,QPointF(20.0,20.0),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);
    Stub s2;
    s2.set(ADDR(TextEdit,renderAllSelections),retintstub);
    Stub s3;
    s3.set(ADDR(Comment::CommentDefinition,isValid),rettruestub);
    Stub s4;
    s4.set(ADDR(QTextBlock,isVisible),retfalsestub);
    Stub s5;
    //inline bool contains(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    s5.set((bool (QString::*)(const QStringRef &, Qt::CaseSensitivity) const) ADDR(QString,contains),rettruestub);
    Stub s6;
   // s6.set(ADDR(QString,isEmpty),retfalsestub);
    Stub s7;
    s7.set((bool (QString::*) (const QString &, Qt::CaseSensitivity) const )ADDR(QString,startsWith),retfalsestub);
    Stub s8;
    s8.set((bool (QString::*)(const QString &, Qt::CaseSensitivity) const) ADDR(QString,contains),rettruestub);


    intvalue=1;
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pFlodArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}



TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_005)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QMouseEvent* e = new QMouseEvent(QEvent::MouseButtonPress,QPointF(20.0,20.0),Qt::RightButton,Qt::RightButton,Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    Stub s2;
    s2.set(ADDR(QList<int>,contains),rettruestub);
    Stub s3;
    s3.set(ADDR(QTextBlock,isVisible),rettruestub);

    intvalue=1;
    edit->m_listMainFlodAllPos.push_back(1);
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pFlodArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}


TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_006)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QHoverEvent* e = new QHoverEvent(QEvent::HoverMove,QPointF(20.0,20.0),QPointF(30.0,30.0),Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    Stub s2;
    s2.set(ADDR(QList<int>,contains),rettruestub);
    Stub s3;
    s3.set(ADDR(QTextBlock,isVisible),retfalsestub);
    Stub s4;
    s4.set(ADDR(TextEdit,getHideRowContent),rettruestub);
    Stub s5;
    s5.set(ADDR(TextEdit,getLinePosYByLineNum),rettruestub);

    intvalue=1;
    edit->m_listMainFlodAllPos.push_back(1);
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pFlodArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}

TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_007)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QHoverEvent* e = new QHoverEvent(QEvent::HoverMove,QPointF(20.0,20.0),QPointF(30.0,30.0),Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    Stub s2;
    s2.set(ADDR(QList<int>,contains),rettruestub);
    Stub s3;
    s3.set(ADDR(QTextBlock,isVisible),rettruestub);
    Stub s4;
    s4.set(ADDR(TextEdit,getHideRowContent),rettruestub);
    Stub s5;
    s5.set(ADDR(TextEdit,getHighLightRowContentLineNum),retintstub);


    intvalue=4;
    edit->m_listMainFlodAllPos.push_back(1);
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pFlodArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}

TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_008)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QHoverEvent* e = new QHoverEvent(QEvent::HoverLeave,QPointF(20.0,20.0),QPointF(30.0,30.0),Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    Stub s2;
    s2.set(ADDR(QList<int>,contains),rettruestub);
    Stub s3;
    s3.set(ADDR(QTextBlock,isVisible),rettruestub);
    Stub s4;
    s4.set(ADDR(TextEdit,getHideRowContent),rettruestub);
    Stub s5;
    s5.set(ADDR(TextEdit,getHighLightRowContentLineNum),retintstub);


    intvalue=4;
    edit->m_listMainFlodAllPos.push_back(1);
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pBookMarkArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}

TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_009)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QHoverEvent* e = new QHoverEvent(QEvent::HoverLeave,QPointF(20.0,20.0),QPointF(30.0,30.0),Qt::NoModifier);

    edit->m_rightMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    Stub s2;
    s2.set(ADDR(QList<int>,contains),rettruestub);
    Stub s3;
    s3.set(ADDR(QTextBlock,isVisible),rettruestub);
    Stub s4;
    s4.set(ADDR(TextEdit,getHideRowContent),rettruestub);
    Stub s5;
    s5.set(ADDR(TextEdit,getHighLightRowContentLineNum),retintstub);


    intvalue=4;
    edit->m_listMainFlodAllPos.push_back(1);
    edit->eventFilter(edit->m_pLeftAreaWidget->m_pFlodArea,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}


TEST(UT_Textedit_eventFilter, UT_Textedit_eventFilter_010)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QKeyEvent* e = new QKeyEvent(QEvent::KeyRelease,Qt::Key_Tab,Qt::NoModifier);

    edit->m_colorMarkMenu = new QMenu;
    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
    Stub s1;
    s1.set(A_foo,retintstub);

    Stub s2;
    s2.set(ADDR(QList<int>,contains),rettruestub);
    Stub s3;
    s3.set(ADDR(QTextBlock,isVisible),rettruestub);
    Stub s4;
    s4.set(ADDR(TextEdit,getHideRowContent),rettruestub);
    Stub s5;
    s5.set(ADDR(TextEdit,getHighLightRowContentLineNum),retintstub);


    intvalue=4;
    edit->m_listMainFlodAllPos.push_back(1);
    edit->eventFilter(edit->m_colorMarkMenu,e);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    edit->m_rightMenu->deleteLater();
    delete e;
    e = nullptr;
}


TEST(UT_Textedit_updateMark, UT_Textedit_updateMark_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QTextEdit::ExtraSelection e1,e2;
    //QList<QPair<QTextEdit::ExtraSelection, qint64>>;
    edit->m_wordMarkSelections.push_back({e1,1});
    edit->m_wordMarkSelections.push_back({e2,2});

    Stub s1;
    s1.set(ADDR(QTextCursor,selectionEnd),retintstub);
    Stub s2;
    s2.set(ADDR(QTextCursor,selectionStart),retintstub2);

    intvalue=10000;
    intvalue2=-10000;
    edit->m_bIsInputMethod = true;
    edit->updateMark(1,2,3);


    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_updateMark, UT_Textedit_updateMark_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QTextEdit::ExtraSelection e1,e2;
    //QList<QPair<QTextEdit::ExtraSelection, qint64>>;
    edit->m_wordMarkSelections.push_back({e1,1});
    edit->m_wordMarkSelections.push_back({e2,2});

    Stub s1;
    s1.set(ADDR(QTextCursor,selectionEnd),retintstub);
    Stub s2;
    s2.set(ADDR(QTextCursor,selectionStart),retintstub2);

    intvalue=10000;
    intvalue2=-10000;
    edit->m_bIsInputMethod = false;
    edit->updateMark(1,2,3);


    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}


TEST(UT_Textedit_updateMark, UT_Textedit_updateMark_004)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QTextEdit::ExtraSelection e1,e2;
    //QList<QPair<QTextEdit::ExtraSelection, qint64>>;
    edit->m_wordMarkSelections.push_back({e1,1});
    edit->m_wordMarkSelections.push_back({e2,2});
    edit->m_mapWordMarkSelections[1]={e1};

    Stub s1;
    s1.set(ADDR(QTextCursor,selectionEnd),retintstub);
    Stub s2;
    s2.set(ADDR(QTextCursor,selectionStart),retintstub2);
    Stub s3;
    s3.set(ADDR(QTextCursor,position),retintstub);

    intvalue=10000;
    intvalue2=10000;
    edit->m_bIsInputMethod = true;
    edit->updateMark(1,2,3);


    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_clearMarksForTextCursor, UT_Textedit_clearMarksForTextCursor_001)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QTextEdit::ExtraSelection e1,e2;
    //QList<QPair<QTextEdit::ExtraSelection, qint64>>;
    edit->m_wordMarkSelections.push_back({e1,1});
    edit->m_wordMarkSelections.push_back({e2,2});

    Stub s1;
    s1.set(ADDR(QTextCursor,hasSelection),rettruestub);

    edit->clearMarksForTextCursor();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}


TEST(UT_Textedit_clearMarksForTextCursor, UT_Textedit_clearMarksForTextCursor_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QTextEdit::ExtraSelection e1,e2;
    //QList<QPair<QTextEdit::ExtraSelection, qint64>>;
    edit->m_wordMarkSelections.push_back({e1,1});
    edit->m_wordMarkSelections.push_back({e2,2});

    Stub s0;
    s0.set(ADDR(QTextCursor,hasSelection),retfalsestub);

    Stub s1;
    s1.set(ADDR(QTextCursor,selectionEnd),retintstub);
    Stub s2;
    s2.set(ADDR(QTextCursor,selectionStart),retintstub2);
    Stub s3;
    s3.set(ADDR(QTextCursor,position),retintstub);

    intvalue=10;
    intvalue2=10;
    edit->clearMarksForTextCursor();

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}


TEST(UT_Textedit_clearMarkOperationForCursor, UT_Textedit_clearMarkOperationForCursor_001)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    TextEdit::MarkOperation e1,e2;
    //QList<QPair<QTextEdit::ExtraSelection, qint64>>;
    edit->m_markOperations.push_back({e1,1});
    edit->m_markOperations.push_back({e2,2});

    Stub s1;
    s1.set(ADDR(QTextCursor,hasSelection),rettruestub);

    edit->clearMarkOperationForCursor(e1.cursor);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
}


TEST(UT_Textedit_tapAndHoldGestureTriggered, UT_Textedit_tapAndHoldGestureTriggered_001)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QTapAndHoldGesture * t = new QTapAndHoldGesture();

    Stub s1;
    s1.set(ADDR(QTapAndHoldGesture,state),retintstub);

    intvalue = 0;
    edit->tapAndHoldGestureTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_tapAndHoldGestureTriggered, UT_Textedit_tapAndHoldGestureTriggered_002)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QTapAndHoldGesture * t = new QTapAndHoldGesture();

    Stub s1;
    s1.set(ADDR(QTapAndHoldGesture,state),retintstub);

    intvalue = 1;
    edit->tapAndHoldGestureTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_tapAndHoldGestureTriggered, UT_Textedit_tapAndHoldGestureTriggered_003)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QTapAndHoldGesture * t = new QTapAndHoldGesture();

    Stub s1;
    s1.set(ADDR(QTapAndHoldGesture,state),retintstub);

    intvalue = 2;
    edit->tapAndHoldGestureTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_tapAndHoldGestureTriggered, UT_Textedit_tapAndHoldGestureTriggered_004)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QTapAndHoldGesture * t = new QTapAndHoldGesture();

    Stub s1;
    s1.set(ADDR(QTapAndHoldGesture,state),retintstub);

    intvalue = 3;
    edit->tapAndHoldGestureTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_tapAndHoldGestureTriggered, UT_Textedit_tapAndHoldGestureTriggered_005)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QTapAndHoldGesture * t = new QTapAndHoldGesture();

    Stub s1;
    s1.set(ADDR(QTapAndHoldGesture,state),retintstub);

    intvalue = 4;
    edit->tapAndHoldGestureTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_panTriggered, UT_Textedit_panTriggered_001)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QPanGesture * t = new QPanGesture();

    Stub s1;
    s1.set(ADDR(QPanGesture,state),retintstub);

    intvalue = 4;
    edit->panTriggered(t);
    intvalue = 3;
    edit->panTriggered(t);
    intvalue = 2;
    edit->panTriggered(t);
    intvalue = 1;
    edit->panTriggered(t);
    intvalue = 0;
    edit->panTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}


TEST(UT_Textedit_pinchTriggered, UT_Textedit_pinchTriggered_001)
{
    Window* w = new Window;
    TextEdit* edit = new TextEdit(w);
    EditWrapper* wra = new EditWrapper(w);
    edit->m_wrapper = wra;
    QPinchGesture* t = new QPinchGesture();


    Stub s1;
    s1.set(ADDR(QPanGesture,state),retintstub);

    intvalue = 4;
    edit->pinchTriggered(t);
    intvalue = 3;
    edit->pinchTriggered(t);
    intvalue = 2;
    edit->pinchTriggered(t);
    intvalue = 1;
    edit->pinchTriggered(t);
    intvalue = 0;
    edit->pinchTriggered(t);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    w->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_swipeTriggered, UT_Textedit_swipeTriggered_001)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;
    QPinchGesture* t = new QPinchGesture();


    edit->swipeTriggered(nullptr);

    EXPECT_NE(edit , nullptr);
    edit->deleteLater();
    wra->deleteLater();
    t->deleteLater();
}

TEST(UT_Textedit_popRightMenu, UT_Textedit_popRightMenu_001)
{
//    Window* w = new Window;
//    TextEdit* edit = new TextEdit(w);
//    EditWrapper* wra = new EditWrapper(w);
//    edit->m_wrapper = wra;

//    edit->m_rightMenu = new QMenu;
//    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
//    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
//    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
//    Stub s1;
//    s1.set(A_foo,retintstub);

//    Stub s2;
//    s2.set(ADDR(QUndoStack,canUndo),rettruestub);
//    Stub s3;
//    s3.set(ADDR(QUndoStack,canRedo),rettruestub);
//    Stub s4;
//    s4.set(ADDR(QTextCursor,hasSelection),rettruestub);
//    Stub s5;
//    s5.set(ADDR(TextEdit,canPaste),rettruestub);
//    Stub s6;
//    s6.set(ADDR(QTextDocument,isEmpty),retfalsestub);
//    Stub s7;
//    s7.set(ADDR(TextEdit,characterCount),retintstub);
//    Stub s8;
//    s8.set(ADDR(QString,isEmpty),retfalsestub);
//    Stub s9;
//    s9.set(ADDR(TextEdit,renderAllSelections),rettruestub);
//    //s9.set(ADDR(QDBusConnection,call),rettruestub);
//     s9.set(ADDR(QDBusConnection,sessionBus),sessionBusstub);
//    //inline QDBusReply& operator=(const  QDBusMessage & dbusError)
//  //  s9.set((QDBusReply<bool>& (QDBusReply<bool>::*) (const QDBusMessage &))ADDR(QDBusReply<bool>,operator=), rettruestub);
//    //inline QDBusReply& operator=(const QDBusError& dbusError)
//  //   s9.set((QDBusReply<bool>& (QDBusReply<bool>::*) (const QDBusError &))ADDR(QDBusReply<bool>,operator=), rettruestub);

//    intvalue=1;
//    edit->m_bReadOnlyPermission = false;
//    edit->popRightMenu(QPoint(10,10));

//    EXPECT_NE(edit , nullptr);
//    edit->deleteLater();
//    wra->deleteLater();
//    w->deleteLater();
}


TEST(UT_Textedit_popRightMenu, UT_Textedit_popRightMenu_002)
{
//    Window* w = new Window;
//    TextEdit* edit = new TextEdit(w);
//    EditWrapper* wra = new EditWrapper(w);
//    edit->m_wrapper = wra;

//    edit->m_rightMenu = new QMenu;
//    edit->m_colorMarkMenu = new QMenu;
//    // QAction *exec(const QPoint &pos, QAction *at = nullptr);
//    typedef QAction * (*fptr)(QMenu*,const QPoint &, QAction *);
//    fptr A_foo = (fptr)((QAction *(QMenu::*)(const QPoint &, QAction *))&QMenu::exec);
//    Stub s1;
//    s1.set(A_foo,retintstub);

//    Stub s2;
//    s2.set(ADDR(QUndoStack,canUndo),rettruestub);
//    Stub s3;
//    s3.set(ADDR(QUndoStack,canRedo),rettruestub);
//    Stub s4;
//    s4.set(ADDR(QTextCursor,hasSelection),rettruestub);
//    Stub s5;
//    s5.set(ADDR(TextEdit,canPaste),rettruestub);
//    Stub s6;
//    s6.set(ADDR(QTextDocument,isEmpty),retfalsestub);
//    Stub s7;
//    s7.set(ADDR(TextEdit,characterCount),retintstub);
//    Stub s8;
//    s8.set(ADDR(QString,isEmpty),retfalsestub);
//    Stub s10;
//    s10.set(ADDR(QPoint,y),retintstub);
//    s10.set(ADDR(TextEdit,renderAllSelections),rettruestub);
//    s10.set(ADDR(QDBusConnection,sessionBus),sessionBusstub);
//    //s10.set((QDBusReply<bool>& (QDBusReply<bool>::*) (const QDBusMessage &))ADDR(QDBusReply<bool>,operator=), rettruestub);

//    TextEdit::MarkOperation m1,m2;
//    edit->m_markOperations.push_back({m1,1});
//    edit->m_markOperations.push_back({m2,2});
//    intvalue=1;
//    edit->m_bReadOnlyPermission = false;
//    edit->popRightMenu(QPoint(10,10));

//    EXPECT_NE(edit , nullptr);
//    edit->deleteLater();
//    wra->deleteLater();
//    w->deleteLater();
}

TEST(UT_Textedit_unindentText, UT_Textedit_unindentText)
{
    Window* w = new Window;
    TextEdit* edit = new TextEdit(w);
    EditWrapper* wra = new EditWrapper(w);
    edit->m_wrapper = wra;

    edit->unindentText();

    EXPECT_NE(edit,nullptr);
    edit->deleteLater();
    wra->deleteLater();
    w->deleteLater();
}

// static QList<QPair<MarkOperation, qint64> > convertMarkFromReplace(const QList<MarkReplaceInfo> &replaceInfo);
TEST(UT_Textedit_convertMark, UT_Textedit_convertMarkToReplace)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("12345\n"
                       "12345\n"
                       "12345\n");

    QList<QPair<TextEdit::MarkOperation, qint64> > markInfo;
    TextEdit::MarkOperation opt;
    opt.type = TextEdit::MarkOnce;
    opt.color = "#E5E5E5";
    opt.cursor = edit->textCursor();
    opt.cursor.setPosition(1);
    opt.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 5);
    opt.matchText = opt.cursor.selectedText();
    markInfo.append(qMakePair(opt, 100));

    TextEdit::MarkOperation opt2;
    markInfo.append(qMakePair(opt2, 200));

    auto replaceInfo = TextEdit::convertMarkToReplace(markInfo);
    EXPECT_EQ(replaceInfo.size(), markInfo.size());

    if (replaceInfo.size() == markInfo.size()) {
        for (int i = 0; i < replaceInfo.size(); ++i) {
            auto &mark = markInfo[i];
            auto &replace = replaceInfo[i];

            EXPECT_EQ(mark.first.type, replace.opt.type);
            EXPECT_EQ(mark.first.color, replace.opt.color);
            EXPECT_EQ(mark.first.cursor.selectionStart(), replace.start);
            EXPECT_EQ(mark.first.cursor.selectionEnd(), replace.end);
            EXPECT_EQ(mark.second, replace.time);
        }
    }

    edit->deleteLater();
    wra->deleteLater();
}

// static QList<MarkReplaceInfo> convertReplaceToMark(const QList<QPair<MarkOperation, qint64> > &markInfo);
TEST(UT_Textedit_convertMark, UT_Textedit_convertReplaceToMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("12345\n"
                       "12345\n"
                       "12345\n");

    QList<TextEdit::MarkReplaceInfo> replaceInfo;
    TextEdit::MarkReplaceInfo replace1;
    replace1.opt.type = TextEdit::MarkOnce;
    replace1.opt.color = "#E5E5E5";
    replace1.opt.cursor = edit->textCursor();
    replace1.start = 1;
    replace1.end = 6;
    replace1.time = INT_MAX;
    replaceInfo.append(replace1);

    TextEdit::MarkReplaceInfo replace2;
    replaceInfo.append(replace2);

    auto markInfo = TextEdit::convertReplaceToMark(replaceInfo);
    EXPECT_EQ(replaceInfo.size(), markInfo.size());

    if (replaceInfo.size() == markInfo.size()) {
        for (int i = 0; i < replaceInfo.size(); ++i) {
            auto &mark = markInfo[i];
            auto &replace = replaceInfo[i];

            EXPECT_EQ(mark.first.type, replace.opt.type);
            EXPECT_EQ(mark.first.color, replace.opt.color);
            if (!mark.first.cursor.isNull()) {
                EXPECT_EQ(mark.first.cursor.selectionStart(), replace.start);
                EXPECT_EQ(mark.first.cursor.selectionEnd(), replace.end);
            }
            EXPECT_EQ(mark.second, replace.time);
        }
    }

    edit->deleteLater();
    wra->deleteLater();
}

// 替换文本在颜色标记左侧
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithRightMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QStringList markList {"45", "89\n", "hi\n1a", "d5\n"};

    for (auto markText : markList) {
        edit->setPlainText("123456789\n"
                           "abcdefghi\n"
                           "1a2b3c4d5\n");

        QTextCursor cursor = edit->findCursor(markText, edit->toPlainText(), 0);
        // note: 含跨行符时，Qt自动替换换行为Unicode U+2029，详见QTextCursor::selectedText()帮助
        EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), markText);
        int oldStart = cursor.selectionStart();
        int oldEnd = cursor.selectionEnd();

        edit->setTextCursor(cursor);
        edit->isMarkCurrentLine(true, "#E5E5E5");
        edit->renderAllSelections();
        EXPECT_FALSE(edit->extraSelections().isEmpty());

        QString replaceText = "12";
        QString withText = "abcd";
        edit->replaceAll(replaceText, withText);

        QTextCursor newCursor = edit->textCursor();
        newCursor.setPosition(oldStart + withText.size() - replaceText.size());
        newCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, oldEnd - oldStart);
        QColor cursorColor = newCursor.charFormat().background().color();
        EXPECT_EQ(markText, newCursor.selectedText().replace("\u2029", "\n"));

        // 使用拓展标记判断颜色标记
        bool find = false;
        for (auto extra : edit->extraSelections()) {
            if (extra.cursor.selectionStart() == newCursor.selectionStart()) {
                EXPECT_EQ(extra.cursor.selectionEnd(), newCursor.selectionEnd());
                EXPECT_EQ(extra.format.background().color(), QColor("#E5E5E5"));

                find = true;
                break;
            }
        }
        EXPECT_TRUE(find);

        edit->slotCancleMarkAllLine(false);
    }

    edit->deleteLater();
    wra->deleteLater();
}

// 替换文本和颜色标记存在交集
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithIntersectMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n"
                       "abcdefghi\n"
                       "1a2b3c4d5\n");
    QString strColor = "#E5E5E5";
    // 选中 "456"
    QString markText = "456";
    QTextCursor cursor = edit->findCursor(markText, edit->toPlainText(), 0);
    // note: 含跨行符时，Qt自动替换换行为Unicode U+2029，详见QTextCursor::selectedText()帮助
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), markText);

    edit->setTextCursor(cursor);
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->extraSelections().isEmpty());

    QString replaceText = "567";
    QString withText = "x567x";
    // "1234x567x89\n"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("4x567x"));
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    replaceText = "234";
    withText = "-234-";
    // "1-234-x567x89\n"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("-234-x567x"));
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    replaceText = "x89\n";
    withText = "x89\n123";
    // "1-234-x567x89\n123"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("-234-x567x89\n123"));
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    edit->deleteLater();
    wra->deleteLater();
}

// 替换文本和颜色标记处于边界，不应互相影响
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithBoundnaryMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n");
    QString strColor = "#E5E5E5";
    // 选中 "456"
    QString markText = "456";
    QTextCursor cursor = edit->findCursor(markText, edit->toPlainText(), 0);
    // note: 含跨行符时，Qt自动替换换行为Unicode U+2029，详见QTextCursor::selectedText()帮助
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), markText);

    edit->setTextCursor(cursor);
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->extraSelections().isEmpty());

    QString replaceText = "123";
    QString withText = "x123x";
    // "x123x456789\n"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("456"));
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    replaceText = "789";
    withText = "x789x";
    // "x123x456x789x\n"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("456"));
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    edit->deleteLater();
    wra->deleteLater();
}

// 颜色标记有多组替换文本
TEST(UT_Textedit_replaceWithMark, UT_Textedit_markWithMultiReplace)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123123123123\n");
    QString strColor = "#E5E5E5";
    // 选中 "31231231"
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(2);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 8);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), QString("31231231"));

    edit->setTextCursor(cursor);
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->extraSelections().isEmpty());

    QString replaceText = "123";
    QString withText = "x123x";
    // "x123xx123xx123xx123x\n"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("x123xx123xx123xx123x"));
        EXPECT_EQ(markInfo.first.cursor.selectionStart(), 0);
        EXPECT_EQ(markInfo.first.cursor.selectionEnd(), selectText.size());
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    // 重置
    edit->slotCancleMarkAllLine();
    edit->setPlainText("xxxxxx\n");
    // 选中中间的两组x
    cursor.setPosition(2);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 2);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), QString("xx"));
    edit->setTextCursor(cursor);
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->extraSelections().isEmpty());

    replaceText = "x";
    withText = "xy";
    // "xyxyxyxyxyxy\n"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("xyxy"));
        EXPECT_EQ(markInfo.first.cursor.selectionStart(), 4);
        EXPECT_EQ(markInfo.first.cursor.selectionEnd(), 8);
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    edit->deleteLater();
    wra->deleteLater();
}


// 替换文本在单行颜色标记内部
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithLineMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n"
                       "abcdefghi\n"
                       "1a2b3c4d5\n");
    QString strColor = "#E5E5E5";
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(0);

    edit->setTextCursor(cursor);
    // 选中首行 "123456789" 不包括换行符
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());

    QString replaceText = "12";
    QString withText = "xxxx";
    // "xxxx3456789"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());

    auto checkSelection = [&](const QString &cmpStr)->bool {
        bool find = false;
        for (auto selection : edit->m_wordMarkSelections) {
            QString selectText = selection.first.cursor.selectedText().replace("\u2029", "\n");
            if (selectText == cmpStr) {
                EXPECT_EQ(selection.first.format.background().color(), QColor(strColor));
                find = true;
                break;
            }
        }
        return find;
    };
    EXPECT_TRUE(checkSelection("xxxx3456789"));

    replaceText = "xxxx";
    withText = "12";
    // "123456789"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    EXPECT_TRUE(checkSelection("123456789"));

    replaceText = "6789\nabcd";
    withText = "";
    // "12345efghi"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    EXPECT_TRUE(checkSelection("12345efghi"));

    edit->deleteLater();
    wra->deleteLater();
}

// 替换文本跨过了单行颜色标记，形成多行颜色标记
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithMultiLineMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n"
                       "abcdefghi\n"
                       "1a2b3c4d5\n");
    QString strColor = "#E5E5E5";
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(0);

    edit->setTextCursor(cursor);
    // 选中首行 "123456789" 不包括换行符
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());

    QString replaceText = "34";
    QString withText = "34\n34";
    // "1234" -> "1234\n34"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());
    EXPECT_EQ(edit->m_wordMarkSelections.size(), 2);

    edit->deleteLater();
    wra->deleteLater();
}

// 替换文本包含了颜色标记
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithInnerMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n"
                       "abcdefghi\n");
    QString strColor = "#E5E5E5";
    QTextCursor cursor = edit->textCursor();
    // 选中"6789\nabcd"
    cursor.setPosition(5);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 9);

    edit->setTextCursor(cursor);
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());

    QString replaceText = "456789\nabcdef";
    QString withText = "";
    // 替换后不存在标记 "" 将被移除
    edit->replaceAll(replaceText, withText);
    EXPECT_TRUE(edit->m_wordMarkSelections.isEmpty());

    edit->deleteLater();
    wra->deleteLater();
}

// 替换文本在颜色标记内部
TEST(UT_Textedit_replaceWithMark, UT_Textedit_replaceWithOutterMark)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n"
                       "abcdefghi\n");
    QString strColor = "#E5E5E5";
    QTextCursor cursor = edit->textCursor();
    // 选中"6789\nabcd"
    cursor.setPosition(5);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 9);

    edit->setTextCursor(cursor);
    edit->isMarkCurrentLine(true, strColor);
    edit->renderAllSelections();
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());

    QString replaceText = "89\n";
    QString withText = "";
    // "67abcd"
    edit->replaceAll(replaceText, withText);
    EXPECT_FALSE(edit->m_wordMarkSelections.isEmpty());

    if (!edit->m_wordMarkSelections.isEmpty()) {
        auto markInfo = edit->m_wordMarkSelections.first();
        QString selectText = markInfo.first.cursor.selectedText().replace("\u2029", "\n");
        EXPECT_EQ(selectText, QString("67abcd"));
        EXPECT_EQ(markInfo.first.format.background().color(), QColor(strColor));
    }

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_selectText, SelectText_SingleBlock_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n");
    // 选中 "456"
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(3);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 3);
    edit->setTextCursor(cursor);

    EXPECT_EQ(cursor.selectedText(), edit->selectedText());
    EXPECT_EQ(QString("456"), edit->selectedText());

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_selectText, SelectText_MultiBlock_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->setPlainText("123456789\n"
                       "abcdefghi\n"
                       "123456789\n");
    // 选中 "789\nabc"
    QString str1 = QString("789\nabc");
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(6);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, str1.length());
    edit->setTextCursor(cursor);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), edit->selectedText());
    EXPECT_EQ(str1, edit->selectedText());

    // 选中 "789\nabcdefghi\n123"
    QString str2 = QString("789\nabcdefghi\n123");
    cursor.setPosition(6);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, str2.length());
    edit->setTextCursor(cursor);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), edit->selectedText());
    EXPECT_EQ(str2, edit->selectedText());

    // 选中 "789\nabcdefghi\n123456789\n"
    QString str3 = QString("789\nabcdefghi\n123456789\n");
    cursor.setPosition(6);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, str3.length());
    edit->setTextCursor(cursor);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), edit->selectedText());
    EXPECT_EQ(str3, edit->selectedText());

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_selectText, SelectText_MultiBlock_MultiByte_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    // 此字符串为特殊多字节编码 CJK
    // "𢝐𢝑𢝒𢝓𢝔𢝕𢝖𢝗𢝘𢝙\n𢝚𢝛𢝜𢝝𢝞𢝟𢝠𢝡𢝢𢝣\n𢝤𢝥𢝦𢝧𢝨𢝩𢝪𢝫𢝬𢝭\n"
    QString multiByteText("\360\242\235\220\360\242\235\221\360\242\235\222\360\242\235\223\360\242\235\224"
                          "\360\242\235\225\360\242\235\226\360\242\235\227\360\242\235\230\360\242\235\231\n"
                          "\360\242\235\232\360\242\235\233\360\242\235\234\360\242\235\235\360\242\235\236"
                          "\360\242\235\237\360\242\235\240\360\242\235\241\360\242\235\242\360\242\235\243\n"
                          "\360\242\235\244\360\242\235\245\360\242\235\246\360\242\235\247\360\242\235\250"
                          "\360\242\235\251\360\242\235\252\360\242\235\253\360\242\235\254\360\242\235\255\n");
    edit->setPlainText(multiByteText);
    // 选中 "𢝘𢝙\n𢝚𢝛"
    QString str1 = QString("\360\242\235\230\360\242\235\231\n\360\242\235\232\360\242\235\233");
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(16);
    cursor.setPosition(16 + str1.length(), QTextCursor::KeepAnchor);
    edit->setTextCursor(cursor);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), edit->selectedText());
    EXPECT_EQ(str1, edit->selectedText());

    // 选中 "𢝘𢝙\n𢝚𢝛𢝜𢝝𢝞𢝟𢝠𢝡𢝢𢝣\n𢝤𢝥"
    QString str2 = multiByteText.mid(16, 30);
    cursor.setPosition(16);
    cursor.setPosition(16 + str2.length(), QTextCursor::KeepAnchor);
    edit->setTextCursor(cursor);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), edit->selectedText());
    EXPECT_EQ(str2, edit->selectedText());

    // 选中 "𢝘𢝙\n𢝚𢝛𢝜𢝝𢝞𢝟𢝠𢝡𢝢𢝣\n𢝤𢝥𢝦𢝧𢝨𢝩𢝪𢝫𢝬𢝭\n"
    QString str3 = multiByteText.mid(16);
    cursor.setPosition(16);
    cursor.setPosition(16 + str3.length(), QTextCursor::KeepAnchor);
    edit->setTextCursor(cursor);
    EXPECT_EQ(cursor.selectedText().replace("\u2029", "\n"), edit->selectedText());
    EXPECT_EQ(str3, edit->selectedText());
    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_MidButtonInsertText, onTextContentChanged_MidButtonInsertText_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QString sourceText("123456789");
    edit->setPlainText(sourceText);
    edit->m_MidButtonPatse = true;

    QString insertText("test");
    QTextCursor cursor = edit->textCursor();
    cursor.setPosition(4);
    // 插入触发 onTextContentChanged()
    cursor.insertText(insertText);

    EXPECT_TRUE(edit->m_pUndoStack->canUndo());
    edit->m_pUndoStack->undo();
    EXPECT_EQ(edit->toPlainText(), sourceText);

    cursor = edit->textCursor();
    EXPECT_EQ(cursor.position(), 4);

    EXPECT_FALSE(edit->m_MidButtonPatse);

    edit->deleteLater();
    wra->deleteLater();
}

DPalette stubApplicationPalette()
{
    DPalette pa;
    pa.setColor(DPalette::Highlight, QColor(Qt::red));
    return pa;
}

TEST(UT_Textedit_onAppPaletteChanged, OnAppPaletteChanged_ChangeBackground_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    Stub s;
    s.set(ADDR(DGuiApplicationHelper, applicationPalette), stubApplicationPalette);

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QBrush(Qt::white));
    edit->m_altModSelections.append(selection);
    edit->m_bIsAltMod = true;
    edit->onAppPaletteChanged();

    ASSERT_FALSE(edit->m_altModSelections.isEmpty());
    QBrush background = edit->m_altModSelections.first().format.background();
    EXPECT_EQ(QColor(Qt::red), background.color());

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_MoveText, moveText_Move_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QString sourceText("123456789\n");
    edit->setPlainText(sourceText);

    edit->moveText(0, 10, "12");
    EXPECT_EQ(edit->toPlainText(), QString("3456789\n12"));
    edit->undo_();
    EXPECT_EQ(edit->toPlainText(), sourceText);
    edit->redo_();
    EXPECT_EQ(edit->toPlainText(), QString("3456789\n12"));

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_MoveText, moveText_Copy_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    QString sourceText("123456789\n");
    edit->setPlainText(sourceText);

    edit->moveText(0, 10, "12", true);
    EXPECT_EQ(edit->toPlainText(), QString("123456789\n12"));
    edit->undo_();
    EXPECT_EQ(edit->toPlainText(), sourceText);
    edit->redo_();
    EXPECT_EQ(edit->toPlainText(), QString("123456789\n12"));

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_MoveText, moveText_Multi_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->insertTextEx(edit->textCursor(), "1\n");
    edit->insertTextEx(edit->textCursor(), "2\n");
    EXPECT_EQ(edit->toPlainText(), QString("1\n2\n"));

    edit->moveText(2, 1, "2");
    EXPECT_EQ(edit->toPlainText(), QString("12\n\n"));
    edit->moveText(0, 4, "12");
    EXPECT_EQ(edit->toPlainText(), QString("\n\n12"));
    edit->moveText(2, 0, "12", true);
    EXPECT_EQ(edit->toPlainText(), QString("12\n\n12"));

    while(edit->m_pUndoStack->canUndo()) {
        edit->undo_();
    }
    EXPECT_TRUE(edit->toPlainText().isEmpty());
    while(edit->m_pUndoStack->canRedo()) {
        edit->redo_();
    }
    EXPECT_EQ(edit->toPlainText(), QString("12\n\n12"));

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_onPressedLineNumber, onPressedLineNumber_BoundaryCheck_Pass)
{
    // 边界检查
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->onPressedLineNumber(QPoint(0, 0));
    EXPECT_EQ(edit->textCursor().position(), 0);

    edit->onPressedLineNumber(QPoint(0, 100));
    EXPECT_EQ(edit->textCursor().position(), 0);

    edit->insertTextEx(edit->textCursor(), "123\n222\n333");
    auto cursor = edit->textCursor();
    cursor.setPosition(0);
    edit->setTextCursor(cursor);
    edit->onPressedLineNumber(QPoint(-1, -1));
    EXPECT_EQ(edit->textCursor().position(), 0);
    EXPECT_TRUE(edit->textCursor().selectedText().isEmpty());

    edit->onPressedLineNumber(QPoint(INT_MAX, INT_MAX));
    EXPECT_EQ(edit->textCursor().position(), 0);
    EXPECT_TRUE(edit->textCursor().selectedText().isEmpty());

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_onPressedLineNumber, onPressedLineNumber_MultiBlock_Pass)
{
    // 多行不同位置选取
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->insertTextEx(edit->textCursor(), "123\n222\n333");
    QTextCursor cursor = edit->textCursor();

    QTextBlock selectBlock = edit->document()->firstBlock();
    cursor.setPosition(selectBlock.position(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QRect rect = edit->cursorRect(cursor);
    edit->onPressedLineNumber(QPoint(rect.left(), rect.center().y()));
    EXPECT_EQ(edit->textCursor().selectedText().replace("\u2029", "\n"), QString("123\n"));

    selectBlock = edit->document()->lastBlock();
    cursor.setPosition(selectBlock.position(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    rect = edit->cursorRect(cursor);
    edit->onPressedLineNumber(QPoint(rect.left(), rect.center().y()));
    EXPECT_EQ(edit->textCursor().selectedText().replace("\u2029", "\n"), QString("333"));

    edit->deleteLater();
    wra->deleteLater();
}

TEST(UT_Textedit_onPressedLineNumber, onPressedLineNumber_OutOfBlock_Pass)
{
    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->insertTextEx(edit->textCursor(), "123\n222\n333");
    QTextCursor cursor = edit->textCursor();

    QTextBlock selectBlock = edit->document()->lastBlock();
    cursor.setPosition(selectBlock.position(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QRect rect = edit->cursorRect(cursor);
    // 超过文档范围选取将锁定尾行
    edit->onPressedLineNumber(QPoint(rect.left(), rect.center().y() + rect.height()));
    EXPECT_EQ(edit->textCursor().selectedText().replace("\u2029", "\n"), QString("333"));

    edit->deleteLater();
    wra->deleteLater();
}

QString stub_getSystemLan()
{
    return "bo_CN";
}

TEST(UT_Textedit_onPressedLineNumber, onPressedLineNumber_LanBo_CN_Pass)
{
    // bo_CN 语言环境调整偏移量，光标向下偏移2
    Stub stub;
    stub.set(ADDR(Utils, getSystemLan), stub_getSystemLan);

    TextEdit* edit = new TextEdit;
    EditWrapper* wra = new EditWrapper;
    edit->m_wrapper = wra;

    edit->insertTextEx(edit->textCursor(), "123\n222\n333");
    QTextCursor cursor = edit->textCursor();

    QTextBlock selectBlock = edit->document()->firstBlock();
    cursor.setPosition(selectBlock.position(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    QRect rect = edit->cursorRect(cursor);
    edit->onPressedLineNumber(QPoint(rect.left(), rect.bottom() + 2));
    EXPECT_EQ(edit->textCursor().selectedText().replace("\u2029", "\n"), QString("123\n"));

    selectBlock = selectBlock.next();
    cursor.setPosition(selectBlock.position(), QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    rect = edit->cursorRect(cursor);
    edit->onPressedLineNumber(QPoint(rect.left(), rect.bottom() + 2));
    EXPECT_EQ(edit->textCursor().selectedText().replace("\u2029", "\n"), QString("222\n"));

    edit->deleteLater();
    wra->deleteLater();
}

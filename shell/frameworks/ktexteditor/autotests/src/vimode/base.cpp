/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "base.h"
#include "vimode/globalstate.h"
#include "vimode/macros.h"
#include "vimode/mappings.h"
#include <inputmode/kateviinputmode.h>
#include <kateconfig.h>
#include <katedocument.h>
#include <kateundomanager.h>
#include <kateview.h>
#include <vimode/emulatedcommandbar/emulatedcommandbar.h>

#include <QMainWindow>
#include <QTest>
#include <QVBoxLayout>
#include <inputmodemanager.h>

using namespace KateVi;
using namespace KTextEditor;

// BEGIN: BaseTest

BaseTest::BaseTest()
{
    // test mode with e.g. own temporary config files
    QStandardPaths::setTestModeEnabled(true);

    kate_view = nullptr;
    kate_document = nullptr;

    mainWindow = new QMainWindow;
    auto centralWidget = new QWidget();
    mainWindowLayout = new QVBoxLayout(centralWidget);
    mainWindow->setCentralWidget(centralWidget);
    mainWindow->resize(640, 480);

    m_codesToModifiers = {
        {QStringLiteral("ctrl"), CONTROL_MODIFIER},
        {QStringLiteral("alt"), Qt::AltModifier},
        {QStringLiteral("meta"), META_MODIFIER},
        {QStringLiteral("keypad"), Qt::KeypadModifier},
    };

    m_codesToSpecialKeys = {
        {QStringLiteral("backspace"), Qt::Key_Backspace},
        {QStringLiteral("esc"), Qt::Key_Escape},
        {QStringLiteral("return"), Qt::Key_Return},
        {QStringLiteral("enter"), Qt::Key_Enter},
        {QStringLiteral("left"), Qt::Key_Left},
        {QStringLiteral("right"), Qt::Key_Right},
        {QStringLiteral("up"), Qt::Key_Up},
        {QStringLiteral("down"), Qt::Key_Down},
        {QStringLiteral("home"), Qt::Key_Home},
        {QStringLiteral("end"), Qt::Key_End},
        {QStringLiteral("delete"), Qt::Key_Delete},
        {QStringLiteral("insert"), Qt::Key_Insert},
        {QStringLiteral("pageup"), Qt::Key_PageUp},
        {QStringLiteral("pagedown"), Qt::Key_PageDown},
    };
}

BaseTest::~BaseTest()
{
    delete kate_document;
}

void BaseTest::waitForCompletionWidgetToActivate(KTextEditor::ViewPrivate *kate_view)
{
    const QDateTime start = QDateTime::currentDateTime();
    while (start.msecsTo(QDateTime::currentDateTime()) < 1000) {
        if (kate_view->isCompletionActive()) {
            break;
        }
        QApplication::processEvents();
    }
    QVERIFY(kate_view->isCompletionActive());
}

void BaseTest::init()
{
    delete kate_view;
    delete kate_document;

    kate_document = new KTextEditor::DocumentPrivate();

    // fixed indentation options
    kate_document->config()->setTabWidth(8);
    kate_document->config()->setIndentationWidth(2);
    kate_document->config()->setReplaceTabsDyn(false);

    // ensure the spellchecking doesn't mess with the expected results
    static_cast<KTextEditor::DocumentPrivate *>(kate_document)->setDefaultDictionary(QStringLiteral("notexistinglanguage"));

    kate_view = new KTextEditor::ViewPrivate(kate_document, mainWindow);
    mainWindowLayout->addWidget(kate_view);
    kate_view->config()->setValue(KateViewConfig::AutoBrackets, false);
    kate_view->setInputMode(View::ViInputMode);
    Q_ASSERT(kate_view->currentInputMode()->viewInputMode() == KTextEditor::View::ViInputMode);
    vi_input_mode = static_cast<KateViInputMode *>(kate_view->currentInputMode());
    vi_input_mode_manager = vi_input_mode->viInputModeManager();
    Q_ASSERT(vi_input_mode_manager);
    vi_global = vi_input_mode->globalState();
    Q_ASSERT(vi_global);
    kate_document->config()->setShowSpaces(KateDocumentConfig::Trailing); // Flush out some issues in the KateRenderer when rendering spaces.
    kate_view->config()->setValue(KateViewConfig::ShowScrollBarMiniMap, false);
    kate_view->config()->setValue(KateViewConfig::ShowScrollBarPreview, false);
    kate_view->setStatusBarEnabled(false);

    connect(kate_document, &KTextEditor::DocumentPrivate::textInsertedRange, this, &BaseTest::textInserted);
    connect(kate_document, &KTextEditor::DocumentPrivate::textRemoved, this, &BaseTest::textRemoved);
}

void BaseTest::TestPressKey(const QString &str)
{
    if (m_firstBatchOfKeypressesForTest) {
        qDebug() << "\n\n>>> running command " << str << " on text " << kate_document->text();
    } else {
        qDebug() << "\n>>> running further keypresses " << str << " on text " << kate_document->text();
    }
    m_firstBatchOfKeypressesForTest = false;

    for (int i = 0; i < str.length(); i++) {
        Qt::KeyboardModifiers keyboard_modifier = Qt::NoModifier;
        QString key;
        int keyCode = -1;
        // Looking for keyboard modifiers
        if (str[i] == QLatin1Char('\\')) {
            int endOfModifier = -1;
            Qt::KeyboardModifier parsedModifier = parseCodedModifier(str, i, &endOfModifier);
            int endOfSpecialKey = -1;
            Qt::Key parsedSpecialKey = parseCodedSpecialKey(str, i, &endOfSpecialKey);
            if (parsedModifier != Qt::NoModifier) {
                keyboard_modifier = parsedModifier;
                // Move to the character after the '-' in the modifier.
                i = endOfModifier + 1;
                // Is this a modifier plus special key?
                int endOfSpecialKeyAfterModifier = -1;
                const Qt::Key parsedCodedSpecialKeyAfterModifier = parseCodedSpecialKey(str, i, &endOfSpecialKeyAfterModifier);
                if (parsedCodedSpecialKeyAfterModifier != Qt::Key_unknown) {
                    key = parsedCodedSpecialKeyAfterModifier <= 0xffff ? QString(QChar(parsedCodedSpecialKeyAfterModifier)) : QString();
                    keyCode = parsedCodedSpecialKeyAfterModifier;
                    i = endOfSpecialKeyAfterModifier;
                }
            } else if (parsedSpecialKey != Qt::Key_unknown) {
                key = parsedSpecialKey <= 0xffff ? QString(QChar(parsedSpecialKey)) : QString();
                keyCode = parsedSpecialKey;
                i = endOfSpecialKey;
            } else if (str.mid(i, 2) == QStringLiteral("\\:")) {
                int start_cmd = i + 2;
                for (i += 2; true; i++) {
                    if (str.at(i) == QLatin1Char('\\')) {
                        if (i + 1 < str.length() && str.at(i + 1) == QLatin1Char('\\')) {
                            // A backslash within a command; skip.
                            i += 2;
                        } else {
                            // End of command.
                            break;
                        }
                    }
                }
                const QString commandToExecute = str.mid(start_cmd, i - start_cmd).replace(QLatin1String("\\\\"), QLatin1String("\\"));
                qDebug() << "Executing command directly from ViModeTest:\n" << commandToExecute;
                vi_input_mode->viModeEmulatedCommandBar()->executeCommand(commandToExecute);
                // We've handled the command; go back round the loop, avoiding sending
                // the closing \ to vi_input_mode_manager.
                continue;
            } else if (str.mid(i, 2) == QStringLiteral("\\\\")) {
                key = QStringLiteral("\\");
                keyCode = Qt::Key_Backslash;
                i++;
            } else {
                Q_ASSERT(false); // Do not use "\" in tests except for modifiers, command mode (\\:) and literal backslashes "\\\\")
            }
        }

        if (keyCode == -1) {
            key = str[i];
            keyCode = key[0].unicode();
            // Kate Vim mode's internals identifier e.g. CTRL-C by Qt::Key_C plus the control modifier,
            // so we need to translate e.g. 'c' to Key_C.
            if (key[0].isLetter()) {
                if (key[0].toLower() == key[0]) {
                    keyCode = keyCode - 'a' + Qt::Key_A;
                } else {
                    keyCode = keyCode - 'A' + Qt::Key_A;
                    keyboard_modifier |= Qt::ShiftModifier;
                }
            }
        }

        QKeyEvent key_event(QEvent::KeyPress, keyCode, keyboard_modifier, key);
        // Attempt to simulate how Qt usually sends events - typically, we want to send them
        // to kate_view->focusProxy() (which is a KateViewInternal).
        QWidget *destWidget = nullptr;
        if (QApplication::activePopupWidget()) {
            // According to the docs, the activePopupWidget, if present, takes all events.
            destWidget = QApplication::activePopupWidget();
        } else if (QApplication::focusWidget()) {
            if (QApplication::focusWidget()->focusProxy()) {
                destWidget = QApplication::focusWidget()->focusProxy();
            } else {
                destWidget = QApplication::focusWidget();
            }
        } else {
            destWidget = kate_view->focusProxy();
        }
        QApplication::sendEvent(destWidget, &key_event);
    }
}

void BaseTest::BeginTest(const QString &original)
{
    vi_input_mode->viInputModeManager()->viEnterNormalMode();
    vi_input_mode->reset();
    vi_input_mode_manager = vi_input_mode->viInputModeManager();
    kate_document->setText(original);
    kate_document->undoManager()->clearUndo();
    kate_document->undoManager()->clearRedo();
    kate_view->setCursorPosition(Cursor(0, 0));
    m_firstBatchOfKeypressesForTest = true;
}

void BaseTest::FinishTest_(int line, const char *file, const char *expected, Expectation expectation, const char *failureReason)
{
    if (expectation == ShouldFail) {
        if (!QTest::qExpectFail("", failureReason, QTest::Continue, file, line)) {
            return;
        }
        qDebug() << "Actual text:\n\t" << kate_document->text() << "\nShould be (for this test to pass):\n\t" << expected;
    }
    if (!QTest::qCompare(kate_document->text(), QString::fromUtf8(expected), "kate_document->text()", "expected_text", file, line)) {
        return;
    }
    Q_ASSERT(!emulatedCommandBarTextEdit()->isVisible() && "Make sure you close the command bar before the end of a test!");
}

void BaseTest::DoTest_(int line,
                       const char *file,
                       const char *original,
                       const char *command,
                       const char *expected,
                       Expectation expectation,
                       const char *failureReason)
{
    BeginTest(QString::fromUtf8(original));
    TestPressKey(QString::fromUtf8(command));
    FinishTest_(line, file, expected, expectation, failureReason);
}

void BaseTest::DoTest2(int line,
                       const char *file,
                       const QString &original,
                       const QString &command,
                       const QString &expected,
                       Expectation expectation,
                       const char *failureReason)
{
    BeginTest(original);
    TestPressKey(command);
    FinishTest_(line, file, expected.toUtf8().constData(), expectation, failureReason);
}

Qt::KeyboardModifier BaseTest::parseCodedModifier(const QString &string, int startPos, int *destEndOfCodedModifier)
{
    for (const auto &[code, modifier] : m_codesToModifiers) {
        // The "+2" is from the leading '\' and the trailing '-'
        if (string.mid(startPos, code.length() + 2) == QStringLiteral("\\") + code + QStringLiteral("-")) {
            if (destEndOfCodedModifier) {
                // destEndOfCodeModifier lies on the trailing '-'.
                *destEndOfCodedModifier = startPos + code.length() + 1;
                Q_ASSERT(string[*destEndOfCodedModifier] == QLatin1Char('-'));
            }
            return modifier;
        }
    }
    return Qt::NoModifier;
}

Qt::Key BaseTest::parseCodedSpecialKey(const QString &string, int startPos, int *destEndOfCodedKey)
{
    for (const auto &[specialKeyCode, specialKey] : m_codesToSpecialKeys) {
        // "+1" is for the leading '\'.
        if (string.mid(startPos, specialKeyCode.length() + 1) == QStringLiteral("\\") + specialKeyCode) {
            if (destEndOfCodedKey) {
                *destEndOfCodedKey = startPos + specialKeyCode.length();
            }
            return specialKey;
        }
    }
    return Qt::Key_unknown;
}

KateVi::EmulatedCommandBar *BaseTest::emulatedCommandBar()
{
    KateVi::EmulatedCommandBar *emulatedCommandBar = vi_input_mode->viModeEmulatedCommandBar();
    Q_ASSERT(emulatedCommandBar);
    return emulatedCommandBar;
}

QLineEdit *BaseTest::emulatedCommandBarTextEdit()
{
    QLineEdit *emulatedCommandBarText = emulatedCommandBar()->findChild<QLineEdit *>(QStringLiteral("commandtext"));
    Q_ASSERT(emulatedCommandBarText);
    return emulatedCommandBarText;
}

void BaseTest::ensureKateViewVisible()
{
    mainWindow->show();
    kate_view->show();
    QApplication::setActiveWindow(mainWindow);
    kate_view->setFocus();
    const QDateTime startTime = QDateTime::currentDateTime();
    while (startTime.msecsTo(QDateTime::currentDateTime()) < 3000 && !mainWindow->isActiveWindow()) {
        QApplication::processEvents();
    }
    QVERIFY(kate_view->isVisible());
    QVERIFY(mainWindow->isActiveWindow());
}

void BaseTest::clearAllMappings()
{
    vi_global->mappings()->clear(Mappings::NormalModeMapping);
    vi_global->mappings()->clear(Mappings::VisualModeMapping);
    vi_global->mappings()->clear(Mappings::InsertModeMapping);
    vi_global->mappings()->clear(Mappings::CommandModeMapping);
}

void BaseTest::clearAllMacros()
{
    vi_global->macros()->clear();
}

void BaseTest::textInserted(Document *document, KTextEditor::Range range)
{
    m_docChanges.append(DocChange(DocChange::TextInserted, range, document->text(range)));
}

void BaseTest::textRemoved(Document *document, KTextEditor::Range range)
{
    Q_UNUSED(document);
    m_docChanges.append(DocChange(DocChange::TextRemoved, range));
}

#include "moc_base.cpp"
// END: BaseTest

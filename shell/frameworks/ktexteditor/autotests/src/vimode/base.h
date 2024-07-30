/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BASE_TEST_H
#define BASE_TEST_H

#include <QObject>

#include <KTextEditor/Range>

namespace KTextEditor
{
class ViewPrivate;
class Document;
class DocumentPrivate;
}
class QMainWindow;
class QVBoxLayout;
class KateViInputMode;

class QLineEdit;
namespace KateVi
{
class EmulatedCommandBar;
class GlobalState;
class InputModeManager;
}

/* Syntactic sugar :P */
#define DoTest(...) DoTest_(__LINE__, __FILE__, __VA_ARGS__)
#define FinishTest(...) FinishTest_(__LINE__, __FILE__, __VA_ARGS__)

/// Helper class that represents a change in a document.
class DocChange
{
public:
    enum ChangeType { TextRemoved, TextInserted };

    DocChange(ChangeType changeType, KTextEditor::Range changeRange, QString newText = QString())
        : m_changeType(changeType)
        , m_changeRange(changeRange)
        , m_newText(newText)
    {
        /* There's nothing to do here. */
    }

    ChangeType changeType() const
    {
        return m_changeType;
    }

    KTextEditor::Range changeRange() const
    {
        return m_changeRange;
    }

    QString newText() const
    {
        return m_newText;
    }

private:
    ChangeType m_changeType;
    KTextEditor::Range m_changeRange;
    QString m_newText;
};

class BaseTest : public QObject
{
    Q_OBJECT

public:
    BaseTest();
    ~BaseTest() override;

    static void waitForCompletionWidgetToActivate(KTextEditor::ViewPrivate *kate_view);

protected:
    // Begin/Do/Finish test.

    enum Expectation { ShouldPass, ShouldFail };

    void TestPressKey(const QString &str);
    void BeginTest(const QString &original);
    void FinishTest_(int line, const char *file, const char *expected, Expectation expectation = ShouldPass, const char *failureReason = nullptr);
    void DoTest_(int line,
                 const char *file,
                 const char *original,
                 const char *command,
                 const char *expected,
                 Expectation expectation = ShouldPass,
                 const char *failureReason = nullptr);

    void DoTest2(int line,
                 const char *file,
                 const QString &original,
                 const QString &command,
                 const QString &expected,
                 Expectation expectation = ShouldPass,
                 const char *failureReason = nullptr);

    // Parsing keys.

    Qt::Key parseCodedSpecialKey(const QString &string, int startPos, int *destEndOfCodedKey);
    Qt::KeyboardModifier parseCodedModifier(const QString &string, int startPos, int *destEndOfCodedModifier);

    // Emulated command bar.

    KateVi::EmulatedCommandBar *emulatedCommandBar();
    QLineEdit *emulatedCommandBarTextEdit();

    // Macros & mappings.

    void clearAllMappings();
    void clearAllMacros();

    // Other auxiliar functions.

    void ensureKateViewVisible();

protected:
    KTextEditor::DocumentPrivate *kate_document;
    KTextEditor::ViewPrivate *kate_view;
    KateViInputMode *vi_input_mode;
    KateVi::GlobalState *vi_global;
    KateVi::InputModeManager *vi_input_mode_manager;

    bool m_firstBatchOfKeypressesForTest;

    QMainWindow *mainWindow;
    QVBoxLayout *mainWindowLayout;

    std::map<QString, Qt::Key> m_codesToSpecialKeys;
    std::map<QString, Qt::KeyboardModifier> m_codesToModifiers;

    QList<DocChange> m_docChanges;

protected Q_SLOTS:
    void init();

private Q_SLOTS:
    void textInserted(KTextEditor::Document *document, KTextEditor::Range range);
    void textRemoved(KTextEditor::Document *document, KTextEditor::Range range);
};

#endif /* BASE_TEST_H */

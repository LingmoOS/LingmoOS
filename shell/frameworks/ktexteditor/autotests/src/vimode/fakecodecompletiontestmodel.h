/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef FAKE_CODE_COMPLETION_TEST_MODEL_H
#define FAKE_CODE_COMPLETION_TEST_MODEL_H

#include <ktexteditor/codecompletionmodel.h>

namespace KTextEditor
{
class ViewPrivate;
}

/**
 * Helper class that mimics some of the behaviour of KDevelop's code completion, in particular
 * whether it performs "bracket merging" on completed function calls e.g. if we complete a call
 * to "functionCall(int a)" at the end of the -> here:
 *
 *  object->(
 *
 * we end up with
 *
 *  object->functionCall(
 *
 * and the cursor placed after the closing bracket: the opening bracket is merged with the existing
 * bracket.
 *
 * However, if we do the same with
 *
 *  object->
 *
 * we end up with
 *
 *  object->functionCall()
 *
 * again with the cursor placed after the opening bracket.  This time, the brackets were not merged.
 *
 * This helper class is used to test how Macros and replaying of last changes works with complex
 * code completion.
 */
class FakeCodeCompletionTestModel : public KTextEditor::CodeCompletionModel
{
    Q_OBJECT

public:
    explicit FakeCodeCompletionTestModel(KTextEditor::View *parent);
    /**
     * List of completions, in sorted order.
     * A string ending with "()" is treated as a call to a function with no arguments.
     * A string ending with "(...)" is treated as a call to a function with at least one argument.  The "..." is not
     * inserted into the text.
     * A string ending with "();" or "(...);" is the same as above, and the semi-colon is added.  Bracket merging
     * never happens with strings ending with ";".
     */
    void setCompletions(const QStringList &completions);
    void setRemoveTailOnComplete(bool removeTailOnCompletion);
    void setFailTestOnInvocation(bool failTestOnInvocation);
    bool wasInvoked();
    void clearWasInvoked();
    /**
     * A more reliable form of setAutomaticInvocationEnabled().
     */
    void forceInvocationIfDocTextIs(const QString &desiredDocText);
    void doNotForceInvocation();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override;

private:
    void failTest() const;
    QStringList m_completions;
    KTextEditor::ViewPrivate *m_kateView;
    KTextEditor::Document *m_kateDoc;
    bool m_removeTailOnCompletion;
    bool m_failTestOnInvocation;
    mutable bool m_wasInvoked;
    QString m_forceInvocationIfDocTextIs;

private Q_SLOTS:
    void textInserted(KTextEditor::Document *document, KTextEditor::Range range);
    void textRemoved(KTextEditor::Document *document, KTextEditor::Range range);
    void checkIfShouldForceInvocation();
};

#endif /* FAKE_CODE_COMPLETION_TEST_MODEL_H */

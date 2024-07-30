/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Miquel Sabaté Solà <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMPLETION_TEST_H
#define COMPLETION_TEST_H

#include "base.h"
#include "fakecodecompletiontestmodel.h"

/**
 * This class handles implements a completion model for the completion
 * tests defined in the CompletionTest class.
 */
class VimCodeCompletionTestModel : public KTextEditor::CodeCompletionModel
{
public:
    explicit VimCodeCompletionTestModel(KTextEditor::View *parent);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

/**
 * This class implements a completion model used in the CompletionTest class
 * to test that code completion is not invoked.
 */
class FailTestOnInvocationModel : public KTextEditor::CodeCompletionModel
{
public:
    explicit FailTestOnInvocationModel(KTextEditor::View *parent);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void failTest() const;
};

class CompletionTest : public BaseTest
{
    Q_OBJECT

private Q_SLOTS:
    void FakeCodeCompletionTests();
    void CompletionTests();

private:
    void waitForCompletionWidgetToActivate();
    void clearTrackedDocumentChanges();
};

#endif /* COMPLETION_TEST_H */

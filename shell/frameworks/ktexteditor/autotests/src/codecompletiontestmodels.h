/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams\gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_COMPLETIONTESTMODELS_H
#define KATE_COMPLETIONTESTMODELS_H

#include "codecompletiontestmodel.h"
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <QRegularExpression>

using namespace KTextEditor;

class CustomRangeModel : public CodeCompletionTestModel, public CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    explicit CustomRangeModel(KTextEditor::View *parent = nullptr, const QString &startText = QString())
        : CodeCompletionTestModel(parent, startText)
    {
    }
    Range completionRange(View *view, const Cursor &position) override
    {
        Range range = CodeCompletionModelControllerInterface::completionRange(view, position);
        if (range.start().column() > 0) {
            KTextEditor::Range preRange(Cursor(range.start().line(), range.start().column() - 1), Cursor(range.start().line(), range.start().column()));
            qDebug() << preRange << view->document()->text(preRange);
            if (view->document()->text(preRange) == QLatin1String("$")) {
                range.expandToRange(preRange);
                qDebug() << "using custom completion range" << range;
            }
        }
        return range;
    }

    bool shouldAbortCompletion(View *view, const Range &range, const QString &currentCompletion) override
    {
        Q_UNUSED(view);
        Q_UNUSED(range);
        static const QRegularExpression allowedText(QStringLiteral("^\\$?(?:\\w*)$"), QRegularExpression::UseUnicodePropertiesOption);
        return !allowedText.match(currentCompletion).hasMatch();
    }
};

class CustomAbortModel : public CodeCompletionTestModel, public CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    explicit CustomAbortModel(KTextEditor::View *parent = nullptr, const QString &startText = QString())
        : CodeCompletionTestModel(parent, startText)
    {
    }

    bool shouldAbortCompletion(View *view, const Range &range, const QString &currentCompletion) override
    {
        Q_UNUSED(view);
        Q_UNUSED(range);
        static const QRegularExpression allowedText(QStringLiteral("^(?:[\\w-]*)"), QRegularExpression::UseUnicodePropertiesOption);
        return !allowedText.match(currentCompletion).hasMatch();
    }
};

class EmptyFilterStringModel : public CodeCompletionTestModel, public CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    explicit EmptyFilterStringModel(KTextEditor::View *parent = nullptr, const QString &startText = QString())
        : CodeCompletionTestModel(parent, startText)
    {
    }

    QString filterString(View *, const Range &, const Cursor &) override
    {
        return QString();
    }
};

class UpdateCompletionRangeModel : public CodeCompletionTestModel, public CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    explicit UpdateCompletionRangeModel(KTextEditor::View *parent = nullptr, const QString &startText = QString())
        : CodeCompletionTestModel(parent, startText)
    {
    }

    Range updateCompletionRange(View *view, const Range &range) override
    {
        Q_UNUSED(view);
        if (view->document()->text(range) == QStringLiteral("ab")) {
            return Range(Cursor(range.start().line(), 0), range.end());
        }
        return range;
    }
    bool shouldAbortCompletion(View *view, const Range &range, const QString &currentCompletion) override
    {
        Q_UNUSED(view);
        Q_UNUSED(range);
        Q_UNUSED(currentCompletion);
        return false;
    }
};

class StartCompletionModel : public CodeCompletionTestModel, public CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    explicit StartCompletionModel(KTextEditor::View *parent = nullptr, const QString &startText = QString())
        : CodeCompletionTestModel(parent, startText)
    {
    }

    bool shouldStartCompletion(View *view, const QString &insertedText, bool userInsertion, const Cursor &position) override
    {
        Q_UNUSED(view);
        Q_UNUSED(userInsertion);
        Q_UNUSED(position);
        if (insertedText.isEmpty()) {
            return false;
        }

        QChar lastChar = insertedText.at(insertedText.size() - 1);
        if (lastChar == u'%') {
            return true;
        }
        return false;
    }
};

class ImmideatelyAbortCompletionModel : public CodeCompletionTestModel, public CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    explicit ImmideatelyAbortCompletionModel(KTextEditor::View *parent = nullptr, const QString &startText = QString())
        : CodeCompletionTestModel(parent, startText)
    {
    }

    bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion) override
    {
        Q_UNUSED(view);
        Q_UNUSED(range);
        Q_UNUSED(currentCompletion);
        return true;
    }
};

#endif

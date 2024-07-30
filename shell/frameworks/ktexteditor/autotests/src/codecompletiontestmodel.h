/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2005 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CODECOMPLETIONTEST_MODEL_H
#define CODECOMPLETIONTEST_MODEL_H

#include <QStringList>
#include <ktexteditor/codecompletionmodel.h>

namespace KTextEditor
{
class View;
}

class CodeCompletionTestModel : public KTextEditor::CodeCompletionModel
{
    Q_OBJECT

public:
    explicit CodeCompletionTestModel(KTextEditor::View *parent = nullptr, const QString &startText = QString());

    KTextEditor::View *view() const;

    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QString m_startText;
    bool m_autoStartText;
};

class AbbreviationCodeCompletionTestModel : public CodeCompletionTestModel
{
    Q_OBJECT

public:
    explicit AbbreviationCodeCompletionTestModel(KTextEditor::View *parent = nullptr, const QString &startText = QString());

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QStringList m_items;
};

class AsyncCodeCompletionTestModel : public CodeCompletionTestModel
{
    Q_OBJECT

public:
    explicit AsyncCodeCompletionTestModel(KTextEditor::View *parent = nullptr, const QString &startText = QString());

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType) override;
    void setItems(const QStringList &items);

private:
    QStringList m_items;
};

#endif

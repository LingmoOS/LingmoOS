/*
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEKEYWORDCOMPLETIONMODEL_H
#define KATEKEYWORDCOMPLETIONMODEL_H

#include "codecompletionmodelcontrollerinterface.h"
#include "ktexteditor/codecompletionmodel.h"

/**
 * @brief Highlighting-file based keyword completion for the editor.
 *
 * This model offers completion of language-specific keywords based on information
 * taken from the kate syntax files. It queries the highlighting engine to get the
 * correct context for a given cursor position, then suggests all keyword items
 * from the XML file for the active language.
 */
class KateKeywordCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)

public:
    explicit KateKeywordCompletionModel(QObject *parent);
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType) override;
    KTextEditor::Range completionRange(KTextEditor::View *view, const KTextEditor::Cursor &position) override;
    bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion) override;
    bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position) override;
    MatchReaction matchingItem(const QModelIndex &matched) override;
    bool shouldHideItemsWithEqualNames() const override;

private:
    QList<QString> m_items;
};

#endif // KATEKEYWORDCOMPLETIONMODEL_H

// kate: indent-width 4; replace-tabs on

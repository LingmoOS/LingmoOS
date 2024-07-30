/*
    SPDX-FileCopyrightText: 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KateWordCompletion_h_
#define _KateWordCompletion_h_

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/codecompletionmodelcontrollerinterface.h>
#include <ktexteditor/view.h>

#include <QEvent>
#include <QList>
#include <QObject>

#include "katepartdebug.h"
#include <ktexteditor_export.h>

class KateWordCompletionModel : public KTextEditor::CodeCompletionModel, public KTextEditor::CodeCompletionModelControllerInterface
{
    Q_OBJECT
    Q_INTERFACES(KTextEditor::CodeCompletionModelControllerInterface)
public:
    KTEXTEDITOR_EXPORT explicit KateWordCompletionModel(QObject *parent);
    KTEXTEDITOR_EXPORT ~KateWordCompletionModel() override;

    /**
     * This function is responsible to generating / updating the list of current
     * completions. The default implementation does nothing.
     *
     * When implementing this function, remember to call setRowCount() (or implement
     * rowCount()), and to generate the appropriate change notifications (for instance
     * by calling QAbstractItemModel::reset()).
     * @param view The view to generate completions for
     * @param range The range of text to generate completions for
     * */
    void completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType invocationType) override;

    bool shouldStartCompletion(KTextEditor::View *view, const QString &insertedText, bool userInsertion, const KTextEditor::Cursor &position) override;
    bool shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion) override;

    void saveMatches(KTextEditor::View *view, const KTextEditor::Range &range);

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    MatchReaction matchingItem(const QModelIndex &matched) override;

    bool shouldHideItemsWithEqualNames() const override;

    KTEXTEDITOR_EXPORT QStringList allMatches(KTextEditor::View *view, const KTextEditor::Range &range);

    void executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const override;

private:
    QStringList m_matches;
    bool m_automatic;
};

class KateWordCompletionView : public QObject
{
public:
    KateWordCompletionView(KTextEditor::View *view, KActionCollection *ac);
    ~KateWordCompletionView() override;

private:
    void completeBackwards();
    void completeForwards();
    void slotCursorMoved();

    void shellComplete();

    void popupCompletionList();

private:
    void complete(bool fw = true);

    QString word() const;
    KTextEditor::Range range() const;

    static QString findLongestUnique(const QStringList &matches, int lead);

    KTextEditor::View *m_view;
    KateWordCompletionModel *m_dWCompletionModel;
    struct KateWordCompletionViewPrivate *d;
};

#endif // _DocWordCompletionPlugin_h_

/*
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2022-2024 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATECOMPLETIONWIDGET_H
#define KATECOMPLETIONWIDGET_H

#include <QFrame>
#include <QObject>
#include <QPointer>

#include <ktexteditor_export.h>

#include <ktexteditor/codecompletionmodel.h>
#include <ktexteditor/movingrange.h>

class QToolButton;
class QPushButton;
class QLabel;
class QTimer;

namespace KTextEditor
{
class ViewPrivate;
}
class DocTip;
class KateCompletionModel;
class KateCompletionTree;
class KateArgumentHintTree;
class KateArgumentHintModel;
class ArgumentHintWidget;

/**
 * This is the code completion's main widget, and also contains the
 * core interface logic.
 *
 * @author Hamish Rodda <rodda@kde.org>
 */
class KTEXTEDITOR_EXPORT KateCompletionWidget : public QFrame
{
    Q_OBJECT

public:
    explicit KateCompletionWidget(KTextEditor::ViewPrivate *parent);
    ~KateCompletionWidget() override;

    KTextEditor::ViewPrivate *view() const;
    KateCompletionTree *treeView() const;

    bool isCompletionActive() const;
    void startCompletion(KTextEditor::CodeCompletionModel::InvocationType invocationType,
                         const QList<KTextEditor::CodeCompletionModel *> &models = QList<KTextEditor::CodeCompletionModel *>());
    void startCompletion(KTextEditor::Range word,
                         KTextEditor::CodeCompletionModel *model,
                         KTextEditor::CodeCompletionModel::InvocationType invocationType = KTextEditor::CodeCompletionModel::ManualInvocation);
    void startCompletion(KTextEditor::Range word,
                         const QList<KTextEditor::CodeCompletionModel *> &models = QList<KTextEditor::CodeCompletionModel *>(),
                         KTextEditor::CodeCompletionModel::InvocationType invocationType = KTextEditor::CodeCompletionModel::ManualInvocation);
    void userInvokedCompletion();

public Q_SLOTS:
    // Executed when return is pressed while completion is active.
    bool execute();
    void cursorDown();
    void cursorUp();

public:
    enum Direction {
        Down,
        Up,
    };

    void tabCompletion(Direction direction = Down);

    void toggleDocumentation();

    const KateCompletionModel *model() const;
    KateCompletionModel *model();

    void registerCompletionModel(KTextEditor::CodeCompletionModel *model);
    void unregisterCompletionModel(KTextEditor::CodeCompletionModel *model);
    bool isCompletionModelRegistered(KTextEditor::CodeCompletionModel *model) const;
    QList<KTextEditor::CodeCompletionModel *> codeCompletionModels() const;

    int automaticInvocationDelay() const;
    void setAutomaticInvocationDelay(int delay);

    void setIgnoreBufferSignals(bool ignore) const;

    bool m_ignoreBufferSignals = false;

    struct CompletionRange {
        CompletionRange()
        {
        }
        explicit CompletionRange(KTextEditor::MovingRange *r)
            : range(r)
        {
        }

        bool operator==(const CompletionRange &rhs) const
        {
            return range->toRange() == rhs.range->toRange();
        }

        KTextEditor::MovingRange *range = nullptr;
        // Whenever the cursor goes before this position, the completion is stopped, unless it is invalid.
        KTextEditor::Cursor leftBoundary;
    };

    KTextEditor::MovingRange *completionRange(KTextEditor::CodeCompletionModel *model = nullptr) const;
    QMap<KTextEditor::CodeCompletionModel *, CompletionRange> completionRanges() const;

    // Navigation
    void pageDown();
    void pageUp();
    void top();
    void bottom();

    QWidget *currentEmbeddedWidget();

    void updatePosition(bool force = false);
    bool eventFilter(QObject *watched, QEvent *event) override;

    KateArgumentHintModel *argumentHintModel() const;

    /// Called by KateViewInternal, because we need the specific information from the event.

    void updateHeight();

    void showDocTip(const QModelIndex &idx);
    DocTip *docTip() const
    {
        return m_docTip;
    }

    bool handleShortcutOverride(QKeyEvent *e);

public Q_SLOTS:
    void waitForModelReset();

    void abortCompletion();
    void automaticInvocation();

    /*    void updateFocus();*/
    void argumentHintsChanged(bool hasContent);

    bool navigateUp();
    bool navigateDown();
    bool navigateLeft();
    bool navigateRight();
    bool navigateAccept();
    bool navigateBack();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private Q_SLOTS:
    void completionModelReset();
    void modelDestroyed(QObject *model);
    void modelContentChanged();
    void cursorPositionChanged();
    void modelReset();
    void rowsInserted(const QModelIndex &parent, int row, int rowEnd);
    void viewFocusOut();

    void wrapLine(KTextEditor::Document *document, KTextEditor::Cursor position);
    void unwrapLine(KTextEditor::Document *, int line);
    void insertText(KTextEditor::Document *, KTextEditor::Cursor position, const QString &text);
    void removeText(KTextEditor::Document *, KTextEditor::Range range, const QString &);

private:
    KTEXTEDITOR_NO_EXPORT
    void updateAndShow();
    KTEXTEDITOR_NO_EXPORT
    void updateArgumentHintGeometry();
    KTEXTEDITOR_NO_EXPORT
    QModelIndex selectedIndex() const;

    KTEXTEDITOR_NO_EXPORT
    void clear();

    KTEXTEDITOR_NO_EXPORT
    void completionRangeChanged(KTextEditor::CodeCompletionModel *, const KTextEditor::Range &word);

    KTEXTEDITOR_NO_EXPORT
    QString tailString() const;

    KTEXTEDITOR_NO_EXPORT
    void deleteCompletionRanges();

private:
    QList<KTextEditor::CodeCompletionModel *> m_sourceModels;
    KateCompletionModel *m_presentationModel;

    QMap<KTextEditor::CodeCompletionModel *, CompletionRange> m_completionRanges;
    QSet<KTextEditor::CodeCompletionModel *> m_waitingForReset;

    KTextEditor::Cursor m_lastCursorPosition;

    KTextEditor::ViewPrivate *m_view;
    KateCompletionTree *m_entryList;
    KateArgumentHintModel *m_argumentHintModel;
    // KateArgumentHintTree *m_argumentHintTree;
    ArgumentHintWidget *m_argumentHintWidget;
    DocTip *m_docTip;

    QTimer *m_automaticInvocationTimer;

    KTextEditor::Cursor m_automaticInvocationAt;
    QString m_automaticInvocationLine;
    int m_automaticInvocationDelay;

    bool m_lastInsertionByUser;
    bool m_isSuspended;
    bool m_dontShowArgumentHints; // Used temporarily to prevent flashing
    bool m_needShow;

    bool m_hadCompletionNavigation;

    bool m_haveExactMatch;

    bool m_noAutoHide;

    /**
     * is a completion edit ongoing?
     */
    bool m_completionEditRunning;

    int m_expandedAddedHeightBase;

    KTextEditor::CodeCompletionModel::InvocationType m_lastInvocationType;
};

#endif

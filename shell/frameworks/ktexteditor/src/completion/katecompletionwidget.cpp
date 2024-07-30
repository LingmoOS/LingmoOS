/*
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2022-2024 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecompletionwidget.h"

#include <ktexteditor/codecompletionmodelcontrollerinterface.h>

#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katerenderer.h"
#include "kateview.h"

#include "documentation_tip.h"
#include "kateargumenthintmodel.h"
#include "kateargumenthinttree.h"
#include "katecompletionmodel.h"
#include "katecompletiontree.h"
#include "katepartdebug.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QScrollBar>
#include <QSizeGrip>
#include <QTimer>
#include <QToolButton>

const bool hideAutomaticCompletionOnExactMatch = true;

#define CALLCI(WHAT, WHATELSE, WHAT2, model, FUNC)                                                                                                             \
    {                                                                                                                                                          \
        static KTextEditor::CodeCompletionModelControllerInterface defaultIf;                                                                                  \
        KTextEditor::CodeCompletionModelControllerInterface *ret = qobject_cast<KTextEditor::CodeCompletionModelControllerInterface *>(model);                 \
        if (!ret) {                                                                                                                                            \
            WHAT2 defaultIf.FUNC;                                                                                                                              \
        } else                                                                                                                                                 \
            WHAT2 ret->FUNC;                                                                                                                                   \
    }

static KTextEditor::Range _completionRange(KTextEditor::CodeCompletionModel *model, KTextEditor::View *view, KTextEditor::Cursor cursor)
{
    CALLCI(return, , return, model, completionRange(view, cursor));
}

static KTextEditor::Range _updateRange(KTextEditor::CodeCompletionModel *model, KTextEditor::View *view, KTextEditor::Range &range)
{
    CALLCI(, return range, return, model, updateCompletionRange(view, range));
}

static QString _filterString(KTextEditor::CodeCompletionModel *model, KTextEditor::View *view, const KTextEditor::Range &range, KTextEditor::Cursor cursor)
{
    CALLCI(return, , return, model, filterString(view, range, cursor));
}

static bool
_shouldAbortCompletion(KTextEditor::CodeCompletionModel *model, KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion)
{
    CALLCI(return, , return, model, shouldAbortCompletion(view, range, currentCompletion));
}

static void _aborted(KTextEditor::CodeCompletionModel *model, KTextEditor::View *view)
{
    CALLCI(return, , return, model, aborted(view));
}

static bool _shouldStartCompletion(KTextEditor::CodeCompletionModel *model,
                                   KTextEditor::View *view,
                                   const QString &automaticInvocationLine,
                                   bool m_lastInsertionByUser,
                                   KTextEditor::Cursor cursor)
{
    CALLCI(return, , return, model, shouldStartCompletion(view, automaticInvocationLine, m_lastInsertionByUser, cursor));
}

KateCompletionWidget::KateCompletionWidget(KTextEditor::ViewPrivate *parent)
    : QFrame(parent)
    , m_presentationModel(new KateCompletionModel(this))
    , m_view(parent)
    , m_entryList(new KateCompletionTree(this))
    , m_argumentHintModel(new KateArgumentHintModel(m_presentationModel))
    , m_argumentHintWidget(new ArgumentHintWidget(m_argumentHintModel, parent->renderer()->currentFont(), this, this))
    , m_docTip(new DocTip(this))
    , m_automaticInvocationDelay(100)
    , m_lastInsertionByUser(false)
    , m_isSuspended(false)
    , m_dontShowArgumentHints(false)
    , m_needShow(false)
    , m_hadCompletionNavigation(false)
    , m_noAutoHide(false)
    , m_completionEditRunning(false)
    , m_expandedAddedHeightBase(0)
    , m_lastInvocationType(KTextEditor::CodeCompletionModel::AutomaticInvocation)
{
    if (parent->mainWindow() != KTextEditor::EditorPrivate::self()->dummyMainWindow() && parent->mainWindow()->window()) {
        setParent(parent->mainWindow()->window());
    } else if (auto w = m_view->window()) {
        setParent(w);
    } else if (auto w = QApplication::activeWindow()) {
        setParent(w);
    } else {
        setParent(parent);
    }
    m_docTip->setParent(this->parentWidget());

    parentWidget()->installEventFilter(this);

    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(1);

    m_entryList->setModel(m_presentationModel);
    m_entryList->setColumnWidth(0, 0); // These will be determined automatically in KateCompletionTree::resizeColumns
    m_entryList->setColumnWidth(1, 0);
    m_entryList->setColumnWidth(2, 0);

    m_argumentHintWidget->setParent(this->parentWidget());

    // trigger completion on double click on completion list
    connect(m_entryList, &KateCompletionTree::doubleClicked, this, &KateCompletionWidget::execute);

    connect(view(), &KTextEditor::ViewPrivate::focusOut, this, &KateCompletionWidget::viewFocusOut);

    m_automaticInvocationTimer = new QTimer(this);
    m_automaticInvocationTimer->setSingleShot(true);
    connect(m_automaticInvocationTimer, &QTimer::timeout, this, &KateCompletionWidget::automaticInvocation);

    // Keep branches expanded
    connect(m_presentationModel, &KateCompletionModel::modelReset, this, &KateCompletionWidget::modelReset);
    connect(m_presentationModel, &KateCompletionModel::rowsInserted, this, &KateCompletionWidget::rowsInserted);
    connect(m_argumentHintModel, &KateArgumentHintModel::contentStateChanged, this, &KateCompletionWidget::argumentHintsChanged);

    // No smart lock, no queued connects
    connect(view(), &KTextEditor::ViewPrivate::cursorPositionChanged, this, &KateCompletionWidget::cursorPositionChanged);
    connect(view(), &KTextEditor::ViewPrivate::verticalScrollPositionChanged, this, [this] {
        abortCompletion();
    });

    // connect to all possible editing primitives
    connect(view()->doc(), &KTextEditor::Document::lineWrapped, this, &KateCompletionWidget::wrapLine);
    connect(view()->doc(), &KTextEditor::Document::lineUnwrapped, this, &KateCompletionWidget::unwrapLine);
    connect(view()->doc(), &KTextEditor::Document::textInserted, this, &KateCompletionWidget::insertText);
    connect(view()->doc(), &KTextEditor::Document::textRemoved, this, &KateCompletionWidget::removeText);

    // This is a non-focus widget, it is passed keyboard input from the view

    // We need to do this, because else the focus goes to nirvana without any control when the completion-widget is clicked.
    setFocusPolicy(Qt::ClickFocus);

    const auto children = findChildren<QWidget *>();
    for (QWidget *childWidget : children) {
        childWidget->setFocusPolicy(Qt::NoFocus);
    }

    // Position the entry-list so a frame can be drawn around it
    m_entryList->move(frameWidth(), frameWidth());

    hide();
    m_docTip->setVisible(false);
}

KateCompletionWidget::~KateCompletionWidget()
{
    // ensure no slot triggered during destruction => else we access already invalidated stuff
    m_presentationModel->disconnect(this);
    m_argumentHintModel->disconnect(this);

    delete m_docTip;
}

void KateCompletionWidget::viewFocusOut()
{
    QWidget *toplevels[4] = {this, m_entryList, m_docTip, m_argumentHintWidget};
    if (!std::any_of(std::begin(toplevels), std::end(toplevels), [](QWidget *w) {
            auto fw = QApplication::focusWidget();
            return fw == w || w->isAncestorOf(fw);
        })) {
        abortCompletion();
    }
}

void KateCompletionWidget::focusOutEvent(QFocusEvent *)
{
    abortCompletion();
}

void KateCompletionWidget::modelContentChanged()
{
    ////qCDebug(LOG_KTE)<<">>>>>>>>>>>>>>>>";
    if (m_completionRanges.isEmpty()) {
        // qCDebug(LOG_KTE) << "content changed, but no completion active";
        abortCompletion();
        return;
    }

    if (!view()->hasFocus()) {
        // qCDebug(LOG_KTE) << "view does not have focus";
        return;
    }

    if (!m_waitingForReset.isEmpty()) {
        // qCDebug(LOG_KTE) << "waiting for" << m_waitingForReset.size() << "completion-models to reset";
        return;
    }

    int realItemCount = 0;
    const auto completionModels = m_presentationModel->completionModels();
    for (KTextEditor::CodeCompletionModel *model : completionModels) {
        realItemCount += model->rowCount();
    }
    if (!m_isSuspended && ((isHidden() && m_argumentHintWidget->isHidden()) || m_needShow) && realItemCount != 0) {
        m_needShow = false;
        updateAndShow();
    }

    if (m_argumentHintModel->rowCount(QModelIndex()) == 0) {
        m_argumentHintWidget->hide();
    }

    if (m_presentationModel->rowCount(QModelIndex()) == 0) {
        hide();
    }

    // For automatic invocations, only autoselect first completion entry when enabled in the config
    if (m_lastInvocationType != KTextEditor::CodeCompletionModel::AutomaticInvocation || view()->config()->automaticCompletionPreselectFirst()) {
        m_entryList->setCurrentIndex(model()->index(0, 0));
    }
    // With each filtering items can be added or removed, so we have to reset the current index here so we always have a selected item
    if (!model()->indexIsItem(m_entryList->currentIndex())) {
        QModelIndex firstIndex = model()->index(0, 0, m_entryList->currentIndex());
        m_entryList->setCurrentIndex(firstIndex);
        // m_entryList->scrollTo(firstIndex, QAbstractItemView::PositionAtTop);
    }

    updateHeight();

    // New items for the argument-hint tree may have arrived, so check whether it needs to be shown
    if (m_argumentHintWidget->isHidden() && !m_dontShowArgumentHints && m_argumentHintModel->rowCount(QModelIndex()) != 0) {
        m_argumentHintWidget->positionAndShow();
    }

    if (!m_noAutoHide && hideAutomaticCompletionOnExactMatch && !isHidden() && m_lastInvocationType == KTextEditor::CodeCompletionModel::AutomaticInvocation
        && m_presentationModel->shouldMatchHideCompletionList()) {
        hide();
    } else if (isHidden() && !m_presentationModel->shouldMatchHideCompletionList() && m_presentationModel->rowCount(QModelIndex())) {
        show();
    }
}

KateArgumentHintModel *KateCompletionWidget::argumentHintModel() const
{
    return m_argumentHintModel;
}

const KateCompletionModel *KateCompletionWidget::model() const
{
    return m_presentationModel;
}

KateCompletionModel *KateCompletionWidget::model()
{
    return m_presentationModel;
}

void KateCompletionWidget::rowsInserted(const QModelIndex &parent, int rowFrom, int rowEnd)
{
    m_entryList->setAnimated(false);

    if (!parent.isValid()) {
        for (int i = rowFrom; i <= rowEnd; ++i) {
            m_entryList->expand(m_presentationModel->index(i, 0, parent));
        }
    }
}

KTextEditor::ViewPrivate *KateCompletionWidget::view() const
{
    return m_view;
}

void KateCompletionWidget::argumentHintsChanged(bool hasContent)
{
    m_dontShowArgumentHints = !hasContent;

    if (m_dontShowArgumentHints) {
        m_argumentHintWidget->hide();
    } else {
        updateArgumentHintGeometry();
    }
}

void KateCompletionWidget::startCompletion(KTextEditor::CodeCompletionModel::InvocationType invocationType,
                                           const QList<KTextEditor::CodeCompletionModel *> &models)
{
    if (invocationType == KTextEditor::CodeCompletionModel::UserInvocation) {
        abortCompletion();
    }
    startCompletion(KTextEditor::Range(KTextEditor::Cursor(-1, -1), KTextEditor::Cursor(-1, -1)), models, invocationType);
}

void KateCompletionWidget::deleteCompletionRanges()
{
    for (const CompletionRange &r : std::as_const(m_completionRanges)) {
        delete r.range;
    }
    m_completionRanges.clear();
}

void KateCompletionWidget::startCompletion(KTextEditor::Range word,
                                           KTextEditor::CodeCompletionModel *model,
                                           KTextEditor::CodeCompletionModel::InvocationType invocationType)
{
    QList<KTextEditor::CodeCompletionModel *> models;
    if (model) {
        models << model;
    } else {
        models = m_sourceModels;
    }
    startCompletion(word, models, invocationType);
}

void KateCompletionWidget::startCompletion(KTextEditor::Range word,
                                           const QList<KTextEditor::CodeCompletionModel *> &modelsToStart,
                                           KTextEditor::CodeCompletionModel::InvocationType invocationType)
{
    ////qCDebug(LOG_KTE)<<"============";

    m_isSuspended = false;
    m_needShow = true;

    if (m_completionRanges.isEmpty()) {
        m_noAutoHide = false; // Re-enable auto-hide on every clean restart of the completion
    }

    m_lastInvocationType = invocationType;

    disconnect(this->model(), &KateCompletionModel::layoutChanged, this, &KateCompletionWidget::modelContentChanged);
    disconnect(this->model(), &KateCompletionModel::modelReset, this, &KateCompletionWidget::modelContentChanged);

    m_dontShowArgumentHints = true;

    QList<KTextEditor::CodeCompletionModel *> models = (modelsToStart.isEmpty() ? m_sourceModels : modelsToStart);

    for (auto it = m_completionRanges.keyBegin(), end = m_completionRanges.keyEnd(); it != end; ++it) {
        KTextEditor::CodeCompletionModel *model = *it;
        if (!models.contains(model)) {
            models << model;
        }
    }

    m_presentationModel->clearCompletionModels();

    if (invocationType == KTextEditor::CodeCompletionModel::UserInvocation) {
        deleteCompletionRanges();
    }

    for (KTextEditor::CodeCompletionModel *model : std::as_const(models)) {
        KTextEditor::Range range;
        if (word.isValid()) {
            range = word;
            // qCDebug(LOG_KTE)<<"word is used";
        } else {
            range = _completionRange(model, view(), view()->cursorPosition());
            // qCDebug(LOG_KTE)<<"completionRange has been called, cursor pos is"<<view()->cursorPosition();
        }
        // qCDebug(LOG_KTE)<<"range is"<<range;
        if (!range.isValid()) {
            if (m_completionRanges.contains(model)) {
                KTextEditor::MovingRange *oldRange = m_completionRanges[model].range;
                // qCDebug(LOG_KTE)<<"removing completion range 1";
                m_completionRanges.remove(model);
                delete oldRange;
            }
            models.removeAll(model);
            continue;
        }
        if (m_completionRanges.contains(model)) {
            if (*m_completionRanges[model].range == range) {
                continue; // Leave it running as it is
            } else { // delete the range that was used previously
                KTextEditor::MovingRange *oldRange = m_completionRanges[model].range;
                // qCDebug(LOG_KTE)<<"removing completion range 2";
                m_completionRanges.remove(model);
                delete oldRange;
            }
        }

        connect(model, &KTextEditor::CodeCompletionModel::waitForReset, this, &KateCompletionWidget::waitForModelReset);

        // qCDebug(LOG_KTE)<<"Before completion invoke: range:"<<range;
        model->completionInvoked(view(), range, invocationType);

        disconnect(model, &KTextEditor::CodeCompletionModel::waitForReset, this, &KateCompletionWidget::waitForModelReset);

        m_completionRanges[model] =
            CompletionRange(view()->doc()->newMovingRange(range, KTextEditor::MovingRange::ExpandRight | KTextEditor::MovingRange::ExpandLeft));

        // In automatic invocation mode, hide the completion widget as soon as the position where the completion was started is passed to the left
        m_completionRanges[model].leftBoundary = view()->cursorPosition();

        // In manual invocation mode, bound the activity either the point from where completion was invoked, or to the start of the range
        if (invocationType != KTextEditor::CodeCompletionModel::AutomaticInvocation) {
            if (range.start() < m_completionRanges[model].leftBoundary) {
                m_completionRanges[model].leftBoundary = range.start();
            }
        }

        if (!m_completionRanges[model].range->toRange().isValid()) {
            qCWarning(LOG_KTE) << "Could not construct valid smart-range from" << range << "instead got" << *m_completionRanges[model].range;
            abortCompletion();
            return;
        }
    }

    m_presentationModel->setCompletionModels(models);

    cursorPositionChanged();

    if (!m_completionRanges.isEmpty()) {
        connect(this->model(), &KateCompletionModel::layoutChanged, this, &KateCompletionWidget::modelContentChanged);
        connect(this->model(), &KateCompletionModel::modelReset, this, &KateCompletionWidget::modelContentChanged);
        // Now that all models have been notified, check whether the widget should be displayed instantly
        modelContentChanged();
    } else {
        abortCompletion();
    }
}

QString KateCompletionWidget::tailString() const
{
    if (!KateViewConfig::global()->wordCompletionRemoveTail()) {
        return QString();
    }

    const int line = view()->cursorPosition().line();
    const int column = view()->cursorPosition().column();

    const QString text = view()->document()->line(line);

    static constexpr auto options = QRegularExpression::UseUnicodePropertiesOption | QRegularExpression::DontCaptureOption;
    static const QRegularExpression findWordEnd(QStringLiteral("^[_\\w]*\\b"), options);

    QRegularExpressionMatch match = findWordEnd.match(text.mid(column));
    if (match.hasMatch()) {
        return match.captured(0);
    }
    return QString();
}

void KateCompletionWidget::waitForModelReset()
{
    KTextEditor::CodeCompletionModel *senderModel = qobject_cast<KTextEditor::CodeCompletionModel *>(sender());
    if (!senderModel) {
        qCWarning(LOG_KTE) << "waitForReset signal from bad model";
        return;
    }
    m_waitingForReset.insert(senderModel);
}

void KateCompletionWidget::updateAndShow()
{
    // qCDebug(LOG_KTE)<<"*******************************************";
    if (!view()->hasFocus()) {
        qCDebug(LOG_KTE) << "view does not have focus";
        return;
    }

    setUpdatesEnabled(false);

    modelReset();

    m_argumentHintModel->buildRows();
    if (m_argumentHintModel->rowCount(QModelIndex()) != 0) {
        argumentHintsChanged(true);
    }

    // update height first
    updateHeight();
    // then resize columns afterwards because we need height information
    m_entryList->resizeColumns(true, true);
    // lastly update position as now we have height and width
    updatePosition(true);

    setUpdatesEnabled(true);

    if (m_argumentHintModel->rowCount(QModelIndex())) {
        updateArgumentHintGeometry();
        m_argumentHintWidget->positionAndShow();
    } else {
        m_argumentHintWidget->hide();
    }

    if (m_presentationModel->rowCount()
        && (!m_presentationModel->shouldMatchHideCompletionList() || !hideAutomaticCompletionOnExactMatch
            || m_lastInvocationType != KTextEditor::CodeCompletionModel::AutomaticInvocation)) {
        show();
    } else {
        hide();
    }
}

void KateCompletionWidget::updatePosition(bool force)
{
    if (!force && !isCompletionActive()) {
        return;
    }

    if (!completionRange()) {
        return;
    }
    const QPoint localCursorCoord = view()->cursorToCoordinate(completionRange()->start());
    if (localCursorCoord == QPoint(-1, -1)) {
        // Start of completion range is now off-screen -> abort
        abortCompletion();
        return;
    }

    const QPoint cursorCoordinate = view()->mapToGlobal(localCursorCoord);
    QPoint p = cursorCoordinate;
    int x = p.x();
    int y = p.y();

    y += view()->renderer()->currentFontMetrics().height() + 2;

    const auto windowGeometry = parentWidget()->geometry();
    if (x + width() > windowGeometry.right()) {
        // crossing right edge
        x = windowGeometry.right() - width();
    }
    if (x < windowGeometry.left()) {
        x = windowGeometry.left();
    }

    if (y + height() > windowGeometry.bottom()) {
        // move above cursor if we are crossing the bottom
        y -= height();
        if (y + height() > cursorCoordinate.y()) {
            y -= (y + height()) - cursorCoordinate.y();
            y -= 2;
        }
    }

    move(parentWidget()->mapFromGlobal(QPoint(x, y)));
}

void KateCompletionWidget::updateArgumentHintGeometry()
{
    if (!m_dontShowArgumentHints) {
        // Now place the argument-hint widget
        m_argumentHintWidget->updateGeometry();
    }
}

// Checks whether the given model has at least "rows" rows, also searching the second level of the tree.
static bool hasAtLeastNRows(int rows, QAbstractItemModel *model)
{
    int count = 0;
    const auto rowCount = model->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        ++count;

        QModelIndex index(model->index(row, 0));
        if (index.isValid()) {
            count += model->rowCount(index);
        }

        if (count > rows) {
            return true;
        }
    }
    return false;
}

void KateCompletionWidget::updateHeight()
{
    QRect geom = geometry();

    constexpr int minBaseHeight = 10;
    constexpr int maxBaseHeight = 300;

    int baseHeight = 0;
    int calculatedCustomHeight = 0;

    if (hasAtLeastNRows(15, m_presentationModel)) {
        // If we know there is enough rows, always use max-height, we don't need to calculate size-hints
        baseHeight = maxBaseHeight;
    } else {
        // Calculate size-hints to determine the best height
        for (int row = 0; row < m_presentationModel->rowCount(); ++row) {
            baseHeight += treeView()->sizeHintForRow(row);

            QModelIndex index(m_presentationModel->index(row, 0));
            if (index.isValid()) {
                for (int row2 = 0; row2 < m_presentationModel->rowCount(index); ++row2) {
                    int h = 0;
                    for (int a = 0; a < m_presentationModel->columnCount(index); ++a) {
                        const QModelIndex child = m_presentationModel->index(row2, a, index);
                        int localHeight = treeView()->sizeHintForIndex(child).height();
                        if (localHeight > h) {
                            h = localHeight;
                        }
                    }
                    baseHeight += h;
                    if (baseHeight > maxBaseHeight) {
                        break;
                    }
                }

                if (baseHeight > maxBaseHeight) {
                    break;
                }
            }
        }

        calculatedCustomHeight = baseHeight;
    }

    baseHeight += 2 * frameWidth();

    if (m_entryList->horizontalScrollBar()->isVisible()) {
        baseHeight += m_entryList->horizontalScrollBar()->height();
    }

    if (baseHeight < minBaseHeight) {
        baseHeight = minBaseHeight;
    }
    if (baseHeight > maxBaseHeight) {
        baseHeight = maxBaseHeight;
        m_entryList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else {
        // Somewhere there seems to be a bug that makes QTreeView add a scroll-bar
        // even if the content exactly fits in. So forcefully disable the scroll-bar in that case
        m_entryList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    int newExpandingAddedHeight = 0;

    if (baseHeight == maxBaseHeight) {
        // Eventually add some more height
        if (calculatedCustomHeight && calculatedCustomHeight > baseHeight && calculatedCustomHeight < maxBaseHeight) {
            newExpandingAddedHeight = calculatedCustomHeight - baseHeight;
        }
    }

    if (m_expandedAddedHeightBase != baseHeight && m_expandedAddedHeightBase - baseHeight > -2 && m_expandedAddedHeightBase - baseHeight < 2) {
        // Re-use the stored base-height if it only slightly differs from the current one.
        // Reason: Qt seems to apply slightly wrong sizes when the completion-widget is moved out of the screen at the bottom,
        //        which completely breaks this algorithm. Solution: re-use the old base-size if it only slightly differs from the computed one.
        baseHeight = m_expandedAddedHeightBase;
    }

    int finalHeight = baseHeight + newExpandingAddedHeight;

    if (finalHeight < 10) {
        m_entryList->resize(m_entryList->width(), height() - 2 * frameWidth());
        return;
    }

    m_expandedAddedHeightBase = geometry().height();

    geom.setHeight(finalHeight);

    // Work around a crash deep within the Qt 4.5 raster engine
    m_entryList->setScrollingEnabled(false);

    if (geometry() != geom) {
        setGeometry(geom);
    }

    QSize entryListSize = QSize(m_entryList->width(), finalHeight - 2 * frameWidth());
    if (m_entryList->size() != entryListSize) {
        m_entryList->resize(entryListSize);
    }

    m_entryList->setScrollingEnabled(true);
}

void KateCompletionWidget::cursorPositionChanged()
{
    ////qCDebug(LOG_KTE);
    if (m_completionRanges.isEmpty()) {
        return;
    }

    QModelIndex oldCurrentSourceIndex;
    if (m_entryList->currentIndex().isValid()) {
        oldCurrentSourceIndex = m_presentationModel->mapToSource(m_entryList->currentIndex());
    }

    QMap<KTextEditor::CodeCompletionModel *, QString> filterStringByModel;

    disconnect(this->model(), &KateCompletionModel::layoutChanged, this, &KateCompletionWidget::modelContentChanged);
    disconnect(this->model(), &KateCompletionModel::modelReset, this, &KateCompletionWidget::modelContentChanged);

    // Check the models and eventually abort some
    const QList<KTextEditor::CodeCompletionModel *> checkCompletionRanges = m_completionRanges.keys();
    for (auto model : checkCompletionRanges) {
        if (!m_completionRanges.contains(model)) {
            continue;
        }

        // qCDebug(LOG_KTE)<<"range before _updateRange:"<< *range;

        // this might invalidate the range, therefore re-check afterwards
        KTextEditor::Range rangeTE = m_completionRanges[model].range->toRange();
        KTextEditor::Range newRange = _updateRange(model, view(), rangeTE);
        if (!m_completionRanges.contains(model)) {
            continue;
        }

        // update value
        m_completionRanges[model].range->setRange(newRange);

        // qCDebug(LOG_KTE)<<"range after _updateRange:"<< *range;
        QString currentCompletion = _filterString(model, view(), *m_completionRanges[model].range, view()->cursorPosition());
        if (!m_completionRanges.contains(model)) {
            continue;
        }

        // qCDebug(LOG_KTE)<<"after _filterString, currentCompletion="<< currentCompletion;
        bool abort = _shouldAbortCompletion(model, view(), *m_completionRanges[model].range, currentCompletion);
        if (!m_completionRanges.contains(model)) {
            continue;
        }

        // qCDebug(LOG_KTE)<<"after _shouldAbortCompletion:abort="<<abort;
        if (view()->cursorPosition() < m_completionRanges[model].leftBoundary) {
            // qCDebug(LOG_KTE) << "aborting because of boundary:
            // cursor:"<<view()->cursorPosition()<<"completion_Range_left_boundary:"<<m_completionRanges[*it].leftBoundary;
            abort = true;
        }

        if (!m_completionRanges.contains(model)) {
            continue;
        }

        if (abort) {
            if (m_completionRanges.count() == 1) {
                // last model - abort whole completion
                abortCompletion();
                return;
            } else {
                {
                    delete m_completionRanges[model].range;
                    // qCDebug(LOG_KTE)<<"removing completion range 3";
                    m_completionRanges.remove(model);
                }

                _aborted(model, view());
                m_presentationModel->removeCompletionModel(model);
            }
        } else {
            filterStringByModel[model] = currentCompletion;
        }
    }

    connect(this->model(), &KateCompletionModel::layoutChanged, this, &KateCompletionWidget::modelContentChanged);
    connect(this->model(), &KateCompletionModel::modelReset, this, &KateCompletionWidget::modelContentChanged);

    m_presentationModel->setCurrentCompletion(filterStringByModel);

    if (oldCurrentSourceIndex.isValid()) {
        QModelIndex idx = m_presentationModel->mapFromSource(oldCurrentSourceIndex);
        // We only want to reselect this if it is still the first item
        if (idx.isValid() && idx.row() == 0) {
            // qCDebug(LOG_KTE) << "setting" << idx;
            m_entryList->setCurrentIndex(idx.sibling(idx.row(), 0));
            //       m_entryList->nextCompletion();
            //       m_entryList->previousCompletion();
        } else {
            // qCDebug(LOG_KTE) << "failed to map from source";
        }
    }

    m_entryList->scheduleUpdate();
}

bool KateCompletionWidget::isCompletionActive() const
{
    return !m_completionRanges.isEmpty() && ((!isHidden() && isVisible()) || (!m_argumentHintWidget->isHidden() && m_argumentHintWidget->isVisible()));
}

void KateCompletionWidget::abortCompletion()
{
    // qCDebug(LOG_KTE) ;

    m_isSuspended = false;

    if (!docTip()->isHidden()) {
        docTip()->hide();
    }

    bool wasActive = isCompletionActive();

    clear();

    if (!isHidden()) {
        hide();
    }

    if (!m_argumentHintWidget->isHidden()) {
        m_argumentHintWidget->hide();
    }

    if (wasActive) {
        view()->sendCompletionAborted();
    }
}

void KateCompletionWidget::clear()
{
    m_presentationModel->clearCompletionModels();
    m_argumentHintModel->clear();
    m_docTip->clearWidgets();

    const auto keys = m_completionRanges.keys();
    for (KTextEditor::CodeCompletionModel *model : keys) {
        _aborted(model, view());
    }

    deleteCompletionRanges();
}

bool KateCompletionWidget::navigateAccept()
{
    m_hadCompletionNavigation = true;

    if (currentEmbeddedWidget()) {
        QMetaObject::invokeMethod(currentEmbeddedWidget(), "embeddedWidgetAccept");
    }

    QModelIndex index = selectedIndex();
    if (index.isValid()) {
        index.data(KTextEditor::CodeCompletionModel::AccessibilityAccept);
        return true;
    }
    return false;
}

bool KateCompletionWidget::execute()
{
    // qCDebug(LOG_KTE) ;

    if (!isCompletionActive()) {
        return false;
    }

    QModelIndex index = selectedIndex();

    if (!index.isValid()) {
        abortCompletion();
        return false;
    }

    QModelIndex toExecute;

    if (index.model() == m_presentationModel) {
        toExecute = m_presentationModel->mapToSource(index);
    } else {
        toExecute = m_argumentHintModel->mapToSource(index);
    }

    if (!toExecute.isValid()) {
        qCWarning(LOG_KTE) << "Could not map index" << m_entryList->selectionModel()->currentIndex() << "to source index.";
        abortCompletion();
        return false;
    }

    // encapsulate all editing as being from the code completion, and undo-able in one step.
    view()->doc()->editStart();
    m_completionEditRunning = true;

    // create scoped pointer, to ensure deletion of cursor
    std::unique_ptr<KTextEditor::MovingCursor> oldPos(view()->doc()->newMovingCursor(view()->cursorPosition(), KTextEditor::MovingCursor::StayOnInsert));

    KTextEditor::CodeCompletionModel *model = static_cast<KTextEditor::CodeCompletionModel *>(const_cast<QAbstractItemModel *>(toExecute.model()));
    Q_ASSERT(model);

    Q_ASSERT(m_completionRanges.contains(model));

    KTextEditor::Cursor start = m_completionRanges[model].range->start();

    // Save the "tail"
    QString tailStr = tailString();
    std::unique_ptr<KTextEditor::MovingCursor> afterTailMCursor(view()->doc()->newMovingCursor(view()->cursorPosition()));
    afterTailMCursor->move(tailStr.size());

    // Handle completion for multi cursors
    std::shared_ptr<QMetaObject::Connection> connection(new QMetaObject::Connection());
    auto autoCompleteMulticursors = [connection, this](KTextEditor::Document *document, const KTextEditor::Range &range) {
        disconnect(*connection);
        const QString text = document->text(range);
        if (text.isEmpty()) {
            return;
        }
        const auto &multicursors = view()->secondaryCursors();
        for (const auto &c : multicursors) {
            const KTextEditor::Cursor pos = c.cursor();
            KTextEditor::Range wordToReplace = view()->doc()->wordRangeAt(pos);
            wordToReplace.setEnd(pos); // limit the word to the current cursor position
            view()->doc()->replaceText(wordToReplace, text);
        }
    };
    *connection = connect(view()->doc(), &KTextEditor::DocumentPrivate::textInsertedRange, this, autoCompleteMulticursors);

    model->executeCompletionItem(view(), *m_completionRanges[model].range, toExecute);
    // NOTE the CompletionRange is now removed from m_completionRanges

    // There are situations where keeping the tail is beneficial, but with the "Remove tail on complete" option is enabled,
    // the tail is removed. For these situations we convert the completion into two edits:
    // 1) Insert the completion
    // 2) Remove the tail
    //
    // When we encounter one of these situations we can just do _one_ undo to have the tail back.
    //
    // Technically the tail is already removed by "executeCompletionItem()", so before this call we save the possible tail
    // and re-add the tail before we end the first grouped "edit". Then immediately after that we add a second edit that
    // removes the tail again.
    // NOTE: The ViInputMode makes assumptions about the edit actions in a completion and breaks if we insert extra
    // edits here, so we just disable this feature for ViInputMode
    if (!tailStr.isEmpty() && view()->viewInputMode() != KTextEditor::View::ViInputMode) {
        KTextEditor::Cursor currentPos = view()->cursorPosition();
        KTextEditor::Cursor afterPos = afterTailMCursor->toCursor();
        // Re add the tail for a possible undo to bring the tail back
        view()->document()->insertText(afterPos, tailStr);
        view()->setCursorPosition(currentPos);
        view()->doc()->editEnd();

        // Now remove the tail in a separate edit
        KTextEditor::Cursor endPos = afterPos;
        endPos.setColumn(afterPos.column() + tailStr.size());
        view()->doc()->editStart();
        view()->document()->removeText(KTextEditor::Range(afterPos, endPos));
    }

    view()->doc()->editEnd();
    m_completionEditRunning = false;

    abortCompletion();

    view()->sendCompletionExecuted(start, model, toExecute);

    KTextEditor::Cursor newPos = view()->cursorPosition();

    if (newPos > *oldPos) {
        m_automaticInvocationAt = newPos;
        m_automaticInvocationLine = view()->doc()->text(KTextEditor::Range(*oldPos, newPos));
        // qCDebug(LOG_KTE) << "executed, starting automatic invocation with line" << m_automaticInvocationLine;
        m_lastInsertionByUser = false;
        m_automaticInvocationTimer->start();
    }

    return true;
}

void KateCompletionWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    // keep argument hint geometry in sync
    if (m_argumentHintWidget->isVisible()) {
        updateArgumentHintGeometry();
    }
}

void KateCompletionWidget::moveEvent(QMoveEvent *event)
{
    QFrame::moveEvent(event);

    // keep argument hint geometry in sync
    if (m_argumentHintWidget->isVisible()) {
        updateArgumentHintGeometry();
    }
}

void KateCompletionWidget::showEvent(QShowEvent *event)
{
    m_isSuspended = false;

    QFrame::showEvent(event);

    if (!m_dontShowArgumentHints && m_argumentHintModel->rowCount(QModelIndex()) != 0) {
        m_argumentHintWidget->positionAndShow();
    }
}

KTextEditor::MovingRange *KateCompletionWidget::completionRange(KTextEditor::CodeCompletionModel *model) const
{
    if (!model) {
        if (m_completionRanges.isEmpty()) {
            return nullptr;
        }

        KTextEditor::MovingRange *ret = m_completionRanges.begin()->range;

        for (const CompletionRange &range : m_completionRanges) {
            if (range.range->start() > ret->start()) {
                ret = range.range;
            }
        }
        return ret;
    }
    if (m_completionRanges.contains(model)) {
        return m_completionRanges[model].range;
    } else {
        return nullptr;
    }
}

QMap<KTextEditor::CodeCompletionModel *, KateCompletionWidget::CompletionRange> KateCompletionWidget::completionRanges() const
{
    return m_completionRanges;
}

void KateCompletionWidget::modelReset()
{
    setUpdatesEnabled(false);
    m_entryList->setAnimated(false);

    for (int row = 0; row < m_entryList->model()->rowCount(QModelIndex()); ++row) {
        QModelIndex index(m_entryList->model()->index(row, 0, QModelIndex()));
        if (!m_entryList->isExpanded(index)) {
            m_entryList->expand(index);
        }
    }
    setUpdatesEnabled(true);
}

KateCompletionTree *KateCompletionWidget::treeView() const
{
    return m_entryList;
}

QModelIndex KateCompletionWidget::selectedIndex() const
{
    if (!isCompletionActive()) {
        return QModelIndex();
    }

    return m_entryList->currentIndex();
}

bool KateCompletionWidget::navigateLeft()
{
    m_hadCompletionNavigation = true;
    if (currentEmbeddedWidget()) {
        QMetaObject::invokeMethod(currentEmbeddedWidget(), "embeddedWidgetLeft");
    }

    QModelIndex index = selectedIndex();

    if (index.isValid()) {
        index.data(KTextEditor::CodeCompletionModel::AccessibilityPrevious);

        return true;
    }
    return false;
}

bool KateCompletionWidget::navigateRight()
{
    m_hadCompletionNavigation = true;
    if (currentEmbeddedWidget()) { ///@todo post 4.2: Make these slots public interface, or create an interface using virtual functions
        QMetaObject::invokeMethod(currentEmbeddedWidget(), "embeddedWidgetRight");
    }

    QModelIndex index = selectedIndex();

    if (index.isValid()) {
        index.data(KTextEditor::CodeCompletionModel::AccessibilityNext);
        return true;
    }

    return false;
}

bool KateCompletionWidget::navigateBack()
{
    m_hadCompletionNavigation = true;
    if (currentEmbeddedWidget()) {
        QMetaObject::invokeMethod(currentEmbeddedWidget(), "embeddedWidgetBack");
    }
    return false;
}

void KateCompletionWidget::toggleDocumentation()
{
    // user has configured the doc to be always visible
    // whenever its available.
    if (view()->config()->showDocWithCompletion()) {
        return;
    }

    if (m_docTip->isVisible()) {
        m_hadCompletionNavigation = false;
        QTimer::singleShot(400, this, [this] {
            // if 400ms later this is not false, it means
            // that the user navigated inside the active
            // widget in doc tip
            if (!m_hadCompletionNavigation) {
                m_docTip->hide();
            }
        });
    } else {
        showDocTip(m_entryList->currentIndex());
    }
}

void KateCompletionWidget::showDocTip(const QModelIndex &idx)
{
    auto data = idx.data(KTextEditor::CodeCompletionModel::ExpandingWidget);
    // No data => hide
    if (!data.isValid()) {
        m_docTip->hide();
        return;
    } else if (data.canConvert<QWidget *>()) {
        m_docTip->setWidget(data.value<QWidget *>());
    } else if (data.canConvert<QString>()) {
        QString text = data.toString();
        if (text.isEmpty()) {
            m_docTip->hide();
            return;
        }
        m_docTip->setText(text);
    }

    m_docTip->updatePosition(this);
    if (!m_docTip->isVisible()) {
        m_docTip->show();
    }
}

bool KateCompletionWidget::handleShortcutOverride(QKeyEvent *e)
{
    if (!isCompletionActive() || e->modifiers() != Qt::AltModifier) {
        return false;
    }
    switch (e->key()) {
    case Qt::Key_Left:
        return navigateLeft();
    case Qt::Key_Right:
        return navigateRight();
    case Qt::Key_Up:
        return navigateUp();
    case Qt::Key_Down:
        return navigateDown();
    case Qt::Key_Return:
        return navigateAccept();
    case Qt::Key_Backspace:
        return navigateBack();
    }
    return false;
}

bool KateCompletionWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != this && event->type() == QEvent::Resize && isCompletionActive()) {
        abortCompletion();
    }
    return QFrame::eventFilter(watched, event);
}

bool KateCompletionWidget::navigateDown()
{
    m_hadCompletionNavigation = true;
    if (m_argumentHintModel->rowCount() > 0) {
        m_argumentHintWidget->selectNext();
        return true;
    } else if (currentEmbeddedWidget()) {
        QMetaObject::invokeMethod(currentEmbeddedWidget(), "embeddedWidgetDown");
    }
    return false;
}

bool KateCompletionWidget::navigateUp()
{
    m_hadCompletionNavigation = true;
    if (m_argumentHintModel->rowCount() > 0) {
        m_argumentHintWidget->selectPrevious();
        return true;
    } else if (currentEmbeddedWidget()) {
        QMetaObject::invokeMethod(currentEmbeddedWidget(), "embeddedWidgetUp");
    }
    return false;
}

QWidget *KateCompletionWidget::currentEmbeddedWidget()
{
    return m_docTip->currentWidget();
}

void KateCompletionWidget::cursorDown()
{
    m_entryList->nextCompletion();
}

void KateCompletionWidget::cursorUp()
{
    m_entryList->previousCompletion();
}

void KateCompletionWidget::pageDown()
{
    m_entryList->pageDown();
}

void KateCompletionWidget::pageUp()
{
    m_entryList->pageUp();
}

void KateCompletionWidget::top()
{
    m_entryList->top();
}

void KateCompletionWidget::bottom()
{
    m_entryList->bottom();
}

void KateCompletionWidget::completionModelReset()
{
    KTextEditor::CodeCompletionModel *model = qobject_cast<KTextEditor::CodeCompletionModel *>(sender());
    if (!model) {
        qCWarning(LOG_KTE) << "bad sender";
        return;
    }

    if (!m_waitingForReset.contains(model)) {
        return;
    }

    m_waitingForReset.remove(model);

    if (m_waitingForReset.isEmpty()) {
        if (!isCompletionActive()) {
            // qCDebug(LOG_KTE) << "all completion-models we waited for are ready. Last one: " << model->objectName();
            // Eventually show the completion-list if this was the last model we were waiting for
            // Use a queued connection once again to make sure that KateCompletionModel is notified before we are
            QMetaObject::invokeMethod(this, "modelContentChanged", Qt::QueuedConnection);
        }
    }
}

void KateCompletionWidget::modelDestroyed(QObject *model)
{
    m_sourceModels.removeAll(model);
    abortCompletion();
}

void KateCompletionWidget::registerCompletionModel(KTextEditor::CodeCompletionModel *model)
{
    if (m_sourceModels.contains(model)) {
        return;
    }

    connect(model, &KTextEditor::CodeCompletionModel::destroyed, this, &KateCompletionWidget::modelDestroyed);
    // This connection must not be queued
    connect(model, &KTextEditor::CodeCompletionModel::modelReset, this, &KateCompletionWidget::completionModelReset);

    m_sourceModels.append(model);

    if (isCompletionActive()) {
        m_presentationModel->addCompletionModel(model);
    }
}

void KateCompletionWidget::unregisterCompletionModel(KTextEditor::CodeCompletionModel *model)
{
    disconnect(model, &KTextEditor::CodeCompletionModel::destroyed, this, &KateCompletionWidget::modelDestroyed);
    disconnect(model, &KTextEditor::CodeCompletionModel::modelReset, this, &KateCompletionWidget::completionModelReset);

    m_sourceModels.removeAll(model);
    abortCompletion();
}

bool KateCompletionWidget::isCompletionModelRegistered(KTextEditor::CodeCompletionModel *model) const
{
    return m_sourceModels.contains(model);
}

QList<KTextEditor::CodeCompletionModel *> KateCompletionWidget::codeCompletionModels() const
{
    return m_sourceModels;
}

int KateCompletionWidget::automaticInvocationDelay() const
{
    return m_automaticInvocationDelay;
}

void KateCompletionWidget::setIgnoreBufferSignals(bool ignore) const
{
    if (ignore) {
        disconnect(view()->doc(), &KTextEditor::Document::lineWrapped, this, &KateCompletionWidget::wrapLine);
        disconnect(view()->doc(), &KTextEditor::Document::lineUnwrapped, this, &KateCompletionWidget::unwrapLine);
        disconnect(view()->doc(), &KTextEditor::Document::textInserted, this, &KateCompletionWidget::insertText);
        disconnect(view()->doc(), &KTextEditor::Document::textRemoved, this, &KateCompletionWidget::removeText);
    } else {
        connect(view()->doc(), &KTextEditor::Document::lineWrapped, this, &KateCompletionWidget::wrapLine);
        connect(view()->doc(), &KTextEditor::Document::lineUnwrapped, this, &KateCompletionWidget::unwrapLine);
        connect(view()->doc(), &KTextEditor::Document::textInserted, this, &KateCompletionWidget::insertText);
        connect(view()->doc(), &KTextEditor::Document::textRemoved, this, &KateCompletionWidget::removeText);
    }
}

void KateCompletionWidget::setAutomaticInvocationDelay(int delay)
{
    m_automaticInvocationDelay = delay;
}

void KateCompletionWidget::wrapLine(KTextEditor::Document *, KTextEditor::Cursor)
{
    m_lastInsertionByUser = !m_completionEditRunning;

    // wrap line, be done
    m_automaticInvocationLine.clear();
    m_automaticInvocationTimer->stop();
}

void KateCompletionWidget::unwrapLine(KTextEditor::Document *, int)
{
    m_lastInsertionByUser = !m_completionEditRunning;

    // just removal
    m_automaticInvocationLine.clear();
    m_automaticInvocationTimer->stop();
}

void KateCompletionWidget::insertText(KTextEditor::Document *, KTextEditor::Cursor position, const QString &text)
{
    m_lastInsertionByUser = !m_completionEditRunning;

    // no invoke?
    if (!view()->isAutomaticInvocationEnabled()) {
        m_automaticInvocationLine.clear();
        m_automaticInvocationTimer->stop();
        return;
    }

    if (m_automaticInvocationAt != position) {
        m_automaticInvocationLine.clear();
        m_lastInsertionByUser = !m_completionEditRunning;
    }

    m_automaticInvocationLine += text;
    m_automaticInvocationAt = position;
    m_automaticInvocationAt.setColumn(position.column() + text.length());

    if (m_automaticInvocationLine.isEmpty()) {
        m_automaticInvocationTimer->stop();
        return;
    }

    m_automaticInvocationTimer->start(m_automaticInvocationDelay);
}

void KateCompletionWidget::removeText(KTextEditor::Document *, KTextEditor::Range, const QString &)
{
    m_lastInsertionByUser = !m_completionEditRunning;

    // just removal
    m_automaticInvocationLine.clear();
    m_automaticInvocationTimer->stop();
}

void KateCompletionWidget::automaticInvocation()
{
    // qCDebug(LOG_KTE)<<"m_automaticInvocationAt:"<<m_automaticInvocationAt;
    // qCDebug(LOG_KTE)<<view()->cursorPosition();
    if (m_automaticInvocationAt != view()->cursorPosition()) {
        return;
    }

    bool start = false;
    QList<KTextEditor::CodeCompletionModel *> models;

    // qCDebug(LOG_KTE)<<"checking models";
    for (KTextEditor::CodeCompletionModel *model : std::as_const(m_sourceModels)) {
        // qCDebug(LOG_KTE)<<"m_completionRanges contains model?:"<<m_completionRanges.contains(model);
        if (m_completionRanges.contains(model)) {
            continue;
        }

        start = _shouldStartCompletion(model, view(), m_automaticInvocationLine, m_lastInsertionByUser, view()->cursorPosition());
        // qCDebug(LOG_KTE)<<"start="<<start;
        if (start) {
            models << model;
        }
    }
    // qCDebug(LOG_KTE)<<"models found:"<<!models.isEmpty();
    if (!models.isEmpty()) {
        // Start automatic code completion
        startCompletion(KTextEditor::CodeCompletionModel::AutomaticInvocation, models);
    }
}

void KateCompletionWidget::userInvokedCompletion()
{
    startCompletion(KTextEditor::CodeCompletionModel::UserInvocation);
}

void KateCompletionWidget::tabCompletion(Direction direction)
{
    m_noAutoHide = true;

    // Not using cursorDown/Up() as we don't want to go into the argument-hint list
    if (direction == Down) {
        const bool res = m_entryList->nextCompletion();
        if (!res) {
            m_entryList->top();
        }
    } else { // direction == Up
        const bool res = m_entryList->previousCompletion();
        if (!res) {
            m_entryList->bottom();
        }
    }
}

#include "moc_katecompletionwidget.cpp"

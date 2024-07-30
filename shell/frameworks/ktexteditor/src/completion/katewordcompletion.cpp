/*
    SPDX-FileCopyrightText: 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// BEGIN includes
#include "katewordcompletion.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "kateview.h"

#include <ktexteditor/movingrange.h>
#include <ktexteditor/range.h>

#include <KAboutData>
#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPageDialog>
#include <KPageWidgetModel>
#include <KParts/Part>
#include <KToggleAction>

#include <Sonnet/Speller>

#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRegularExpression>
#include <QSet>
#include <QSpinBox>
#include <QString>

// END

/// amount of lines to scan backwards and forwards
static const int maxLinesToScan = 10000;

// BEGIN KateWordCompletionModel
KateWordCompletionModel::KateWordCompletionModel(QObject *parent)
    : CodeCompletionModel(parent)
    , m_automatic(false)
{
    setHasGroups(false);
}

KateWordCompletionModel::~KateWordCompletionModel()
{
}

void KateWordCompletionModel::saveMatches(KTextEditor::View *view, const KTextEditor::Range &range)
{
    m_matches = allMatches(view, range);
    m_matches.sort();
}

QVariant KateWordCompletionModel::data(const QModelIndex &index, int role) const
{
    if (role == UnimportantItemRole) {
        return QVariant(true);
    }
    if (role == InheritanceDepth) {
        return 10000;
    }

    if (!index.parent().isValid()) {
        // It is the group header
        switch (role) {
        case Qt::DisplayRole:
            return i18n("Auto Word Completion");
        case GroupRole:
            return Qt::DisplayRole;
        }
    }

    if (index.column() == KTextEditor::CodeCompletionModel::Name && role == Qt::DisplayRole) {
        return m_matches.at(index.row());
    }

    if (index.column() == KTextEditor::CodeCompletionModel::Icon && role == Qt::DecorationRole) {
        static QIcon icon(QIcon::fromTheme(QStringLiteral("insert-text")).pixmap(QSize(16, 16)));
        return icon;
    }

    return QVariant();
}

QModelIndex KateWordCompletionModel::parent(const QModelIndex &index) const
{
    if (index.internalId()) {
        return createIndex(0, 0, quintptr(0));
    } else {
        return QModelIndex();
    }
}

QModelIndex KateWordCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row == 0) {
            return createIndex(row, column, quintptr(0));
        } else {
            return QModelIndex();
        }

    } else if (parent.parent().isValid()) {
        return QModelIndex();
    }

    if (row < 0 || row >= m_matches.count() || column < 0 || column >= ColumnCount) {
        return QModelIndex();
    }

    return createIndex(row, column, 1);
}

int KateWordCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid() && !m_matches.isEmpty()) {
        return 1; // One root node to define the custom group
    } else if (parent.parent().isValid()) {
        return 0; // Completion-items have no children
    } else {
        return m_matches.count();
    }
}

bool KateWordCompletionModel::shouldStartCompletion(KTextEditor::View *view,
                                                    const QString &insertedText,
                                                    bool userInsertion,
                                                    const KTextEditor::Cursor &position)
{
    if (!userInsertion) {
        return false;
    }
    if (insertedText.isEmpty()) {
        return false;
    }

    KTextEditor::ViewPrivate *v = qobject_cast<KTextEditor::ViewPrivate *>(view);

    const QString &text = view->document()->line(position.line()).left(position.column());
    const uint check = v->config()->wordCompletionMinimalWordLength();
    // Start completion immediately if min. word size is zero
    if (!check) {
        return true;
    }
    // Otherwise, check if user has typed long enough text...
    const int start = text.length();
    const int end = start - check;
    if (end < 0) {
        return false;
    }
    for (int i = start - 1; i >= end; i--) {
        const QChar c = text.at(i);
        if (!(c.isLetter() || (c.isNumber()) || c == QLatin1Char('_'))) {
            return false;
        }
    }

    return true;
}

bool KateWordCompletionModel::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion)
{
    if (m_automatic) {
        KTextEditor::ViewPrivate *v = qobject_cast<KTextEditor::ViewPrivate *>(view);
        if (currentCompletion.length() < v->config()->wordCompletionMinimalWordLength()) {
            return true;
        }
    }

    return CodeCompletionModelControllerInterface::shouldAbortCompletion(view, range, currentCompletion);
}

void KateWordCompletionModel::completionInvoked(KTextEditor::View *view, const KTextEditor::Range &range, InvocationType it)
{
    m_automatic = it == AutomaticInvocation;
    saveMatches(view, range);
}

/**
 * Scan throughout the entire document for possible completions,
 * ignoring any dublets and words shorter than configured and/or
 * reasonable minimum length.
 */
QStringList KateWordCompletionModel::allMatches(KTextEditor::View *view, const KTextEditor::Range &range)
{
    QSet<QStringView> result;
    const int minWordSize = qMax(2, qobject_cast<KTextEditor::ViewPrivate *>(view)->config()->wordCompletionMinimalWordLength());
    const auto cursorPosition = view->cursorPosition();
    const auto document = view->document();
    const int startLine = std::max(0, cursorPosition.line() - maxLinesToScan);
    const int endLine = std::min(cursorPosition.line() + maxLinesToScan, view->document()->lines());
    for (int line = startLine; line < endLine; line++) {
        const QString text = document->line(line);
        if (text.isEmpty() || text.isNull()) {
            continue;
        }
        QStringView textView = text;
        int wordBegin = 0;
        int offset = 0;
        const int end = text.size();
        const bool cursorLine = cursorPosition.line() == line;
        const bool isNotLastLine = line != range.end().line();
        const QChar *d = text.data();
        while (offset < end) {
            const QChar c = d[offset];
            // increment offset when at line end, so we take the last character too
            if ((!c.isLetterOrNumber() && c != QChar(u'_')) || (offset == end - 1 && offset++)) {
                if (offset - wordBegin >= minWordSize && (isNotLastLine || offset != range.end().column())) {
                    // don't add the word we are inside with cursor!
                    if (!cursorLine || (cursorPosition.column() < wordBegin || cursorPosition.column() > offset)) {
                        result.insert(textView.mid(wordBegin, offset - wordBegin));
                    }
                }
                wordBegin = offset + 1;
            }
            if (c.isSpace()) {
                wordBegin = offset + 1;
            }
            offset += 1;
        }
    }

    // ensure words that are ok spell check wise always end up in the completion, see bug 468705
    const auto language = static_cast<KTextEditor::DocumentPrivate *>(document)->defaultDictionary();
    const auto word = view->document()->text(range);
    Sonnet::Speller speller;
    QStringList spellerSuggestions;
    speller.setLanguage(language);
    if (speller.isValid()) {
        if (speller.isCorrect(word)) {
            result.insert(word);
        } else {
            spellerSuggestions = speller.suggest(word);
            for (const auto &alternative : std::as_const(spellerSuggestions)) {
                result.insert(alternative);
            }
        }
    }

    m_matches.clear();
    m_matches.reserve(result.size());
    for (auto v : std::as_const(result)) {
        m_matches << v.toString();
    }

    return m_matches;
}

void KateWordCompletionModel::executeCompletionItem(KTextEditor::View *view, const KTextEditor::Range &word, const QModelIndex &index) const
{
    view->document()->replaceText(word, m_matches.at(index.row()));
}

KTextEditor::CodeCompletionModelControllerInterface::MatchReaction KateWordCompletionModel::matchingItem(const QModelIndex & /*matched*/)
{
    return HideListIfAutomaticInvocation;
}

bool KateWordCompletionModel::shouldHideItemsWithEqualNames() const
{
    // We don't want word-completion items if the same items
    // are available through more sophisticated completion models
    return true;
}

// END KateWordCompletionModel

// BEGIN KateWordCompletionView
struct KateWordCompletionViewPrivate {
    KTextEditor::MovingRange *liRange; // range containing last inserted text
    KTextEditor::Range dcRange; // current range to be completed by directional completion
    KTextEditor::Cursor dcCursor; // directional completion search cursor
    int directionalPos; // be able to insert "" at the correct time
    bool isCompleting; // true when the directional completion is doing a completion
};

KateWordCompletionView::KateWordCompletionView(KTextEditor::View *view, KActionCollection *ac)
    : QObject(view)
    , m_view(view)
    , m_dWCompletionModel(KTextEditor::EditorPrivate::self()->wordCompletionModel())
    , d(new KateWordCompletionViewPrivate)
{
    d->isCompleting = false;
    d->dcRange = KTextEditor::Range::invalid();

    d->liRange =
        static_cast<KTextEditor::DocumentPrivate *>(m_view->document())->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::DoNotExpand);

    KTextEditor::Attribute::Ptr a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    a->setBackground(static_cast<KTextEditor::ViewPrivate *>(view)->rendererConfig()->selectionColor());
    d->liRange->setAttribute(a);

    QAction *action;

    action = new QAction(i18n("Shell Completion"), this);
    ac->addAction(QStringLiteral("doccomplete_sh"), action);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(action, &QAction::triggered, this, &KateWordCompletionView::shellComplete);

    action = new QAction(i18n("Reuse Word Above"), this);
    ac->addAction(QStringLiteral("doccomplete_bw"), action);
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::Key_8);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(action, &QAction::triggered, this, &KateWordCompletionView::completeBackwards);

    action = new QAction(i18n("Reuse Word Below"), this);
    ac->addAction(QStringLiteral("doccomplete_fw"), action);
    ac->setDefaultShortcut(action, Qt::CTRL | Qt::Key_9);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(action, &QAction::triggered, this, &KateWordCompletionView::completeForwards);
}

KateWordCompletionView::~KateWordCompletionView()
{
    delete d;
}

void KateWordCompletionView::completeBackwards()
{
    complete(false);
}

void KateWordCompletionView::completeForwards()
{
    complete();
}

// Pop up the editors completion list if applicable
void KateWordCompletionView::popupCompletionList()
{
    qCDebug(LOG_KTE) << "entered ...";
    KTextEditor::Range r = range();

    if (m_view->isCompletionActive()) {
        return;
    }

    m_dWCompletionModel->saveMatches(m_view, r);

    qCDebug(LOG_KTE) << "after save matches ...";

    if (!m_dWCompletionModel->rowCount(QModelIndex())) {
        return;
    }

    m_view->startCompletion(r, m_dWCompletionModel);
}

// Contributed by <brain@hdsnet.hu>
void KateWordCompletionView::shellComplete()
{
    KTextEditor::Range r = range();

    const QStringList matches = m_dWCompletionModel->allMatches(m_view, r);

    if (matches.size() == 0) {
        return;
    }

    QString partial = findLongestUnique(matches, r.columnWidth());

    if (partial.isEmpty()) {
        popupCompletionList();
    }

    else {
        m_view->document()->insertText(r.end(), partial.mid(r.columnWidth()));
        d->liRange->setView(m_view);
        d->liRange->setRange(KTextEditor::Range(r.end(), partial.length() - r.columnWidth()));
        connect(m_view, &KTextEditor::View::cursorPositionChanged, this, &KateWordCompletionView::slotCursorMoved);
    }
}

// Do one completion, searching in the desired direction,
// if possible
void KateWordCompletionView::complete(bool fw)
{
    KTextEditor::Range r = range();

    int inc = fw ? 1 : -1;
    KTextEditor::Document *doc = m_view->document();

    if (d->dcRange.isValid()) {
        // qCDebug(LOG_KTE)<<"CONTINUE "<<d->dcRange;
        // this is a repeated activation

        // if we are back to where we started, reset.
        if ((fw && d->directionalPos == -1) || (!fw && d->directionalPos == 1)) {
            const int spansColumns = d->liRange->end().column() - d->liRange->start().column();
            if (spansColumns > 0) {
                doc->removeText(*d->liRange);
            }

            d->liRange->setRange(KTextEditor::Range::invalid());
            d->dcCursor = r.end();
            d->directionalPos = 0;

            return;
        }

        if (fw) {
            const int spansColumns = d->liRange->end().column() - d->liRange->start().column();
            d->dcCursor.setColumn(d->dcCursor.column() + spansColumns);
        }

        d->directionalPos += inc;
    } else { // new completion, reset all
        // qCDebug(LOG_KTE)<<"RESET FOR NEW";
        d->dcRange = r;
        d->liRange->setRange(KTextEditor::Range::invalid());
        d->dcCursor = r.start();
        d->directionalPos = inc;

        d->liRange->setView(m_view);

        connect(m_view, &KTextEditor::View::cursorPositionChanged, this, &KateWordCompletionView::slotCursorMoved);
    }

    const QRegularExpression wordRegEx(QLatin1String("\\b") + doc->text(d->dcRange) + QLatin1String("(\\w+)"), QRegularExpression::UseUnicodePropertiesOption);
    int pos(0);
    QString ln = doc->line(d->dcCursor.line());

    while (true) {
        // qCDebug(LOG_KTE)<<"SEARCHING FOR "<<wordRegEx.pattern()<<" "<<ln<<" at "<<d->dcCursor;
        QRegularExpressionMatch match;
        pos = fw ? ln.indexOf(wordRegEx, d->dcCursor.column(), &match) : ln.lastIndexOf(wordRegEx, d->dcCursor.column(), &match);

        if (match.hasMatch()) { // we matched a word
            // qCDebug(LOG_KTE)<<"USABLE MATCH";
            const QStringView m = match.capturedView(1);
            if (m != doc->text(*d->liRange) && (d->dcCursor.line() != d->dcRange.start().line() || pos != d->dcRange.start().column())) {
                // we got good a match! replace text and return.
                d->isCompleting = true;
                KTextEditor::Range replaceRange(d->liRange->toRange());
                if (!replaceRange.isValid()) {
                    replaceRange.setRange(r.end(), r.end());
                }
                doc->replaceText(replaceRange, m.toString());
                d->liRange->setRange(KTextEditor::Range(d->dcRange.end(), m.length()));

                d->dcCursor.setColumn(pos); // for next try

                d->isCompleting = false;
                return;
            }

            // equal to last one, continue
            else {
                // qCDebug(LOG_KTE)<<"SKIPPING, EQUAL MATCH";
                d->dcCursor.setColumn(pos); // for next try

                if (fw) {
                    d->dcCursor.setColumn(pos + m.length());
                }

                else {
                    if (pos == 0) {
                        if (d->dcCursor.line() > 0) {
                            int l = d->dcCursor.line() + inc;
                            ln = doc->line(l);
                            d->dcCursor.setPosition(l, ln.length());
                        } else {
                            return;
                        }
                    }

                    else {
                        d->dcCursor.setColumn(d->dcCursor.column() - 1);
                    }
                }
            }
        }

        else { // no match
            // qCDebug(LOG_KTE)<<"NO MATCH";
            if ((!fw && d->dcCursor.line() == 0) || (fw && d->dcCursor.line() >= doc->lines())) {
                return;
            }

            int l = d->dcCursor.line() + inc;
            ln = doc->line(l);
            d->dcCursor.setPosition(l, fw ? 0 : ln.length());
        }
    } // while true
}

void KateWordCompletionView::slotCursorMoved()
{
    if (d->isCompleting) {
        return;
    }

    d->dcRange = KTextEditor::Range::invalid();

    disconnect(m_view, &KTextEditor::View::cursorPositionChanged, this, &KateWordCompletionView::slotCursorMoved);

    d->liRange->setView(nullptr);
    d->liRange->setRange(KTextEditor::Range::invalid());
}

// Contributed by <brain@hdsnet.hu> FIXME
QString KateWordCompletionView::findLongestUnique(const QStringList &matches, int lead)
{
    QString partial = matches.first();

    for (const QString &current : matches) {
        if (!current.startsWith(partial)) {
            while (partial.length() > lead) {
                partial.remove(partial.length() - 1, 1);
                if (current.startsWith(partial)) {
                    break;
                }
            }

            if (partial.length() == lead) {
                return QString();
            }
        }
    }

    return partial;
}

// Return the string to complete (the letters behind the cursor)
QString KateWordCompletionView::word() const
{
    return m_view->document()->text(range());
}

// Return the range containing the word behind the cursor
KTextEditor::Range KateWordCompletionView::range() const
{
    return m_dWCompletionModel->completionRange(m_view, m_view->cursorPosition());
}
// END

#include "moc_katewordcompletion.cpp"

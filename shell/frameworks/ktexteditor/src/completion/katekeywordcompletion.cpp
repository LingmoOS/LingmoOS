/*
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katekeywordcompletion.h"

#include "katedocument.h"
#include "katehighlight.h"
#include "katetextline.h"

#include <ktexteditor/view.h>

#include <KLocalizedString>
#include <QString>

KateKeywordCompletionModel::KateKeywordCompletionModel(QObject *parent)
    : CodeCompletionModel(parent)
{
    setHasGroups(false);
}

void KateKeywordCompletionModel::completionInvoked(KTextEditor::View *view,
                                                   const KTextEditor::Range &range,
                                                   KTextEditor::CodeCompletionModel::InvocationType /*invocationType*/)
{
    KTextEditor::DocumentPrivate *doc = static_cast<KTextEditor::DocumentPrivate *>(view->document());
    if (!doc->highlight() || doc->highlight()->noHighlighting()) {
        return;
    }
    m_items = doc->highlight()->keywordsForLocation(doc, range.end());
    std::sort(m_items.begin(), m_items.end());
}

QModelIndex KateKeywordCompletionModel::parent(const QModelIndex &index) const
{
    if (index.internalId()) {
        return createIndex(0, 0);
    } else {
        return QModelIndex();
    }
}

QModelIndex KateKeywordCompletionModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        if (row == 0) {
            return createIndex(row, column);
        } else {
            return QModelIndex();
        }
    } else if (parent.parent().isValid()) {
        return QModelIndex();
    }

    if (row < 0 || row >= m_items.count() || column < 0 || column >= ColumnCount) {
        return QModelIndex();
    }

    return createIndex(row, column, 1);
}

int KateKeywordCompletionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid() && !m_items.isEmpty()) {
        return 1; // One root node to define the custom group
    } else if (parent.parent().isValid()) {
        return 0; // Completion-items have no children
    } else {
        return m_items.count();
    }
}

static bool isInWord(const KTextEditor::View *view, const KTextEditor::Cursor &position, QChar c)
{
    KTextEditor::DocumentPrivate *document = static_cast<KTextEditor::DocumentPrivate *>(view->document());
    KateHighlighting *highlight = document->highlight();
    Kate::TextLine line = document->kateTextLine(position.line());
    return highlight->isInWord(c, line.attribute(position.column() - 1));
}

KTextEditor::Range KateKeywordCompletionModel::completionRange(KTextEditor::View *view, const KTextEditor::Cursor &position)
{
    const QString &text = view->document()->text(KTextEditor::Range(position, KTextEditor::Cursor(position.line(), 0)));
    int pos;
    for (pos = text.size() - 1; pos >= 0; pos--) {
        if (isInWord(view, position, text.at(pos))) {
            // This needs to be aware of what characters are word-characters in the
            // active language, so that languages which prefix commands with e.g. @
            // or \ have properly working completion.
            continue;
        }
        break;
    }
    return KTextEditor::Range(KTextEditor::Cursor(position.line(), pos + 1), position);
}

bool KateKeywordCompletionModel::shouldAbortCompletion(KTextEditor::View *view, const KTextEditor::Range &range, const QString &currentCompletion)
{
    if (view->cursorPosition() < range.start() || view->cursorPosition() > range.end()) {
        return true; // Always abort when the completion-range has been left
    }
    // Do not abort completions when the text has been empty already before and a newline has been entered

    for (QChar c : currentCompletion) {
        if (!isInWord(view, range.start(), c)) {
            return true;
        }
    }
    return false;
}

bool KateKeywordCompletionModel::shouldStartCompletion(KTextEditor::View * /*view*/,
                                                       const QString &insertedText,
                                                       bool userInsertion,
                                                       const KTextEditor::Cursor & /*position*/)
{
    if (userInsertion && insertedText.size() > 3 && !insertedText.contains(QLatin1Char(' ')) && insertedText.at(insertedText.size() - 1).isLetter()) {
        return true;
    }
    return false;
}

bool KateKeywordCompletionModel::shouldHideItemsWithEqualNames() const
{
    return true;
}

QVariant KateKeywordCompletionModel::data(const QModelIndex &index, int role) const
{
    if (role == UnimportantItemRole) {
        return QVariant(true);
    }
    if (role == InheritanceDepth) {
        return 9000;
    }

    if (!index.parent().isValid()) {
        // group header
        switch (role) {
        case Qt::DisplayRole:
            return i18n("Language keywords");
        case GroupRole:
            return Qt::DisplayRole;
        }
    }

    if (index.column() == KTextEditor::CodeCompletionModel::Name && role == Qt::DisplayRole) {
        return m_items.at(index.row());
    }

    if (index.column() == KTextEditor::CodeCompletionModel::Icon && role == Qt::DecorationRole) {
        static const QIcon icon(QIcon::fromTheme(QStringLiteral("code-variable")).pixmap(QSize(16, 16)));
        return icon;
    }

    return QVariant();
}

KTextEditor::CodeCompletionModelControllerInterface::MatchReaction KateKeywordCompletionModel::matchingItem(const QModelIndex & /*matched*/)
{
    return KTextEditor::CodeCompletionModelControllerInterface::HideListIfAutomaticInvocation;
}

#include "moc_katekeywordcompletion.cpp"

// kate: indent-width 4; replace-tabs on

/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Jesse Yurkovich <yurkjes@iit.edu>

    KateVarIndent class:
    SPDX-FileCopyrightText: 2004 Anders Lund <anders@alweb.dk>

    Basic support for config page:
    SPDX-FileCopyrightText: 2005 Dominik Haumann <dhdev@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kateautoindent.h"

#include "attribute.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katehighlight.h"
#include "kateindentscript.h"
#include "katepartdebug.h"
#include "katescriptmanager.h"

#include <KLocalizedString>

#include <QActionGroup>
#include <QMenu>

namespace
{
inline const QString MODE_NONE()
{
    return QStringLiteral("none");
}
inline const QString MODE_NORMAL()
{
    return QStringLiteral("normal");
}
}

// BEGIN KateAutoIndent

QStringList KateAutoIndent::listModes()
{
    QStringList l;
    l.reserve(modeCount());
    for (int i = 0; i < modeCount(); ++i) {
        l << modeDescription(i);
    }

    return l;
}

QStringList KateAutoIndent::listIdentifiers()
{
    QStringList l;
    l.reserve(modeCount());
    for (int i = 0; i < modeCount(); ++i) {
        l << modeName(i);
    }

    return l;
}

int KateAutoIndent::modeCount()
{
    // inbuild modes + scripts
    return 2 + KTextEditor::EditorPrivate::self()->scriptManager()->indentationScriptCount();
}

QString KateAutoIndent::modeName(int mode)
{
    if (mode == 0 || mode >= modeCount()) {
        return MODE_NONE();
    }

    if (mode == 1) {
        return MODE_NORMAL();
    }

    return KTextEditor::EditorPrivate::self()->scriptManager()->indentationScriptByIndex(mode - 2)->indentHeader().baseName();
}

QString KateAutoIndent::modeDescription(int mode)
{
    if (mode == 0 || mode >= modeCount()) {
        return i18nc("Autoindent mode", "None");
    }

    if (mode == 1) {
        return i18nc("Autoindent mode", "Normal");
    }

    const QString &name = KTextEditor::EditorPrivate::self()->scriptManager()->indentationScriptByIndex(mode - 2)->indentHeader().name();
    return i18nc("Autoindent mode", name.toUtf8().constData());
}

QString KateAutoIndent::modeRequiredStyle(int mode)
{
    if (mode == 0 || mode == 1 || mode >= modeCount()) {
        return QString();
    }

    return KTextEditor::EditorPrivate::self()->scriptManager()->indentationScriptByIndex(mode - 2)->indentHeader().requiredStyle();
}

uint KateAutoIndent::modeNumber(const QString &name)
{
    for (int i = 0; i < modeCount(); ++i) {
        if (modeName(i) == name) {
            return i;
        }
    }

    return 0;
}

KateAutoIndent::KateAutoIndent(KTextEditor::DocumentPrivate *_doc)
    : QObject(_doc)
    , doc(_doc)
    , m_script(nullptr)
{
    // don't call updateConfig() here, document might is not ready for that....

    // on script reload, the script pointer is invalid -> force reload
    connect(KTextEditor::EditorPrivate::self()->scriptManager(), &KateScriptManager::reloaded, this, &KateAutoIndent::reloadScript);
}

KateAutoIndent::~KateAutoIndent() = default;

QString KateAutoIndent::tabString(int length, int align) const
{
    QString s;
    length = qMin(length, 256); // sanity check for large values of pos
    int spaces = qBound(0, align - length, 256);

    if (!useSpaces) {
        s.append(QString(length / tabWidth, QLatin1Char('\t')));
        length = length % tabWidth;
    }
    // we use spaces to indent any left over length
    s.append(QString(length + spaces, QLatin1Char(' ')));

    return s;
}

bool KateAutoIndent::doIndent(int line, int indentDepth, int align)
{
    Kate::TextLine textline = doc->plainKateTextLine(line);

    // sanity check
    if (indentDepth < 0) {
        indentDepth = 0;
    }

    const QString oldIndentation = textline.leadingWhitespace();

    // Preserve existing "tabs then spaces" alignment if and only if:
    //  - no alignment was passed to doIndent and
    //  - we aren't using spaces for indentation and
    //  - we aren't rounding indentation up to the next multiple of the indentation width and
    //  - we aren't using a combination to tabs and spaces for alignment, or in other words
    //    the indent width is a multiple of the tab width.
    bool preserveAlignment = !useSpaces && keepExtra && indentWidth % tabWidth == 0;
    if (align == 0 && preserveAlignment) {
        // Count the number of consecutive spaces at the end of the existing indentation
        int i = oldIndentation.size() - 1;
        while (i >= 0 && oldIndentation.at(i) == QLatin1Char(' ')) {
            --i;
        }
        // Use the passed indentDepth as the alignment, and set the indentDepth to
        // that value minus the number of spaces found (but don't let it get negative).
        align = indentDepth;
        indentDepth = qMax(0, align - (oldIndentation.size() - 1 - i));
    }

    QString indentString = tabString(indentDepth, align);

    // Modify the document *ONLY* if smth has really changed!
    if (oldIndentation != indentString) {
        // insert the required new indentation
        // before removing the old indentation
        // to prevent the selection to be shrink by the first removal
        // (see bug329247)
        doc->editStart();
        doc->editInsertText(line, 0, indentString);
        doc->editRemoveText(line, indentString.length(), oldIndentation.length());
        doc->editEnd();
    }

    return true;
}

bool KateAutoIndent::doIndentRelative(int line, int change)
{
    Kate::TextLine textline = doc->plainKateTextLine(line);

    // get indent width of current line
    int indentDepth = textline.indentDepth(tabWidth);
    int extraSpaces = indentDepth % indentWidth;

    // add change
    indentDepth += change;

    // if keepExtra is off, snap to a multiple of the indentWidth
    if (!keepExtra && extraSpaces > 0) {
        if (change < 0) {
            indentDepth += indentWidth - extraSpaces;
        } else {
            indentDepth -= extraSpaces;
        }
    }

    // do indent
    return doIndent(line, indentDepth);
}

void KateAutoIndent::keepIndent(int line)
{
    // no line in front, no work...
    if (line <= 0) {
        return;
    }

    // keep indentation: find line with content
    int nonEmptyLine = line - 1;
    while (nonEmptyLine >= 0) {
        if (doc->lineLength(nonEmptyLine) > 0) {
            break;
        }
        --nonEmptyLine;
    }

    // no line in front, no work...
    if (nonEmptyLine < 0) {
        return;
    }

    Kate::TextLine prevTextLine = doc->plainKateTextLine(nonEmptyLine);
    Kate::TextLine textLine = doc->plainKateTextLine(line);

    const QString previousWhitespace = prevTextLine.leadingWhitespace();

    // remove leading whitespace, then insert the leading indentation
    doc->editStart();

    int indentDepth = textLine.indentDepth(tabWidth);
    int extraSpaces = indentDepth % indentWidth;
    doc->editRemoveText(line, 0, textLine.leadingWhitespace().size());
    if (keepExtra && extraSpaces > 0)
        doc->editInsertText(line, 0, QString(extraSpaces, QLatin1Char(' ')));

    doc->editInsertText(line, 0, previousWhitespace);
    doc->editEnd();
}

void KateAutoIndent::reloadScript()
{
    // small trick to force reload
    m_script = nullptr; // prevent dangling pointer
    QString currentMode = m_mode;
    m_mode = QString();
    setMode(currentMode);
}

void KateAutoIndent::scriptIndent(KTextEditor::ViewPrivate *view, const KTextEditor::Cursor position, QChar typedChar)
{
    // start edit
    doc->pushEditState();
    doc->editStart();

    QPair<int, int> result = m_script->indent(view, position, typedChar, indentWidth);
    int newIndentInChars = result.first;

    // handle negative values special
    if (newIndentInChars < -1) {
        // do nothing atm
    }

    // reuse indentation of the previous line, just like the "normal" indenter
    else if (newIndentInChars == -1) {
        // keep indent of previous line
        keepIndent(position.line());
    }

    // get align
    else {
        // we got a positive or zero indent to use...
        doIndent(position.line(), newIndentInChars, result.second);
    }

    // end edit in all cases
    doc->editEnd();
    doc->popEditState();
}

bool KateAutoIndent::isStyleProvided(const KateIndentScript *script, const KateHighlighting *highlight)
{
    QString requiredStyle = script->indentHeader().requiredStyle();
    return (requiredStyle.isEmpty() || requiredStyle == highlight->style());
}

void KateAutoIndent::setMode(const QString &name)
{
    // bail out, already set correct mode...
    if (m_mode == name) {
        return;
    }

    // cleanup
    m_script = nullptr;

    // first, catch easy stuff... normal mode and none, easy...
    if (name.isEmpty() || name == MODE_NONE()) {
        m_mode = MODE_NONE();
        return;
    }

    if (name == MODE_NORMAL()) {
        m_mode = MODE_NORMAL();
        return;
    }

    // handle script indenters, if any for this name...
    KateIndentScript *script = KTextEditor::EditorPrivate::self()->scriptManager()->indentationScript(name);
    if (script) {
        if (isStyleProvided(script, doc->highlight())) {
            m_script = script;
            m_mode = name;
            return;
        } else {
            qCWarning(LOG_KTE) << "mode" << name << "requires a different highlight style: highlighting" << doc->highlight()->name() << "with style"
                               << doc->highlight()->style() << "but script requires" << script->indentHeader().requiredStyle();
        }
    } else {
        qCWarning(LOG_KTE) << "mode" << name << "does not exist";
    }

    // Fall back to normal
    m_mode = MODE_NORMAL();
}

void KateAutoIndent::checkRequiredStyle()
{
    if (m_script) {
        if (!isStyleProvided(m_script, doc->highlight())) {
            qCDebug(LOG_KTE) << "mode" << m_mode << "requires a different highlight style: highlighting" << doc->highlight()->name() << "with style"
                             << doc->highlight()->style() << "but script requires" << m_script->indentHeader().requiredStyle();
            doc->config()->setIndentationMode(MODE_NORMAL());
        }
    }
}

void KateAutoIndent::updateConfig()
{
    KateDocumentConfig *config = doc->config();

    useSpaces = config->replaceTabsDyn();
    keepExtra = config->keepExtraSpaces();
    tabWidth = config->tabWidth();
    indentWidth = config->indentationWidth();
}

bool KateAutoIndent::changeIndent(KTextEditor::Range range, int change)
{
    std::vector<int> skippedLines;

    // loop over all lines given...
    for (int line = range.start().line() < 0 ? 0 : range.start().line(); line <= qMin(range.end().line(), doc->lines() - 1); ++line) {
        // don't indent empty lines
        if (doc->line(line).isEmpty()) {
            skippedLines.push_back(line);
            continue;
        }
        // don't indent the last line when the cursor is on the first column
        if (line == range.end().line() && range.end().column() == 0) {
            skippedLines.push_back(line);
            continue;
        }

        doIndentRelative(line, change * indentWidth);
    }

    if (static_cast<int>(skippedLines.size()) > range.numberOfLines()) {
        // all lines were empty, so indent them nevertheless
        for (int line : skippedLines) {
            doIndentRelative(line, change * indentWidth);
        }
    }

    return true;
}

void KateAutoIndent::indent(KTextEditor::ViewPrivate *view, KTextEditor::Range range)
{
    // no script, do nothing...
    if (!m_script) {
        return;
    }

    // we want one undo action >= START
    doc->setUndoMergeAllEdits(true);

    bool prevKeepExtra = keepExtra;
    keepExtra = false; // we are formatting a block of code, no extra spaces
    // loop over all lines given...
    for (int line = range.start().line() < 0 ? 0 : range.start().line(); line <= qMin(range.end().line(), doc->lines() - 1); ++line) {
        // let the script indent for us...
        scriptIndent(view, KTextEditor::Cursor(line, 0), QChar());
    }

    keepExtra = prevKeepExtra;
    // we want one undo action => END
    doc->setUndoMergeAllEdits(false);
}

void KateAutoIndent::userTypedChar(KTextEditor::ViewPrivate *view, const KTextEditor::Cursor position, QChar typedChar)
{
    // normal mode
    if (m_mode == MODE_NORMAL()) {
        // only indent on new line, per default
        if (typedChar != QLatin1Char('\n')) {
            return;
        }

        // keep indent of previous line
        keepIndent(position.line());
        return;
    }

    // no script, do nothing...
    if (!m_script) {
        return;
    }

    // does the script allow this char as trigger?
    if (typedChar != QLatin1Char('\n') && !m_script->triggerCharacters().contains(typedChar)) {
        return;
    }

    // let the script indent for us...
    scriptIndent(view, position, typedChar);
}
// END KateAutoIndent

// BEGIN KateViewIndentAction
KateViewIndentationAction::KateViewIndentationAction(KTextEditor::DocumentPrivate *_doc, const QString &text, QObject *parent)
    : KActionMenu(text, parent)
    , doc(_doc)
{
    setPopupMode(QToolButton::InstantPopup);
    connect(menu(), &QMenu::aboutToShow, this, &KateViewIndentationAction::slotAboutToShow);
    actionGroup = new QActionGroup(menu());
}

void KateViewIndentationAction::slotAboutToShow()
{
    const QStringList modes = KateAutoIndent::listModes();

    menu()->clear();
    const auto actions = actionGroup->actions();
    for (QAction *action : actions) {
        actionGroup->removeAction(action);
    }
    for (int z = 0; z < modes.size(); ++z) {
        QAction *action = menu()->addAction(QLatin1Char('&') + KateAutoIndent::modeDescription(z).replace(QLatin1Char('&'), QLatin1String("&&")));
        actionGroup->addAction(action);
        action->setCheckable(true);
        action->setData(z);

        QString requiredStyle = KateAutoIndent::modeRequiredStyle(z);
        action->setEnabled(requiredStyle.isEmpty() || requiredStyle == doc->highlight()->style());

        if (doc->config()->indentationMode() == KateAutoIndent::modeName(z)) {
            action->setChecked(true);
        }
    }

    disconnect(menu(), &QMenu::triggered, this, &KateViewIndentationAction::setMode);
    connect(menu(), &QMenu::triggered, this, &KateViewIndentationAction::setMode);
}

void KateViewIndentationAction::setMode(QAction *action)
{
    // set new mode
    doc->config()->setIndentationMode(KateAutoIndent::modeName(action->data().toInt()));
    doc->rememberUserDidSetIndentationMode();
}
// END KateViewIndentationAction

#include "moc_kateautoindent.cpp"

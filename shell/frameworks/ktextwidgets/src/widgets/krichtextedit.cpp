/*
    krichtextedit
    SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
    SPDX-FileCopyrightText: 2008 Thomas McGuire <thomas.mcguire@gmx.net>
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "krichtextedit.h"
#include "krichtextedit_p.h"

// Own includes
#include "klinkdialog_p.h"

// kdelibs includes
#include <KColorScheme>
#include <KCursor>

// Qt includes
#include <QRegularExpression>

void KRichTextEditPrivate::activateRichText()
{
    Q_Q(KRichTextEdit);

    if (mMode == KRichTextEdit::Plain) {
        q->setAcceptRichText(true);
        mMode = KRichTextEdit::Rich;
        Q_EMIT q->textModeChanged(mMode);
    }
}

void KRichTextEditPrivate::setTextCursor(QTextCursor &cursor)
{
    Q_Q(KRichTextEdit);

    q->setTextCursor(cursor);
}

void KRichTextEditPrivate::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    Q_Q(KRichTextEdit);

    QTextCursor cursor = q->textCursor();
    QTextCursor wordStart(cursor);
    QTextCursor wordEnd(cursor);

    wordStart.movePosition(QTextCursor::StartOfWord);
    wordEnd.movePosition(QTextCursor::EndOfWord);

    cursor.beginEditBlock();
    if (!cursor.hasSelection() && cursor.position() != wordStart.position() && cursor.position() != wordEnd.position()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    q->mergeCurrentCharFormat(format);
    cursor.endEditBlock();
}

KRichTextEdit::KRichTextEdit(const QString &text, QWidget *parent)
    : KRichTextEdit(*new KRichTextEditPrivate(this), text, parent)
{
}

KRichTextEdit::KRichTextEdit(KRichTextEditPrivate &dd, const QString &text, QWidget *parent)
    : KTextEdit(dd, text, parent)
{
    Q_D(KRichTextEdit);

    d->init();
}

KRichTextEdit::KRichTextEdit(QWidget *parent)
    : KRichTextEdit(*new KRichTextEditPrivate(this), parent)
{
}

KRichTextEdit::KRichTextEdit(KRichTextEditPrivate &dd, QWidget *parent)
    : KTextEdit(dd, parent)
{
    Q_D(KRichTextEdit);

    d->init();
}

KRichTextEdit::~KRichTextEdit() = default;

//@cond PRIVATE
void KRichTextEditPrivate::init()
{
    Q_Q(KRichTextEdit);

    q->setAcceptRichText(false);
    KCursor::setAutoHideCursor(q, true, true);
}
//@endcond

void KRichTextEdit::setListStyle(int _styleIndex)
{
    Q_D(KRichTextEdit);

    d->nestedListHelper->handleOnBulletType(-_styleIndex);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::indentListMore()
{
    Q_D(KRichTextEdit);

    d->nestedListHelper->changeIndent(+1);
    d->activateRichText();
}

void KRichTextEdit::indentListLess()
{
    Q_D(KRichTextEdit);

    d->nestedListHelper->changeIndent(-1);
}

void KRichTextEdit::insertHorizontalRule()
{
    Q_D(KRichTextEdit);

    QTextCursor cursor = textCursor();
    QTextBlockFormat bf = cursor.blockFormat();
    QTextCharFormat cf = cursor.charFormat();

    cursor.beginEditBlock();
    cursor.insertHtml(QStringLiteral("<hr>"));
    cursor.insertBlock(bf, cf);
    cursor.endEditBlock();
    setTextCursor(cursor);
    d->activateRichText();
}

void KRichTextEdit::alignLeft()
{
    Q_D(KRichTextEdit);

    setAlignment(Qt::AlignLeft);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::alignCenter()
{
    Q_D(KRichTextEdit);

    setAlignment(Qt::AlignHCenter);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::alignRight()
{
    Q_D(KRichTextEdit);

    setAlignment(Qt::AlignRight);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::alignJustify()
{
    Q_D(KRichTextEdit);

    setAlignment(Qt::AlignJustify);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::makeRightToLeft()
{
    Q_D(KRichTextEdit);

    QTextBlockFormat format;
    format.setLayoutDirection(Qt::RightToLeft);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::makeLeftToRight()
{
    Q_D(KRichTextEdit);

    QTextBlockFormat format;
    format.setLayoutDirection(Qt::LeftToRight);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextBold(bool bold)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextItalic(bool italic)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFontItalic(italic);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextUnderline(bool underline)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFontUnderline(underline);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextStrikeOut(bool strikeOut)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFontStrikeOut(strikeOut);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextForegroundColor(const QColor &color)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setForeground(color);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextBackgroundColor(const QColor &color)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setBackground(color);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setFontFamily(const QString &fontFamily)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFontFamilies({fontFamily});
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setFontSize(int size)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setFont(const QFont &font)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setFont(font);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::switchToPlainText()
{
    Q_D(KRichTextEdit);

    if (d->mMode == Rich) {
        d->mMode = Plain;
        // TODO: Warn the user about this?
        auto insertPlainFunc = [this]() {
            insertPlainTextImplementation();
        };
        QMetaObject::invokeMethod(this, insertPlainFunc);
        setAcceptRichText(false);
        Q_EMIT textModeChanged(d->mMode);
    }
}

void KRichTextEdit::insertPlainTextImplementation()
{
    document()->setPlainText(document()->toPlainText());
}

void KRichTextEdit::setTextSuperScript(bool superscript)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setVerticalAlignment(superscript ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setTextSubScript(bool subscript)
{
    Q_D(KRichTextEdit);

    QTextCharFormat fmt;
    fmt.setVerticalAlignment(subscript ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
    d->mergeFormatOnWordOrSelection(fmt);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::setHeadingLevel(int level)
{
    Q_D(KRichTextEdit);

    const int boundedLevel = qBound(0, 6, level);
    // Apparently, 5 is maximum for FontSizeAdjustment; otherwise level=1 and
    // level=2 look the same
    const int sizeAdjustment = boundedLevel > 0 ? 5 - boundedLevel : 0;

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blkfmt;
    blkfmt.setHeadingLevel(boundedLevel);
    cursor.mergeBlockFormat(blkfmt);

    QTextCharFormat chrfmt;
    chrfmt.setFontWeight(boundedLevel > 0 ? QFont::Bold : QFont::Normal);
    chrfmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
    // Applying style to the current line or selection
    QTextCursor selectCursor = cursor;
    if (selectCursor.hasSelection()) {
        QTextCursor top = selectCursor;
        top.setPosition(qMin(top.anchor(), top.position()));
        top.movePosition(QTextCursor::StartOfBlock);

        QTextCursor bottom = selectCursor;
        bottom.setPosition(qMax(bottom.anchor(), bottom.position()));
        bottom.movePosition(QTextCursor::EndOfBlock);

        selectCursor.setPosition(top.position(), QTextCursor::MoveAnchor);
        selectCursor.setPosition(bottom.position(), QTextCursor::KeepAnchor);
    } else {
        selectCursor.select(QTextCursor::BlockUnderCursor);
    }
    selectCursor.mergeCharFormat(chrfmt);

    cursor.mergeBlockCharFormat(chrfmt);
    cursor.endEditBlock();
    setTextCursor(cursor);
    setFocus();
    d->activateRichText();
}

void KRichTextEdit::enableRichTextMode()
{
    Q_D(KRichTextEdit);

    d->activateRichText();
}

KRichTextEdit::Mode KRichTextEdit::textMode() const
{
    Q_D(const KRichTextEdit);

    return d->mMode;
}

QString KRichTextEdit::textOrHtml() const
{
    if (textMode() == Rich) {
        return toCleanHtml();
    } else {
        return toPlainText();
    }
}

void KRichTextEdit::setTextOrHtml(const QString &text)
{
    Q_D(KRichTextEdit);

    // might be rich text
    if (Qt::mightBeRichText(text)) {
        if (d->mMode == KRichTextEdit::Plain) {
            d->activateRichText();
        }
        setHtml(text);
    } else {
        setPlainText(text);
    }
}

// KF6 TODO: remove constness
QString KRichTextEdit::currentLinkText() const
{
    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    return cursor.selectedText();
}

// KF6 TODO: remove constness
void KRichTextEdit::selectLinkText() const
{
    Q_D(const KRichTextEdit);

    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    // KF6 TODO: remove const_cast
    const_cast<KRichTextEditPrivate *>(d)->setTextCursor(cursor);
}

void KRichTextEdit::selectLinkText(QTextCursor *cursor) const
{
    // If the cursor is on a link, select the text of the link.
    if (cursor->charFormat().isAnchor()) {
        QString aHref = cursor->charFormat().anchorHref();

        // Move cursor to start of link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atStart()) {
                break;
            }
            cursor->setPosition(cursor->position() - 1);
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }

        // Move selection to the end of the link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atEnd()) {
                break;
            }
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() - 1, QTextCursor::KeepAnchor);
        }
    } else if (cursor->hasSelection()) {
        // Nothing to to. Using the currently selected text as the link text.
    } else {
        // Select current word
        cursor->movePosition(QTextCursor::StartOfWord);
        cursor->movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
}

QString KRichTextEdit::currentLinkUrl() const
{
    return textCursor().charFormat().anchorHref();
}

void KRichTextEdit::updateLink(const QString &linkUrl, const QString &linkText)
{
    Q_D(KRichTextEdit);

    selectLinkText();

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format = cursor.charFormat();
    // Save original format to create an extra space with the existing char
    // format for the block
    const QTextCharFormat originalFormat = format;
    if (!linkUrl.isEmpty()) {
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        format.setForeground(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        d->activateRichText();
    } else {
        // Remove link details
        format.setAnchor(false);
        format.setAnchorHref(QString());
        // Workaround for QTBUG-1814:
        // Link formatting does not get removed immediately when setAnchor(false)
        // is called. So the formatting needs to be applied manually.
        QTextDocument defaultTextDocument;
        QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();

        format.setUnderlineStyle(defaultCharFormat.underlineStyle());
        format.setUnderlineColor(defaultCharFormat.underlineColor());
        format.setForeground(defaultCharFormat.foreground());
    }

    // Insert link text specified in dialog, otherwise write out url.
    QString _linkText;
    if (!linkText.isEmpty()) {
        _linkText = linkText;
    } else {
        _linkText = linkUrl;
    }
    cursor.insertText(_linkText, format);

    // Insert a space after the link if at the end of the block so that
    // typing some text after the link does not carry link formatting
    if (!linkUrl.isEmpty() && cursor.atBlockEnd()) {
        cursor.setPosition(cursor.selectionEnd());
        cursor.setCharFormat(originalFormat);
        cursor.insertText(QStringLiteral(" "));
    }

    cursor.endEditBlock();
}

void KRichTextEdit::keyPressEvent(QKeyEvent *event)
{
    Q_D(KRichTextEdit);

    bool handled = false;
    if (textCursor().currentList()) {
        handled = d->nestedListHelper->handleKeyPressEvent(event);
    }

    // If a line was merged with previous (next) one, with different heading level,
    // the style should also be adjusted accordingly (i.e. merged)
    if ((event->key() == Qt::Key_Backspace && textCursor().atBlockStart()
         && (textCursor().blockFormat().headingLevel() != textCursor().block().previous().blockFormat().headingLevel()))
        || (event->key() == Qt::Key_Delete && textCursor().atBlockEnd()
            && (textCursor().blockFormat().headingLevel() != textCursor().block().next().blockFormat().headingLevel()))) {
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        if (event->key() == Qt::Key_Delete) {
            cursor.deleteChar();
        } else {
            cursor.deletePreviousChar();
        }
        setHeadingLevel(cursor.blockFormat().headingLevel());
        cursor.endEditBlock();
        handled = true;
    }

    const auto prevHeadingLevel = textCursor().blockFormat().headingLevel();
    if (!handled) {
        KTextEdit::keyPressEvent(event);
    }

    // Match the behavior of office suites: newline after header switches to normal text
    if (event->key() == Qt::Key_Return //
        && prevHeadingLevel > 0) {
        // it should be undoable together with actual "return" keypress
        textCursor().joinPreviousEditBlock();
        if (textCursor().atBlockEnd()) {
            setHeadingLevel(0);
        } else {
            setHeadingLevel(prevHeadingLevel);
        }
        textCursor().endEditBlock();
    }

    Q_EMIT cursorPositionChanged();
}

// void KRichTextEdit::dropEvent(QDropEvent *event)
// {
//     int dropSize = event->mimeData()->text().size();
//
//     dropEvent( event );
//     QTextCursor cursor = textCursor();
//     int cursorPosition = cursor.position();
//     cursor.setPosition( cursorPosition - dropSize );
//     cursor.setPosition( cursorPosition, QTextCursor::KeepAnchor );
//     setTextCursor( cursor );
//     d->nestedListHelper->handleAfterDropEvent( event );
// }

bool KRichTextEdit::canIndentList() const
{
    Q_D(const KRichTextEdit);

    return d->nestedListHelper->canIndent();
}

bool KRichTextEdit::canDedentList() const
{
    Q_D(const KRichTextEdit);

    return d->nestedListHelper->canDedent();
}

QString KRichTextEdit::toCleanHtml() const
{
    QString result = toHtml();

    static const QString EMPTYLINEHTML = QLatin1String(
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
        "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; \">&nbsp;</p>");

    // Qt inserts various style properties based on the current mode of the editor (underline,
    // bold, etc), but only empty paragraphs *also* have qt-paragraph-type set to 'empty'.
    static const QString EMPTYLINEREGEX = QStringLiteral("<p style=\"-qt-paragraph-type:empty;(.*?)</p>");

    static const QString OLLISTPATTERNQT = QStringLiteral("<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ULLISTPATTERNQT = QStringLiteral("<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ORDEREDLISTHTML = QStringLiteral("<ol style=\"margin-top: 0px; margin-bottom: 0px;");

    static const QString UNORDEREDLISTHTML = QStringLiteral("<ul style=\"margin-top: 0px; margin-bottom: 0px;");

    // fix 1 - empty lines should show as empty lines - MS Outlook treats margin-top:0px; as
    // a non-existing line.
    // Although we can simply remove the margin-top style property, we still get unwanted results
    // if you have three or more empty lines. It's best to replace empty <p> elements with <p>&nbsp;</p>.
    // replace all occurrences with the new line text
    result.replace(QRegularExpression(EMPTYLINEREGEX), EMPTYLINEHTML);

    // fix 2a - ordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing number; e.g: "1. First item" turns into "First Item"
    result.replace(OLLISTPATTERNQT, ORDEREDLISTHTML);

    // fix 2b - unordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing bullet; e.g: "* First bullet" turns into "First Bullet"
    result.replace(ULLISTPATTERNQT, UNORDEREDLISTHTML);

    return result;
}

#include "moc_krichtextedit.cpp"

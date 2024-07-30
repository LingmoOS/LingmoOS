/*
    This file is part of the Qt Quick Controls module of the Qt Toolkit.
    SPDX-FileCopyrightText: 2013 Digia Plc and /or its subsidiary(-ies) <http://www.qt-project.org/legal>

    SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This file was imported from Qt into Plasma
 * I expect this to become public API at some point
 * at which point we should delete this
 */

#include "documenthandler.h"

#include <QClipboard>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QMimeData>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include <KTextToHTML>

DocumentHandler::DocumentHandler()
    : m_target(nullptr)
    , m_doc(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
{
}

void DocumentHandler::setTarget(QQuickItem *target)
{
    m_doc = nullptr;
    m_target = target;
    if (!m_target) {
        return;
    }

    QVariant doc = m_target->property("textDocument");
    if (doc.canConvert<QQuickTextDocument *>()) {
        QQuickTextDocument *qqdoc = doc.value<QQuickTextDocument *>();
        if (qqdoc) {
            m_doc = qqdoc->textDocument();
        }
    }
    Q_EMIT targetChanged();
}

QString DocumentHandler::documentTitle() const
{
    return m_documentTitle;
}

void DocumentHandler::setDocumentTitle(QString arg)
{
    if (m_documentTitle != arg) {
        m_documentTitle = arg;
        Q_EMIT documentTitleChanged();
    }
}

void DocumentHandler::pasteWithoutFormatting()
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }

    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        return;
    }

    const QMimeData *mimeData = clipboard->mimeData();
    if (!mimeData) {
        return;
    }

    QString content = KTextToHTML::convertToHtml(mimeData->text(), KTextToHTML::Options(KTextToHTML::PreserveSpaces));

    if (content.endsWith("</a>", Qt::CaseInsensitive)) {
        content += " ";
    }

    cursor.insertHtml(content);
}

void DocumentHandler::setText(const QString &arg)
{
    if (m_text != arg) {
        m_text = arg;
        Q_EMIT textChanged();
    }
}

QString DocumentHandler::text() const
{
    return m_text;
}

void DocumentHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition) {
        return;
    }

    m_cursorPosition = position;

    reset();
}

void DocumentHandler::reset()
{
    Q_EMIT fontFamilyChanged();
    Q_EMIT alignmentChanged();
    Q_EMIT boldChanged();
    Q_EMIT italicChanged();
    Q_EMIT underlineChanged();
    Q_EMIT strikeOutChanged();
    Q_EMIT fontSizeChanged();
    Q_EMIT textColorChanged();
}

QTextCursor DocumentHandler::textCursor() const
{
    if (!m_doc) {
        return QTextCursor();
    }
    QTextCursor cursor = QTextCursor(m_doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

void DocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
}

void DocumentHandler::setSelectionStart(int position)
{
    m_selectionStart = position;
}

void DocumentHandler::setSelectionEnd(int position)
{
    m_selectionEnd = position;
}

void DocumentHandler::setAlignment(Qt::Alignment a)
{
    QTextBlockFormat fmt;
    fmt.setAlignment((Qt::Alignment)a);
    QTextCursor cursor = QTextCursor(m_doc);
    cursor.setPosition(m_selectionStart, QTextCursor::MoveAnchor);
    cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    cursor.mergeBlockFormat(fmt);
    Q_EMIT alignmentChanged();
}

Qt::Alignment DocumentHandler::alignment() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return Qt::AlignLeft;
    }
    return textCursor().blockFormat().alignment();
}

bool DocumentHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

bool DocumentHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return textCursor().charFormat().fontItalic();
}

bool DocumentHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return textCursor().charFormat().fontUnderline();
}

bool DocumentHandler::strikeOut() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return textCursor().charFormat().fontStrikeOut();
}

void DocumentHandler::setBold(bool arg)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(arg ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
    Q_EMIT boldChanged();
}

void DocumentHandler::setItalic(bool arg)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(arg);
    mergeFormatOnWordOrSelection(fmt);
    Q_EMIT italicChanged();
}

void DocumentHandler::setUnderline(bool arg)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(arg);
    mergeFormatOnWordOrSelection(fmt);
    Q_EMIT underlineChanged();
}

void DocumentHandler::setStrikeOut(bool arg)
{
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(arg);
    mergeFormatOnWordOrSelection(fmt);
    Q_EMIT strikeOutChanged();
}

int DocumentHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return 0;
    }
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

void DocumentHandler::setFontSize(int arg)
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    QTextCharFormat format;
    format.setFontPointSize(arg);
    mergeFormatOnWordOrSelection(format);
    // Also set the block char format, otherwise the old block font will stay and
    // end up taking effect if the user deletes all the text.
    cursor.mergeBlockCharFormat(format);

    Q_EMIT fontSizeChanged();
}

int DocumentHandler::defaultFontSize() const
{
    return m_doc ? m_doc->defaultFont().pointSize() : 0;
}

void DocumentHandler::setDefaultFontSize(int arg)
{
    if (!m_doc) {
        return;
    }

    auto font = m_doc->defaultFont();
    font.setPointSize(arg);
    m_doc->setDefaultFont(font);
    Q_EMIT defaultFontSizeChanged();
}

QColor DocumentHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return QColor(Qt::black);
    }
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

void DocumentHandler::setTextColor(const QColor &c)
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    QTextCharFormat format;
    format.setForeground(QBrush(c));
    mergeFormatOnWordOrSelection(format);
    Q_EMIT textColorChanged();
}

QString DocumentHandler::fontFamily() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return QString();
    }
    QTextCharFormat format = cursor.charFormat();
    return format.font().family();
}

void DocumentHandler::setFontFamily(const QString &arg)
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    QTextCharFormat format;
    format.setFontFamilies({arg});
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontFamilyChanged();
}

QStringList DocumentHandler::defaultFontSizes() const
{
    // uhm... this is quite ugly
    QStringList sizes;
    const QList<int> standardSizes = QFontDatabase::standardSizes();
    sizes.reserve(standardSizes.count());
    for (int size : standardSizes) {
        sizes.append(QString::number(size));
    }
    return sizes;
}

QString DocumentHandler::strip(const QString text)
{
    return QTextDocumentFragment::fromHtml(text).toPlainText();
}

QString DocumentHandler::stripAndSimplify(const QString text)
{
    QString myText = text;
    QString plainText = QTextDocumentFragment::fromHtml(myText).toPlainText();
    // Normalize all whitespace to increase the fuzziness of the match
    plainText = plainText.simplified();
    return plainText;
}

QString DocumentHandler::strippedClipboardText()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        return QString();
    }

    return stripAndSimplify(clipboard->text());
}

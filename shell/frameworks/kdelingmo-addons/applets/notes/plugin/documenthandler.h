/*
    This file is part of the Qt Quick Controls module of the Qt Toolkit.
    SPDX-FileCopyrightText: 2013 Digia Plc and /or its subsidiary(-ies) <http://www.qt-project.org/legal>

    SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H

#include <QQuickTextDocument>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class DocumentHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY underlineChanged)
    Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut NOTIFY strikeOutChanged)

    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(int defaultFontSize READ defaultFontSize WRITE setDefaultFontSize NOTIFY defaultFontSizeChanged)

    Q_PROPERTY(QStringList defaultFontSizes READ defaultFontSizes NOTIFY defaultFontSizesChanged)

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString documentTitle READ documentTitle WRITE setDocumentTitle NOTIFY documentTitleChanged)

public:
    DocumentHandler();

    QQuickItem *target()
    {
        return m_target;
    }

    void setTarget(QQuickItem *target);

    void setCursorPosition(int position);
    void setSelectionStart(int position);
    void setSelectionEnd(int position);

    int cursorPosition() const
    {
        return m_cursorPosition;
    }

    int selectionStart() const
    {
        return m_selectionStart;
    }

    int selectionEnd() const
    {
        return m_selectionEnd;
    }

    QString fontFamily() const;

    QColor textColor() const;

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment a);

    bool bold() const;
    bool italic() const;
    bool underline() const;
    bool strikeOut() const;
    int fontSize() const;
    int defaultFontSize() const;

    QStringList defaultFontSizes() const;
    QString text() const;

    QString documentTitle() const;

    Q_INVOKABLE QString strip(const QString text);
    Q_INVOKABLE QString stripAndSimplify(const QString text);
    Q_INVOKABLE QString strippedClipboardText();

public Q_SLOTS:
    void setBold(bool arg);
    void setItalic(bool arg);
    void setUnderline(bool arg);
    void setStrikeOut(bool arg);
    void setFontSize(int arg);
    void setDefaultFontSize(int arg);
    void setTextColor(const QColor &arg);
    void setFontFamily(const QString &arg);

    void setText(const QString &arg);

    void setDocumentTitle(QString arg);

    void pasteWithoutFormatting();
    void reset();

Q_SIGNALS:
    void targetChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();

    void fontFamilyChanged();
    void textColorChanged();
    void alignmentChanged();

    void boldChanged();
    void italicChanged();
    void underlineChanged();
    void strikeOutChanged();

    void fontSizeChanged();
    void defaultFontSizeChanged();
    void defaultFontSizesChanged();

    void fileUrlChanged();

    void textChanged();
    void documentTitleChanged();

private:
    QTextCursor textCursor() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    QQuickItem *m_target;
    QTextDocument *m_doc;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    QFont m_font;
    int m_fontSize;
    QString m_text;
    QString m_documentTitle;
};

#endif

// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "QWaylandInputMethodEventBuilder.h"

#include "wayland-text-input-unstable-v3-client-protocol.h"

#include <QBrush>
#include <QGuiApplication>
#include <QInputMethod>
#include <QPalette>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE

QWaylandInputMethodEventBuilder::~QWaylandInputMethodEventBuilder() { }

void QWaylandInputMethodEventBuilder::reset()
{
    m_anchor = 0;
    m_cursor = 0;
    m_deleteBefore = 0;
    m_deleteAfter = 0;
    m_preeditCursor = 0;
    m_preeditStyles.clear();
}

void QWaylandInputMethodEventBuilder::setCursorPosition(int32_t index, int32_t anchor)
{
    m_cursor = index;
    m_anchor = anchor;
}

void QWaylandInputMethodEventBuilder::setDeleteSurroundingText(uint32_t beforeLength,
                                                               uint32_t afterLength)
{
    m_deleteBefore = beforeLength;
    m_deleteAfter = afterLength;
}

void QWaylandInputMethodEventBuilder::setPreeditCursor(int32_t index)
{
    m_preeditCursor = index;
}

QInputMethodEvent *QWaylandInputMethodEventBuilder::buildCommit(const QString &text)
{
    QList<QInputMethodEvent::Attribute> attributes;

    const QPair<int, int> replacement = replacementForDeleteSurrounding();

    if (m_cursor != 0 || m_anchor != 0) {
        QString surrounding =
            QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant()).toString();
        const int cursor = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant()).toInt();
        const int anchor = QInputMethod::queryFocusObject(Qt::ImAnchorPosition, QVariant()).toInt();
        const int absoluteCursor =
            QInputMethod::queryFocusObject(Qt::ImAbsolutePosition, QVariant()).toInt();

        const int absoluteOffset = absoluteCursor - cursor;

        const int cursorAfterCommit = qMin(anchor, cursor) + replacement.first + text.size();
        surrounding.replace(qMin(anchor, cursor) + replacement.first,
                            qAbs(anchor - cursor) + replacement.second,
                            text);

        attributes.push_back(QInputMethodEvent::Attribute(
            QInputMethodEvent::Selection,
            indexFromWayland(surrounding, m_cursor, cursorAfterCommit) + absoluteOffset,
            indexFromWayland(surrounding, m_anchor, cursorAfterCommit) + absoluteOffset,
            QVariant()));
    }

    QInputMethodEvent *event = new QInputMethodEvent(QString(), attributes);
    event->setCommitString(text, replacement.first, replacement.second);

    return event;
}

QInputMethodEvent *QWaylandInputMethodEventBuilder::buildPreedit(const QString &text)
{
    QList<QInputMethodEvent::Attribute> attributes;

    if (m_preeditCursor < 0) {
        attributes.append(
            QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, 0, 0, QVariant()));
    } else {
        attributes.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor,
                                                       indexFromWayland(text, m_preeditCursor),
                                                       1,
                                                       QVariant()));
    }

    for (const QInputMethodEvent::Attribute &attr : std::as_const(m_preeditStyles)) {
        int start = indexFromWayland(text, attr.start);
        int length = indexFromWayland(text, attr.start + attr.length) - start;
        attributes.append(QInputMethodEvent::Attribute(attr.type, start, length, attr.value));
    }

    QInputMethodEvent *event = new QInputMethodEvent(text, attributes);

    const QPair<int, int> replacement = replacementForDeleteSurrounding();
    event->setCommitString(QString(), replacement.first, replacement.second);

    return event;
}

QPair<int, int> QWaylandInputMethodEventBuilder::replacementForDeleteSurrounding()
{
    if (m_deleteBefore == 0 && m_deleteAfter == 0)
        return QPair<int, int>(0, 0);

    const QString &surrounding =
        QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant()).toString();
    const int cursor = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant()).toInt();
    const int anchor = QInputMethod::queryFocusObject(Qt::ImAnchorPosition, QVariant()).toInt();

    const int selectionStart = qMin(cursor, anchor);
    const int selectionEnd = qMax(cursor, anchor);

    const int deleteBefore =
        selectionStart - indexFromWayland(surrounding, -m_deleteBefore, selectionStart);
    const int deleteAfter =
        indexFromWayland(surrounding, m_deleteAfter, selectionEnd) - selectionEnd;

    return QPair<int, int>(-deleteBefore, deleteBefore + deleteAfter);
}

QWaylandInputMethodContentType QWaylandInputMethodContentType::convertV4(Qt::InputMethodHints hints)
{
    uint32_t hint = ZWP_TEXT_INPUT_V3_CONTENT_HINT_NONE;
    uint32_t purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NORMAL;

    if (hints & Qt::ImhHiddenText)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_HIDDEN_TEXT;
    if (hints & Qt::ImhSensitiveData)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_SENSITIVE_DATA;
    if ((hints & Qt::ImhNoAutoUppercase) == 0)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_AUTO_CAPITALIZATION;
    if (hints & Qt::ImhPreferNumbers) {
        // Nothing yet
    }
    if (hints & Qt::ImhPreferUppercase)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_UPPERCASE;
    if (hints & Qt::ImhPreferLowercase)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_LOWERCASE;
    if ((hints & Qt::ImhNoPredictiveText) == 0) {
        hint |=
            (ZWP_TEXT_INPUT_V3_CONTENT_HINT_COMPLETION | ZWP_TEXT_INPUT_V3_CONTENT_HINT_SPELLCHECK);
    }

    if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime) == 0)
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DATE;
    else if ((hints & Qt::ImhDate) && (hints & Qt::ImhTime))
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DATETIME;
    else if ((hints & Qt::ImhDate) == 0 && (hints & Qt::ImhTime))
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_TIME;
    if (hints & Qt::ImhPreferLatin)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_LATIN;
    if (hints & Qt::ImhMultiLine)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_MULTILINE;
    if (hints & Qt::ImhDigitsOnly)
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_DIGITS;
    if (hints & Qt::ImhFormattedNumbersOnly)
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_NUMBER;
    if (hints & Qt::ImhUppercaseOnly)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_UPPERCASE;
    if (hints & Qt::ImhLowercaseOnly)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_LOWERCASE;
    if (hints & Qt::ImhDialableCharactersOnly)
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_PHONE;
    if (hints & Qt::ImhEmailCharactersOnly)
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_EMAIL;
    if (hints & Qt::ImhUrlCharactersOnly)
        purpose = ZWP_TEXT_INPUT_V3_CONTENT_PURPOSE_URL;
    if (hints & Qt::ImhLatinOnly)
        hint |= ZWP_TEXT_INPUT_V3_CONTENT_HINT_LATIN;

    return QWaylandInputMethodContentType{ hint, purpose };
}

int QWaylandInputMethodEventBuilder::indexFromWayland(const QString &text, int length, int base)
{
    if (length == 0)
        return base;

    if (length < 0) {
        const QByteArray &utf8 = QStringView{ text }.left(base).toUtf8();
        return QString::fromUtf8(utf8.left(qMax(utf8.size() + length, 0))).size();
    } else {
        const QByteArray &utf8 = QStringView{ text }.mid(base).toUtf8();
        return QString::fromUtf8(utf8.left(length)).size() + base;
    }
}

int QWaylandInputMethodEventBuilder::trimmedIndexFromWayland(const QString &text,
                                                             int length,
                                                             int base)
{
    if (length == 0)
        return base;

    if (length < 0) {
        const QByteArray &utf8 = QStringView{ text }.left(base).toUtf8();
        const int len = utf8.size();
        const int start = len + length;
        if (start <= 0)
            return 0;

        for (int i = 0; i < 4; i++) {
            if (start + i >= len)
                return base;

            const unsigned char ch = utf8.at(start + i);
            // check if current character is a utf8's initial character.
            if (ch < 0x80 || ch > 0xbf)
                return QString::fromUtf8(utf8.left(start + i)).size();
        }
    } else {
        const QByteArray &utf8 = QStringView{ text }.mid(base).toUtf8();
        const int len = utf8.size();
        const int start = length;
        if (start >= len)
            return base + QString::fromUtf8(utf8).size();

        for (int i = 0; i < 4; i++) {
            const unsigned char ch = utf8.at(start - i);
            // check if current character is a utf8's initial character.
            if (ch < 0x80 || ch > 0xbf)
                return base + QString::fromUtf8(utf8.left(start - i)).size();
        }
    }
    return -1;
}

int QWaylandInputMethodEventBuilder::indexToWayland(const QString &text, int length, int base)
{
    return QStringView{ text }.mid(base, length).toUtf8().size();
}

QT_END_NAMESPACE

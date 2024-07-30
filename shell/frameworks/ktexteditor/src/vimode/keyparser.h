/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2008 Evgeniy Ivanov <powerfox@kde.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_KEY_PARSER_H
#define KATEVI_KEY_PARSER_H

#include <QChar>
#include <QHash>
#include <QString>
#include <ktexteditor_export.h>

class QKeyEvent;

namespace KateVi
{

class KeyEvent;

/**
 * for encoding keypresses w/ modifiers into an internal QChar representation and back again to a
 * descriptive text string
 */
class KeyParser
{
private:
    KeyParser();

public:
    KTEXTEDITOR_EXPORT static KeyParser *self();
    ~KeyParser()
    {
        m_instance = nullptr;
    }
    KeyParser(const KeyParser &) = delete;
    KeyParser &operator=(const KeyParser &) = delete;

    KTEXTEDITOR_EXPORT const QString encodeKeySequence(const QString &keys) const;
    KTEXTEDITOR_EXPORT const QString decodeKeySequence(const QString &keys) const;
    QString qt2vi(int key) const;
    KTEXTEDITOR_EXPORT int vi2qt(const QString &keypress) const;
    int encoded2qt(const QString &keypress) const;
    KTEXTEDITOR_EXPORT const QChar KeyEventToQChar(const QKeyEvent &keyEvent);
    const QChar KeyEventToQChar(const KeyEvent &keyEvent);

private:
    void initKeyTables();
    const QChar KeyEventToQChar(int keyCode, const QString &text, Qt::KeyboardModifiers mods) const;

    QHash<int, QString> m_qt2katevi;
    QHash<QString, int> m_katevi2qt;
    QHash<QString, int> m_nameToKeyCode;
    QHash<int, QString> m_keyCodeToName;

    static KeyParser *m_instance;
};

}

#endif /* KATEVI_KEY_PARSER_H */

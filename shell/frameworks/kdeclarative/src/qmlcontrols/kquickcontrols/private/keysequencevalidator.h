/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KEYSEQUENCEVALIDATOR_H
#define KEYSEQUENCEVALIDATOR_H

#include <QKeySequence>
#include <QObject>
#include <QQuickItem>

#include "keysequencehelper.h"

class KeySequenceValidator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QKeySequence currentKeySequence READ currentKeySequence WRITE setCurrentKeySequence NOTIFY currentKeySequenceChanged)
    Q_PROPERTY(KeySequenceHelper::ShortcutTypes validateTypes READ validateTypes WRITE setValidateTypes NOTIFY validateTypesChanged)

public:
    /**
     * Constructor.
     */
    explicit KeySequenceValidator(QObject *parent = nullptr);

    QKeySequence currentKeySequence() const;
    void setCurrentKeySequence(const QKeySequence &sequence);
    Q_SIGNAL void currentKeySequenceChanged();

    KeySequenceHelper::ShortcutTypes validateTypes() const;
    void setValidateTypes(KeySequenceHelper::ShortcutTypes types);
    Q_SIGNAL void validateTypesChanged();

    Q_INVOKABLE void validateSequence(const QKeySequence &keySequence);
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();

    Q_SIGNAL void error(const QString &title, const QString &message);
    Q_SIGNAL void question(const QString &title, const QString &message);
    Q_SIGNAL void finished(const QKeySequence &keySequence);

private:
    enum class ValidationResult {
        Reject,
        Accept,
        QuestionPending,
    };

    ValidationResult validateGlobalShortcut(const QKeySequence &keySequence);
    ValidationResult validateStandardShortcut(const QKeySequence &keySequence);

    KeySequenceHelper::ShortcutTypes m_validateTypes = KeySequenceHelper::GlobalShortcuts | KeySequenceHelper::StandardShortcuts;
    QKeySequence m_currentKeySequence;
    QKeySequence m_pendingKeySequence;
    bool m_validateGlobalPending = false;
};

#endif // KEYSEQUENCEVALIDATOR_H

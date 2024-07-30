/*
    SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 David Redondo <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "keysequencevalidator.h"

#include <KLocalizedString>
#include <KStandardShortcut>

#include <config-kdeclarative.h>
#if HAVE_KGLOBALACCEL
#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#endif

KeySequenceValidator::KeySequenceValidator(QObject *parent)
    : QObject(parent)
{
}

QKeySequence KeySequenceValidator::currentKeySequence() const
{
    return m_currentKeySequence;
}

void KeySequenceValidator::setCurrentKeySequence(const QKeySequence &sequence)
{
    if (m_currentKeySequence == sequence) {
        return;
    }

    m_currentKeySequence = sequence;
    Q_EMIT currentKeySequenceChanged();
}

KeySequenceHelper::ShortcutTypes KeySequenceValidator::validateTypes() const
{
    return m_validateTypes;
}

void KeySequenceValidator::setValidateTypes(KeySequenceHelper::ShortcutTypes types)
{
    if (m_validateTypes == types) {
        return;
    }

    m_validateTypes = types;
    Q_EMIT validateTypesChanged();
}

void KeySequenceValidator::validateSequence(const QKeySequence &keySequence)
{
    ValidationResult result = ValidationResult::Accept;
    if (m_validateTypes & KeySequenceHelper::GlobalShortcuts) {
        result = validateGlobalShortcut(keySequence);
    }

    if (result == ValidationResult::Reject) {
        Q_EMIT finished(m_currentKeySequence);
        return;
    }

    if (result == ValidationResult::QuestionPending) {
        return;
    }

    if (m_validateTypes & KeySequenceHelper::StandardShortcuts) {
        result = validateStandardShortcut(keySequence);
    }

    if (result == ValidationResult::Reject) {
        Q_EMIT finished(m_currentKeySequence);
    } else if (result == ValidationResult::Accept) {
        Q_EMIT finished(keySequence);
    }
}

void KeySequenceValidator::accept()
{
    QKeySequence keySequence = m_pendingKeySequence;
    m_pendingKeySequence = QKeySequence{};

    ValidationResult result = ValidationResult::Accept;
    if (m_validateGlobalPending) {
        m_validateGlobalPending = false;
        if (m_validateTypes & KeySequenceHelper::StandardShortcuts) {
            result = validateStandardShortcut(keySequence);
        }
    }

    if (result == ValidationResult::Reject) {
        Q_EMIT finished(m_currentKeySequence);
    } else if (result == ValidationResult::Accept) {
        Q_EMIT finished(keySequence);
    }
}

void KeySequenceValidator::reject()
{
    m_pendingKeySequence = QKeySequence{};
    Q_EMIT finished(m_currentKeySequence);
}

KeySequenceValidator::ValidationResult KeySequenceValidator::validateGlobalShortcut(const QKeySequence &keySequence)
{
#ifdef Q_OS_WIN
    // on windows F12 is reserved by the debugger at all times, so we can't use it for a global shortcut
    if (keySequence.toString().contains(QLatin1String("F12"))) {
        QString title = i18n("Reserved Shortcut");
        QString message = i18n(
            "The F12 key is reserved on Windows, so cannot be used for a global shortcut.\n"
            "Please choose another one.");

        Q_EMIT error(title, message);
        return ValidationResult::Reject;
    } else {
        return ValidationResult::Accept;
    }
#elif HAVE_KGLOBALACCEL
    // Global shortcuts are on key+modifier shortcuts. They can clash with a multi key shortcut.
    QList<KGlobalShortcutInfo> others;
    QList<KGlobalShortcutInfo> shadow;
    QList<KGlobalShortcutInfo> shadowed;
    if (!KGlobalAccel::isGlobalShortcutAvailable(keySequence, QString())) {
        others << KGlobalAccel::globalShortcutsByKey(keySequence);

        // look for shortcuts shadowing
        shadow << KGlobalAccel::globalShortcutsByKey(keySequence, KGlobalAccel::MatchType::Shadows);
        shadowed << KGlobalAccel::globalShortcutsByKey(keySequence, KGlobalAccel::MatchType::Shadowed);
    }

    if (!shadow.isEmpty() || !shadowed.isEmpty()) {
        QString title = i18n("Global Shortcut Shadowing");
        QString message;
        if (!shadowed.isEmpty()) {
            message += i18n("The '%1' key combination is shadowed by following global actions:\n").arg(keySequence.toString());
            for (const KGlobalShortcutInfo &info : std::as_const(shadowed)) {
                message += i18n("Action '%1' in context '%2'\n").arg(info.friendlyName(), info.contextFriendlyName());
            }
        }
        if (!shadow.isEmpty()) {
            message += i18n("The '%1' key combination shadows following global actions:\n").arg(keySequence.toString());
            for (const KGlobalShortcutInfo &info : std::as_const(shadow)) {
                message += i18n("Action '%1' in context '%2'\n").arg(info.friendlyName(), info.contextFriendlyName());
            }
        }

        Q_EMIT error(title, message);
        return ValidationResult::Reject;
    }

    if (!others.isEmpty()) {
        QString title = i18nc("@dialog:title", "Found Conflict");
        QString message;

        if (others.size() == 1) {
            auto info = others.at(0);
            message = i18n("Shortcut '%1' is already assigned to action '%2' of %3.\nDo you want to reassign it?",
                           keySequence.toString(),
                           info.friendlyName(),
                           info.componentFriendlyName());
        } else {
            message = i18n("Shortcut '%1' is already assigned to the following actions:\n");
            for (const auto &info : std::as_const(others)) {
                message += i18n("Action '%1' of %2\n", info.friendlyName(), info.componentFriendlyName());
            }
            message += i18n("Do you want to reassign it?");
        }

        m_pendingKeySequence = keySequence;
        m_validateGlobalPending = true;
        Q_EMIT question(title, message);
        return ValidationResult::QuestionPending;
    }
#endif
    return ValidationResult::Accept;
}

KeySequenceValidator::ValidationResult KeySequenceValidator::validateStandardShortcut(const QKeySequence &keySequence)
{
    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
    if (ssc != KStandardShortcut::AccelNone) {
        QString title = i18n("Conflict with Standard Application Shortcut");
        QString message = i18n(
            "The '%1' key combination is also used for the standard action "
            "\"%2\" that some applications use.\n"
            "Do you really want to use it as a global shortcut as well?",
            keySequence.toString(QKeySequence::NativeText),
            KStandardShortcut::label(ssc));

        m_pendingKeySequence = keySequence;
        Q_EMIT question(title, message);
        return ValidationResult::QuestionPending;
    } else {
        return ValidationResult::Accept;
    }
}

#include "moc_keysequencevalidator.cpp"

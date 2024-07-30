/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "macros.h"
#include <vimode/keyparser.h>

#include <KConfigGroup>

using namespace KateVi;

Macros::Macros() = default;

Macros::~Macros() = default;

void Macros::writeConfig(KConfigGroup &config) const
{
    const auto macroKeys = m_macros.keys();
    QStringList macroRegisters;
    for (const QChar macroRegister : macroKeys) {
        macroRegisters.append(macroRegister);
    }
    QStringList macroContents;
    for (const QChar macroRegister : macroKeys) {
        macroContents.append(KeyParser::self()->decodeKeySequence(m_macros[macroRegister]));
    }
    QStringList macroCompletions;
    for (const QChar macroRegister : macroKeys) {
        macroCompletions.append(QString::number(m_completions[macroRegister].length()));
        for (const Completion &completionForMacro : m_completions[macroRegister]) {
            macroCompletions.append(encodeMacroCompletionForConfig(completionForMacro));
        }
    }
    config.writeEntry("Macro Registers", macroRegisters);
    config.writeEntry("Macro Contents", macroContents);
    config.writeEntry("Macro Completions", macroCompletions);
}

void Macros::readConfig(const KConfigGroup &config)
{
    const QStringList macroRegisters = config.readEntry("Macro Registers", QStringList());
    const QStringList macroContents = config.readEntry("Macro Contents", QStringList());
    const QStringList macroCompletions = config.readEntry("Macro Completions", QStringList());
    int macroCompletionsIndex = 0;
    if (macroRegisters.length() == macroContents.length()) {
        for (int macroIndex = 0; macroIndex < macroRegisters.length(); macroIndex++) {
            const QChar macroRegister = macroRegisters[macroIndex].at(0);
            m_macros[macroRegister] = KeyParser::self()->encodeKeySequence(macroContents[macroIndex]);
            macroCompletionsIndex = readMacroCompletions(macroRegister, macroCompletions, macroCompletionsIndex);
        }
    }
}

void Macros::clear()
{
    m_macros.clear();
}

void Macros::remove(const QChar &reg)
{
    m_macros.remove(reg);
}

void Macros::store(const QChar &reg, const QList<KeyEvent> &macroKeyEventLog, const CompletionList &completions)
{
    m_macros[reg].clear();
    QList<KeyEvent> withoutClosingQ = macroKeyEventLog;
    Q_ASSERT(!macroKeyEventLog.isEmpty() && macroKeyEventLog.last().key() == Qt::Key_Q);
    withoutClosingQ.pop_back();
    for (const KeyEvent &keyEvent : std::as_const(withoutClosingQ)) {
        const QChar key = KeyParser::self()->KeyEventToQChar(keyEvent);
        m_macros[reg].append(key);
    }
    m_completions[reg] = completions;
}

QString Macros::get(const QChar &reg) const
{
    return m_macros.contains(reg) ? m_macros[reg] : QString();
}

CompletionList Macros::getCompletions(const QChar &reg) const
{
    return m_completions.contains(reg) ? m_completions[reg] : CompletionList();
}

int Macros::readMacroCompletions(const QChar &reg, const QStringList &encodedMacroCompletions, int macroCompletionsIndex)
{
    if (macroCompletionsIndex < encodedMacroCompletions.length()) {
        bool parsedNumCompletionsSuccessfully = false;
        const QString numCompletionsAsString = encodedMacroCompletions[macroCompletionsIndex++];
        const int numCompletions = numCompletionsAsString.toInt(&parsedNumCompletionsSuccessfully);
        int count = 0;
        m_completions[reg].clear();
        while (count < numCompletions && macroCompletionsIndex < encodedMacroCompletions.length()) {
            const QString encodedMacroCompletion = encodedMacroCompletions[macroCompletionsIndex++];
            count++;
            m_completions[reg].append(decodeMacroCompletionFromConfig(encodedMacroCompletion));
        }
    }
    return macroCompletionsIndex;
}

QString Macros::encodeMacroCompletionForConfig(const Completion &completionForMacro)
{
    const bool endedWithSemiColon = completionForMacro.completedText().endsWith(QLatin1Char(';'));
    QString encodedMacroCompletion = completionForMacro.completedText().remove(QStringLiteral("()")).remove(QLatin1Char(';'));
    if (completionForMacro.completionType() == Completion::FunctionWithArgs) {
        encodedMacroCompletion += QLatin1String("(...)");
    } else if (completionForMacro.completionType() == Completion::FunctionWithoutArgs) {
        encodedMacroCompletion += QLatin1String("()");
    }
    if (endedWithSemiColon) {
        encodedMacroCompletion += QLatin1Char(';');
    }
    if (completionForMacro.removeTail()) {
        encodedMacroCompletion += QLatin1Char('|');
    }
    return encodedMacroCompletion;
}

Completion Macros::decodeMacroCompletionFromConfig(const QString &encodedMacroCompletion)
{
    const bool removeTail = encodedMacroCompletion.endsWith(QLatin1Char('|'));
    Completion::CompletionType completionType = Completion::PlainText;
    if (encodedMacroCompletion.contains(QLatin1String("(...)"))) {
        completionType = Completion::FunctionWithArgs;
    } else if (encodedMacroCompletion.contains(QLatin1String("()"))) {
        completionType = Completion::FunctionWithoutArgs;
    }
    QString completionText = encodedMacroCompletion;
    completionText.replace(QLatin1String("(...)"), QLatin1String("()")).remove(QLatin1Char('|'));

    return Completion(completionText, removeTail, completionType);
}

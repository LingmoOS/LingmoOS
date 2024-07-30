/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_MACROS_H
#define KATEVI_MACROS_H

#include "completion.h"
#include "keyevent.h"
#include "ktexteditor_export.h"

#include <QKeyEvent>

class KConfigGroup;

namespace KateVi
{
class Macros
{
public:
    explicit Macros();
    ~Macros();

    void writeConfig(KConfigGroup &config) const;
    void readConfig(const KConfigGroup &config);

    void store(const QChar &reg, const QList<KeyEvent> &macroKeyEventLog, const CompletionList &completions);
    void remove(const QChar &reg);
    KTEXTEDITOR_EXPORT void clear();

    QString get(const QChar &reg) const;
    CompletionList getCompletions(const QChar &reg) const;

private:
    int readMacroCompletions(const QChar &reg, const QStringList &encodedMacroCompletions, int macroCompletionIndex);
    static QString encodeMacroCompletionForConfig(const Completion &completionForMacro);
    static Completion decodeMacroCompletionFromConfig(const QString &encodedMacroCompletion);

private:
    QHash<QChar, QString> m_macros;
    QHash<QChar, QList<Completion>> m_completions;
};

}

#endif // KATEVI_MACROS_H

/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_COMPLETIONREPLAYER_H
#define KATEVI_COMPLETIONREPLAYER_H

#include "completion.h"

#include <QStack>

namespace KTextEditor
{
class Cursor;
}

namespace KateVi
{
class InputModeManager;

class CompletionReplayer
{
public:
    explicit CompletionReplayer(InputModeManager *viInputModeManager);
    ~CompletionReplayer();

    void start(const CompletionList &completions);
    void stop();

    void replay();

private:
    Completion nextCompletion();
    int findNextMergeableBracketPos(const KTextEditor::Cursor startPos) const;

private:
    InputModeManager *m_viInputModeManager;

    QStack<CompletionList> m_CompletionsToReplay;
    QStack<int> m_nextCompletionIndex;
};

}

#endif // KATEVI_COMPLETIONREPLAYER_H

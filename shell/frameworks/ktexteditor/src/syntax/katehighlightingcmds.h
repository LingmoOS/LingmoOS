/*
    SPDX-FileCopyrightText: 2014 Christoph Rüßler <christoph.ruessler@mailbox.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_HIGHLIGHTING_CMDS_H
#define KATE_HIGHLIGHTING_CMDS_H

#include <KTextEditor/Command>

namespace KateCommands
{
class Highlighting : public KTextEditor::Command
{
    Highlighting()
        : KTextEditor::Command({QStringLiteral("reload-highlighting"), QStringLiteral("edit-highlighting")})
    {
    }

    static Highlighting *m_instance;

public:
    ~Highlighting() override
    {
        m_instance = nullptr;
    }

    static Highlighting *self()
    {
        if (m_instance == nullptr) {
            m_instance = new Highlighting();
        }
        return m_instance;
    }

    /**
     * execute command
     * @param view view to use for execution
     * @param cmd cmd string
     * @param errorMsg error to return if no success
     * @return success
     */
    bool exec(class KTextEditor::View *view, const QString &cmd, QString &errorMsg, const KTextEditor::Range &range = KTextEditor::Range::invalid()) override;

    /** This command does not have help. @see KTextEditor::Command::help */
    bool help(class KTextEditor::View *, const QString &, QString &) override;
};

}

#endif

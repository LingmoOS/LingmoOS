/*
    SPDX-FileCopyrightText: 2014 Christoph Rüßler <christoph.ruessler@mailbox.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katehighlightingcmds.h"

#include <ktexteditor/application.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/view.h>

#include "katedocument.h"
#include "katehighlight.h"
#include "katesyntaxmanager.h"

#include <QUrl>

KateCommands::Highlighting *KateCommands::Highlighting::m_instance = nullptr;

bool KateCommands::Highlighting::exec(KTextEditor::View *view, const QString &cmd, QString &, const KTextEditor::Range &)
{
    if (cmd.startsWith(QLatin1String("reload-highlighting"))) {
        KateHlManager::self()->reload();
        return true;
    } else if (cmd.startsWith(QLatin1String("edit-highlighting"))) {
        KTextEditor::DocumentPrivate *document = static_cast<KTextEditor::DocumentPrivate *>(view->document());
        KateHighlighting *highlighting = document->highlight();

        if (!highlighting->noHighlighting()) {
            QUrl url = QUrl::fromLocalFile(highlighting->getIdentifier());
            KTextEditor::Application *app = KTextEditor::Editor::instance()->application();
            app->openUrl(url);
        }

        return true;
    }

    return true;
}

bool KateCommands::Highlighting::help(KTextEditor::View *, const QString &, QString &)
{
    return false;
}

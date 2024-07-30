/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateviinputmodefactory.h"
#include "kateviinputmode.h"
#include "vimode/globalstate.h"
#include <vimode/config/configtab.h>

#include <KConfig>
#include <KLocalizedString>

KateViInputModeFactory::KateViInputModeFactory()
    : KateAbstractInputModeFactory()
    , m_viGlobal(new KateVi::GlobalState())
{
}

KateAbstractInputMode *KateViInputModeFactory::createInputMode(KateViewInternal *viewInternal)
{
    return new KateViInputMode(viewInternal, m_viGlobal.get());
}

KateConfigPage *KateViInputModeFactory::createConfigPage(QWidget *parent)
{
    return new KateVi::ConfigTab(parent, m_viGlobal->mappings());
}

KTextEditor::View::InputMode KateViInputModeFactory::inputMode()
{
    return KTextEditor::View::ViInputMode;
}

QString KateViInputModeFactory::name()
{
    return i18n("Vi Input Mode");
}

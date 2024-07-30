/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katenormalinputmodefactory.h"
#include "katenormalinputmode.h"

#include <KLocalizedString>

KateAbstractInputMode *KateNormalInputModeFactory::createInputMode(KateViewInternal *viewInternal)
{
    return new KateNormalInputMode(viewInternal);
}

KateConfigPage *KateNormalInputModeFactory::createConfigPage(QWidget *)
{
    return nullptr;
}

KTextEditor::View::InputMode KateNormalInputModeFactory::inputMode()
{
    return KTextEditor::View::NormalInputMode;
}

QString KateNormalInputModeFactory::name()
{
    return i18n("Normal Mode");
}

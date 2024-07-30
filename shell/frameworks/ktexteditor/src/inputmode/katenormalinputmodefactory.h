/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_NORMAL_INPUT_MODE_FACTORY_H
#define KATE_NORMAL_INPUT_MODE_FACTORY_H

#include "kateabstractinputmodefactory.h"

class KateNormalInputModeFactory : public KateAbstractInputModeFactory
{
public:
    KateNormalInputModeFactory() = default;
    KateAbstractInputMode *createInputMode(KateViewInternal *viewInternal) override;

    QString name() override;
    KTextEditor::View::InputMode inputMode() override;

    KateConfigPage *createConfigPage(QWidget *) override;
};

#endif

/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_ABSTRACT_INPUT_MODE_FACTORY_H
#define KATE_ABSTRACT_INPUT_MODE_FACTORY_H

class KateAbstractInputMode;
class KateViewInternal;

class KConfig;
class KateConfigPage;

#include "ktexteditor/view.h"
#include <QString>
class QWidget;

class KateAbstractInputModeFactory
{
public:
    KateAbstractInputModeFactory();

    virtual ~KateAbstractInputModeFactory();
    virtual KateAbstractInputMode *createInputMode(KateViewInternal *viewInternal) = 0;

    virtual QString name() = 0;
    virtual KTextEditor::View::InputMode inputMode() = 0;

    virtual KateConfigPage *createConfigPage(QWidget *) = 0;
};

#endif

/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_DEFINITIONS_H
#define KATEVI_DEFINITIONS_H

#ifdef Q_OS_MACOS
// From the Qt docs: On macOS, the ControlModifier value corresponds to the Command keys on the
// keyboard, and the MetaModifier value corresponds to the Control keys.
#define CONTROL_MODIFIER Qt::MetaModifier
#define META_MODIFIER Qt::ControlModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#define META_MODIFIER Qt::MetaModifier
#endif

namespace KateVi
{
/**
 * The four vi modes supported by Kate's vi input mode
 */
enum ViMode { NormalMode, InsertMode, VisualMode, VisualLineMode, VisualBlockMode, ReplaceMode };

enum OperationMode { CharWise = 0, LineWise, Block };

const unsigned int EOL = 99999;
}

#endif // KATEVI_DEFINITIONS_H

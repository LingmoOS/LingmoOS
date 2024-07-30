/*
    SPDX-FileCopyrightText: 2001, 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEEXTENDEDATTRIBUTE_H
#define KATEEXTENDEDATTRIBUTE_H

#include <ktexteditor/attribute.h>

/**
 * Custom property types, which may or may not be supported by implementations.
 * Internally used
 */
enum CustomProperties {
    /// Draws an outline around the text
    Outline = QTextFormat::UserProperty,
    /// Changes the brush used to paint the text when it is selected
    SelectedForeground,
    /// Changes the brush used to paint the background when it is selected
    SelectedBackground,
    /// Determines whether background color is drawn over whitespace. Defaults to true.
    BackgroundFillWhitespace,
    /// Defined to allow storage of dynamic effect information
    AttributeDynamicEffect = 0x10A00,
    /// Defined for internal usage of KTextEditor implementations
    AttributeInternalProperty = 0x10E00,
    AttributeName = AttributeInternalProperty,
    AttributeDefaultStyleIndex,
    Spellchecking,
    /// Defined to allow 3rd party code to create their own custom attributes - you may use values at or above this property.
    AttributeUserProperty = 0x110000
};

typedef QList<KTextEditor::Attribute::Ptr> KateAttributeList;

#endif

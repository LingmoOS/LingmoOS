/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_UTIL_P_H
#define KTEXTTEMPLATE_UTIL_P_H

#include "context.h"
#include "ktexttemplate_export.h"
#include "safestring.h"

#include <QVariant>

/// @file util.h Utility functions used throughout %KTextTemplate.
/// @headerfile util.h <KTextTemplate/Util>

namespace KTextTemplate
{

/**
  Returns whether the @p variant is evaluated to true.

  @see @ref truthiness
*/
KTEXTTEMPLATE_EXPORT bool variantIsTrue(const QVariant &variant);

/**
  Converts the @p input into its unescaped form.

  Double slashes are converted to single slashes, \\&quot is converted to &quot
  etc.
*/
KTEXTTEMPLATE_EXPORT QString unescapeStringLiteral(const QString &input);

/**
  Marks the @p input as safe.
*/
KTEXTTEMPLATE_EXPORT KTextTemplate::SafeString markSafe(const KTextTemplate::SafeString &input);

/**
  Marks the @p input as requiring escaping.
*/
KTEXTTEMPLATE_EXPORT KTextTemplate::SafeString markForEscaping(const KTextTemplate::SafeString &input);

/**
  Retrieves and returns a SafeString from the @p input.
*/
KTEXTTEMPLATE_EXPORT KTextTemplate::SafeString getSafeString(const QVariant &input);

/**
  Returns whether @p input contains a SafeString.
*/
KTEXTTEMPLATE_EXPORT bool isSafeString(const QVariant &input);

/**
  Returns true if the type of @p input can be inserted into a rendered template
  directly.

  Note that lists, hashes and QObject*s can not be.
*/
KTEXTTEMPLATE_EXPORT bool supportedOutputType(const QVariant &input);

/**
  Compares @p lhs and @p rhs for equality. SafeStrings are compared as raw
  QStrings. Their safeness is not part of the comparison.

  @see QVariant::operator==
*/
KTEXTTEMPLATE_EXPORT bool equals(const QVariant &lhs, const QVariant &rhs);

/**
  Converts @p size into the nearest file size unit like MB or MiB, based on the
  @p unitSystem value. Use @c 2 for the @p unitSystem to get binary units, use
  @c 10 to get decimal units - by default, decimal units will be returned. The
  @p multiplier can be used if the input @p size is not in pure bytes. If @p
  size is for example given in @a KiB, use a multiplier of @a 1024. The returned
  pair will have the converted size as @a first and the unit as @a second.
*/
KTEXTTEMPLATE_EXPORT std::pair<qreal, QString> calcFileSize(qreal size, int unitSystem = 10, qreal multiplier = 1.0);

#ifndef K_DOXYGEN
/**
  @internal
  Returns the @p list as a formatted string. This is for debugging purposes
  only.
*/
KTEXTTEMPLATE_EXPORT KTextTemplate::SafeString toString(const QVariantList &list);
#endif
}

#endif

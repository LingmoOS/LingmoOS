/*
    This file is part of the KContacts framework.

    SPDX-FileCopyrightText: 2002-2010 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONTACTS_VCARDDRAG_H
#define KCONTACTS_VCARDDRAG_H

#include "kcontacts_export.h"

#include <QMimeData>
#include <kcontacts/addressee.h>

namespace KContacts
{
/**
  @brief Utility function for implementing drag&drop for vCards

  See the Qt drag'n'drop documentation.

  @since 4.5
*/
namespace VCardDrag
{
/**
  Adds the vCard representation as data of the drag object.

  @param md the object to set the data on
  @param content the vCard data to set

  @return Always @c true
*/
KCONTACTS_EXPORT bool populateMimeData(QMimeData *md, const QByteArray &content);

/**
  Adds the vCard representation as data of the drag object.

  @param md the object to set the data on
  @param contacts list of Addressee objects to serialize to vCard

  @return @c true if serializing of the given list worked, otherwise @c false

  @see VCardConverter::createVCards()
*/
KCONTACTS_EXPORT bool populateMimeData(QMimeData *md, const KContacts::Addressee::List &contacts);

/**
  Returns if drag&drop object can be decoded to vCard.

  @param md the object to check for vCard data

  @return @c true if the given data object contains a vCard MIME type, otherwise @c false
*/
KCONTACTS_EXPORT bool canDecode(const QMimeData *md);

/**
  Decodes the drag&drop object to vCard component @p content.

  @param md the object to check for vCard data
  @param content where to put the vCard data from @p md

  @return @c true if there was data for the vCard MIME type, otherwise @c false

  @see canDecode()
*/
KCONTACTS_EXPORT bool fromMimeData(const QMimeData *md, QByteArray &content);

/**
  Decodes the MIME data @p md and puts the resulting vCard into @p contacts.

  @param md the object to check for vCard data
  @param contacts where to put the parsed vCards from @p md

  @return @c true if there was data for the vCard MIME type and it could be parsed successfully,
          otherwise @c false

  @see canDecode()
*/
KCONTACTS_EXPORT bool fromMimeData(const QMimeData *md, KContacts::Addressee::List &contacts);
}
}

#endif // VCARDDRAG_H

/*
    This file is part of the syndication library

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_ITEMATOMIMPL_H
#define SYNDICATION_MAPPER_ITEMATOMIMPL_H

#include <atom/entry.h>
#include <item.h>

namespace Syndication
{
class ItemAtomImpl;
typedef QSharedPointer<ItemAtomImpl> ItemAtomImplPtr;

/**
 * @internal
 */
class ItemAtomImpl : public Syndication::Item
{
public:
    explicit ItemAtomImpl(const Syndication::Atom::Entry &entry);

    QString title() const override;

    QString link() const override;

    QString description() const override;

    QString content() const override;

    QList<PersonPtr> authors() const override;

    QString language() const override;

    QString id() const override;

    time_t datePublished() const override;

    time_t dateUpdated() const override;

    QList<EnclosurePtr> enclosures() const override;

    QList<CategoryPtr> categories() const override;

    SpecificItemPtr specificItem() const override;

    int commentsCount() const override;

    QString commentsLink() const override;

    QString commentsFeed() const override;

    QString commentPostUri() const override;

    QMultiMap<QString, QDomElement> additionalProperties() const override;

private:
    Syndication::Atom::Entry m_entry;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_ITEMATOMIMPL_H

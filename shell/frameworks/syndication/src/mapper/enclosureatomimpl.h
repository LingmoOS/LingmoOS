/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_MAPPER_ENCLOSUREATOMIMPL_H
#define SYNDICATION_MAPPER_ENCLOSUREATOMIMPL_H

#include <atom/link.h>
#include <enclosure.h>

namespace Syndication
{
class EnclosureAtomImpl;
typedef QSharedPointer<EnclosureAtomImpl> EnclosureAtomImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class EnclosureAtomImpl : public Syndication::Enclosure
{
public:
    explicit EnclosureAtomImpl(const Syndication::Atom::Link &link);

    Q_REQUIRED_RESULT bool isNull() const override;

    Q_REQUIRED_RESULT QString url() const override;

    Q_REQUIRED_RESULT QString title() const override;

    Q_REQUIRED_RESULT QString type() const override;

    Q_REQUIRED_RESULT uint length() const override;

    Q_REQUIRED_RESULT uint duration() const override;

private:
    Syndication::Atom::Link m_link;
};

} // namespace Syndication

#endif // SYNDICATION_MAPPER_ENCLOSUREATOMIMPL_H

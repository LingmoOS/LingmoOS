/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_MODELMAKER_H
#define SYNDICATION_RDF_MODELMAKER_H

#include <QSharedPointer>

#include "../syndication_export.h"

class QDomDocument;
class QDomElement;

namespace Syndication
{
namespace RDF
{
class Model;
class Resource;
//@cond PRIVATE
typedef QSharedPointer<Resource> ResourcePtr;
//@endcond

/**
 * An RDF parser, used to parse an RDF model from RDF/XML. It doesn't support
 * the complete RDF specification, but suffices for our purposes.
 *
 * @author Frank Osterfeld
 */
class ModelMaker
{
public:
    /**
     * parses an RDF model from RDF/XML
     * @param doc an DOM document, must contain RDF/XML
     * @return the parsed model, or an empty model if parsing failed
     */
    Model createFromXML(const QDomDocument &doc);

private:
    ResourcePtr readResource(Model &model, const QDomElement &el);
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_MODELMAKER_H

/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_RDF_DUBLINCORE_H
#define SYNDICATION_RDF_DUBLINCORE_H

#include <syndication/rdf/resourcewrapper.h>

#include <QStringList>

#include <ctime>

class QString;

namespace Syndication
{
namespace RDF
{
class Resource;
//@cond PRIVATE
typedef QSharedPointer<Resource> ResourcePtr;
//@endcond

/**
 * A resource wrapper providing convenient
 * access to Dublin Core metadata.
 *
 * For more information on Dublin Core, see
 * http://dublincore.org/
 */
class DublinCore : public ResourceWrapper
{
public:
    /**
     * creates a dublin core convenience wrapper for a resource
     *
     * @param resource the resource to wrap
     */
    explicit DublinCore(ResourcePtr resource);

    /**
     * virtual destructor
     */
    ~DublinCore() override;

    /**
     * A name given to the resource.
     * Typically, a Title will be a name by which the resource is
     * formally known.
     */
    QString title() const;

    /**
     * "An entity primarily responsible for making the content of
     * the resource.
     * Examples of a Creator include a person, an organization,
     * or a service.
     * Typically, the name of a Creator should be used to
     * indicate the entity."
     */
    QString creator() const;

    /**
     * like creator(), but returns all dc:creator properties,
     * not only one.
     */
    QStringList creators() const;

    /**
     * "A date associated with an event in the life cycle of the resource.
     * Typically, Date will be associated with the creation or
     * availability of the resource.  Recommended best practice
     * for encoding the date value is defined in a profile of
     * ISO 8601 [W3CDTF] and follows the YYYY-MM-DD format."
     */
    time_t date() const;

    /**
     * "An account of the content of the resource.
     * Description may include but is not limited to: an abstract,
     * table of contents, reference to a graphical representation
     * of content or a free-text account of the content."
     */
    QString description() const;

    /**
     * "The topic of the content of the resource.
     * Typically, a Subject will be expressed as keywords,
     * key phrases or classification codes that describe a topic
     * of the resource.
     * Recommended best practice is to select a value from a
     * controlled vocabulary or formal classification scheme."
     */
    QString subject() const;

    /**
     * like subject(), but returns all dc:subject properties,
     * not only one.
     */
    QStringList subjects() const;

    /**
     * "An entity responsible for making contributions to the content of the
     * resource.
     * Examples of a Contributor include a person, an organization, or a
     * service. Typically, the name of a Contributor should be used to
     * indicate the entity."
     * */
    QString contributor() const;

    /**
     * like contributor(), but returns all dc:contributor properties,
     * not only one.
     */
    QStringList contributors() const;

    /**
     * "Information about rights held in and over the resource.
     * Typically, a Rights element will contain a rights
     * management statement for the resource, or reference
     * a service providing such information. Rights information
     * often encompasses Intellectual Property Rights (IPR),
     * Copyright, and various Property Rights.
     * If the Rights element is absent, no assumptions can be made
     * about the status of these and other rights with respect to
     * the resource."
     */
    QString rights() const;

    /**
     * "A language of the intellectual content of the resource.
     * Recommended best practice for the values of the Language
     * element is defined by RFC 1766 [RFC1766] which includes
     * a two-letter Language Code (taken from the ISO 639
     * standard [ISO639]), followed optionally, by a two-letter
     * Country Code (taken from the ISO 3166 standard [ISO3166]).
     * For example, 'en' for English, 'fr' for French, or
     * 'en-uk' for English used in the United Kingdom."
     */
    QString language() const;

    /**
     * "The extent or scope of the content of the resource.
     * Coverage will typically include spatial location (a place name
     * or geographic coordinates), temporal period (a period label,
     * date, or date range) or jurisdiction (such as a named
     * administrative entity).
     * Recommended best practice is to select a value from a
     * controlled vocabulary (for example, the Thesaurus of Geographic
     * Names [TGN]) and that, where appropriate, named places or time
     * periods be used in preference to numeric identifiers such as
     * sets of coordinates or date ranges."
     */
    QString coverage() const;

    /**
     * "The physical or digital manifestation of the resource.
     * Typically, Format may include the media-type or dimensions of
     * the resource. Format may be used to determine the software,
     * hardware or other equipment needed to display or operate the
     * resource. Examples of dimensions include size and duration.
     * Recommended best practice is to select a value from a
     * controlled vocabulary (for example, the list of Internet Media
     * Types [MIME] defining computer media formats).
     */
    QString format() const;

    /**
     * "An unambiguous reference to the resource within a given context.
     * Recommended best practice is to identify the resource by means
     * if a string or number conforming to a formal identification
     * system.
     * Example formal identification systems include the Uniform
     * Resource Identifier (URI) (including the Uniform Resource
     * Locator (URL)), the Digital Object Identifier (DOI) and the
     * International Standard Book Number (ISBN).
     */
    QString identifier() const;

    /**
     * "An entity responsible for making the resource available.
     * Examples of a Publisher include a person, an organization, or a
     * service. Typically, the name of a Publisher should be used to
     * indicate the entity."
     */
    QString publisher() const;

    /**
     * "A reference to a related resource.
     * Recommended best practice is to reference the resource by means
     * of a string or number conforming to a formal identification
     * system."
     */
    QString relation() const;

    /**
     * A Reference to a resource from which the present resource
     * is derived.
     * The present resource may be derived from the Source resource
     * in whole or in part.  Recommended best practice is to reference
     * the resource by means of a string or number conforming to a
     * formal identification system.
     */
    QString source() const;

    /**
     * "The nature or genre of the content of the resource.
     * Type includes terms describing general categories, functions,
     * genres, or aggregation levels for content. Recommended best
     * practice is to select a value from a controlled vocabulary
     * (for example, the working draft list of Dublin Core Types
     * [DCT1]). To describe the physical or digital manifestation
     * of the resource, use the FORMAT element."
     */
    QString type() const;

    /**
     * returns a debug string describing the available DC metadata
     * for debugging purposes
     *
     * @return debug string
     */
    QString debugInfo() const;
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_DUBLINCORE_H

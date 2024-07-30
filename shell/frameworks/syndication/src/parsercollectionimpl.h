/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_PARSERCOLLECTIONIMPL_H
#define SYNDICATION_PARSERCOLLECTIONIMPL_H

#include <syndication/abstractparser.h>
#include <syndication/documentsource.h>
#include <syndication/feed.h>
#include <syndication/global.h>
#include <syndication/mapper.h>
#include <syndication/parsercollection.h>
#include <syndication/specificdocument.h>

#include <QDomDocument>
#include <QHash>
#include <QString>

namespace Syndication
{
//@cond PRIVATE
/** @internal
 */
// default implementation of ParserCollection. This is separated
// from the interface to move the implementation out of the public API
// (template classes require implementations to be in the header)

template<class T>
class SYNDICATION_EXPORT ParserCollectionImpl : public ParserCollection<T>
{
public:
    ParserCollectionImpl();

    ~ParserCollectionImpl() override;

    QSharedPointer<T> parse(const DocumentSource &source, const QString &formatHint = QString()) override;

    bool registerParser(AbstractParser *parser, Mapper<T> *mapper) override;

    void changeMapper(const QString &format, Mapper<T> *mapper) override;

    ErrorCode lastError() const override;

private:
    ParserCollectionImpl(const ParserCollectionImpl &);
    ParserCollectionImpl &operator=(const ParserCollectionImpl &);
    QHash<QString, AbstractParser *> m_parsers;
    QHash<QString, Mapper<T> *> m_mappers;
    QList<AbstractParser *> m_parserList;

    ErrorCode m_lastError;
};

//@endcond

// template <class T>
// class ParserCollectionImpl<T>::ParserCollectionImplPrivate

template<class T>
ParserCollectionImpl<T>::ParserCollectionImpl()
{
}

template<class T>
ParserCollectionImpl<T>::~ParserCollectionImpl()
{
    // Delete the values
    qDeleteAll(m_parsers);
    qDeleteAll(m_mappers);
}

template<class T>
bool ParserCollectionImpl<T>::registerParser(AbstractParser *parser, Mapper<T> *mapper)
{
    if (m_parsers.contains(parser->format())) {
        return false;
    }

    m_parserList.append(parser);
    m_parsers.insert(parser->format(), parser);
    m_mappers.insert(parser->format(), mapper);
    return true;
}
template<class T>
void ParserCollectionImpl<T>::changeMapper(const QString &format, Mapper<T> *mapper)
{
    m_mappers[format] = mapper;
}

template<class T>
QSharedPointer<T> ParserCollectionImpl<T>::parse(const DocumentSource &source, const QString &formatHint)
{
    m_lastError = Syndication::Success;

    if (!formatHint.isNull() && m_parsers.contains(formatHint)) {
        if (m_parsers[formatHint]->accept(source)) {
            SpecificDocumentPtr doc = m_parsers[formatHint]->parse(source);
            if (!doc->isValid()) {
                m_lastError = InvalidFormat;
                return FeedPtr();
            }

            return m_mappers[formatHint]->map(doc);
        }
    }

    for (AbstractParser *i : std::as_const(m_parserList)) {
        if (i->accept(source)) {
            SpecificDocumentPtr doc = i->parse(source);
            if (!doc->isValid()) {
                m_lastError = InvalidFormat;
                return FeedPtr();
            }

            return m_mappers[i->format()]->map(doc);
        }
    }
    if (source.asDomDocument().isNull()) {
        m_lastError = InvalidXml;
    } else {
        m_lastError = XmlNotAccepted;
    }

    return FeedPtr();
}

template<class T>
Syndication::ErrorCode ParserCollectionImpl<T>::lastError() const
{
    return m_lastError;
}

template<class T>
ParserCollectionImpl<T>::ParserCollectionImpl(const ParserCollectionImpl &)
{
}

template<class T>
ParserCollectionImpl<T> &ParserCollectionImpl<T>::operator=(const ParserCollectionImpl &)
{
    return *this;
}

} // namespace Syndication

#endif // SYNDICATION_PARSERCOLLECTIONIMPL_H

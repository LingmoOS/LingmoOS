/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "modelmaker.h"
#include "literal.h"
#include "model.h"
#include "property.h"
#include "rdfvocab.h"
#include "resource.h"
#include "sequence.h"
#include "statement.h"

#include <QDomElement>
#include <QString>

namespace Syndication
{
namespace RDF
{
Model ModelMaker::createFromXML(const QDomDocument &doc)
{
    Model model;

    if (doc.isNull()) {
        return model;
    }

    const QDomElement rdfNode = doc.documentElement();

    const QDomNodeList list = rdfNode.childNodes();

    for (int i = 0; i < list.length(); ++i) {
        if (list.item(i).isElement()) {
            const QDomElement el = list.item(i).toElement();
            readResource(model, el);
        }
    }

    return model;
}

ResourcePtr ModelMaker::readResource(Model &model, const QDomElement &el)
{
    QString about = QStringLiteral("about");
    QString resource = QStringLiteral("resource");

    ResourcePtr res;

    ResourcePtr type = model.createResource(el.namespaceURI() + el.localName());

    if (*type == *(RDFVocab::self()->seq())) {
        SequencePtr seq = model.createSequence(el.attribute(about));

        res = seq;
    } else {
        res = model.createResource(el.attribute(about));
    }

    model.addStatement(res, RDFVocab::self()->type(), type);

    const QDomNodeList children = el.childNodes();

    bool isSeq = res->isSequence();

    for (int i = 0; i < children.length(); ++i) {
        if (children.item(i).isElement()) {
            QDomElement ce = children.item(i).toElement();

            PropertyPtr pred = model.createProperty(ce.namespaceURI() + ce.localName());

            if (ce.hasAttribute(resource)) { // referenced Resource via (rdf:)resource
                NodePtr obj = model.createResource(ce.attribute(resource));

                if (isSeq && *pred == *(RDFVocab::self()->li())) {
                    SequencePtr tseq = res.staticCast<Sequence>();
                    tseq->append(obj);
                } else {
                    model.addStatement(res, pred, obj);
                }
            } else if (!ce.text().isEmpty() && ce.lastChildElement().isNull()) { // Literal
                NodePtr obj = model.createLiteral(ce.text());

                if (isSeq && *pred == *(RDFVocab::self()->li())) {
                    SequencePtr tseq = res.staticCast<Sequence>();
                    tseq->append(obj);
                } else {
                    model.addStatement(res, pred, obj);
                }
            } else { // embedded description
                QDomElement re = ce.lastChildElement();

                // QString uri = re.attribute(about);

                // read recursively
                NodePtr obj = readResource(model, re);

                if (isSeq && *pred == *(RDFVocab::self()->li())) {
                    SequencePtr tseq = res.staticCast<Sequence>();
                    tseq->append(obj);
                } else {
                    model.addStatement(res, pred, obj);
                }
            }

            // TODO: bag, reification (nice to have, but not important for basic RSS 1.0)
        }
    }

    return res;
}

} // namespace RDF
} // namespace Syndication

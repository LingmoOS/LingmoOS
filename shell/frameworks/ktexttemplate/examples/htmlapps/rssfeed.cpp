/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "rssfeed.h"

#include <KTextTemplate/Util>

#include <QAbstractXmlNodeModel>
#include <QBuffer>
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QXmlNodeModelIndex>
#include <QXmlQuery>
#include <QXmlResultItems>

Q_DECLARE_METATYPE(QXmlQuery)

RssFeedNodeFactory::RssFeedNodeFactory(QObject *parent)
    : KTextTemplate::AbstractNodeFactory(parent)
{
}

KTextTemplate::Node *RssFeedNodeFactory::getNode(const QString &tagContent, KTextTemplate::Parser *p) const
{
    QStringList expr = smartSplit(tagContent);
    KTextTemplate::FilterExpression url(expr.at(1), p);
    KTextTemplate::FilterExpression query(expr.at(2), p);

    RssFeedNode *n = new RssFeedNode(url, query);

    QList<KTextTemplate::Node *> nodes = p->parse(n, "endrssfeed");
    p->takeNextToken();

    n->setChildNodes(nodes);

    return n;
}

RssFeedNode::RssFeedNode(const KTextTemplate::FilterExpression &url, const KTextTemplate::FilterExpression &query, QObject *parent)
    : KTextTemplate::Node(parent)
    , m_url(url)
    , m_query(query)
{
}

void RssFeedNode::setChildNodes(QList<KTextTemplate::Node *> childNodes)
{
    m_childNodes = childNodes;
}

void RssFeedNode::render(KTextTemplate::OutputStream *stream, KTextTemplate::Context *c) const
{
    QNetworkAccessManager *mgr = new QNetworkAccessManager(const_cast<RssFeedNode *>(this));
    QUrl url(KTextTemplate::getSafeString(m_url.resolve(c)));
    QNetworkReply *reply = mgr->get(QNetworkRequest(url));
    QEventLoop eLoop;
    connect(mgr, SIGNAL(finished(QNetworkReply *)), &eLoop, SLOT(quit()));
    eLoop.exec(QEventLoop::ExcludeUserInputEvents);

    c->push();
    foreach (KTextTemplate::Node *n, m_childNodes) {
        if (!n->inherits(XmlNamespaceNode::staticMetaObject.className()))
            continue;
        KTextTemplate::OutputStream _dummy;
        n->render(&_dummy, c);
    }

    QXmlQuery query;
    QByteArray ba = reply->readAll();

    QBuffer buffer;
    buffer.setData(ba);
    buffer.open(QIODevice::ReadOnly);
    query.bindVariable("inputDocument", &buffer);
    QString ns;
    QHash<QString, QVariant> h = c->lookup("_ns").toHash();
    QHash<QString, QVariant>::const_iterator it = h.constBegin();
    const QHash<QString, QVariant>::const_iterator end = h.constEnd();
    for (; it != end; ++it) {
        if (it.key().isEmpty()) {
            ns += QStringLiteral("declare default element namespace ") + QLatin1String(" \"") + it.value().toString() + QLatin1String("\";\n");
        } else {
            ns += QStringLiteral("declare namespace ") + it.key() + QLatin1String(" = \"") + it.value().toString() + QLatin1String("\";\n");
        }
    }
    query.setQuery(ns + "doc($inputDocument)" + KTextTemplate::getSafeString(m_query.resolve(c)).get());

    QXmlResultItems result;
    query.evaluateTo(&result);

    QXmlItem item(result.next());
    int count = 0;
    while (!item.isNull()) {
        if (count++ > 20)
            break;
        query.setFocus(item);
        c->push();
        foreach (KTextTemplate::Node *n, m_childNodes) {
            if (n->inherits(XmlNamespaceNode::staticMetaObject.className()))
                continue;
            c->insert("_q", QVariant::fromValue(query));
            n->render(stream, c);
        }
        c->pop();
        item = result.next();
    }
    c->pop();
}

XmlRoleNodeFactory::XmlRoleNodeFactory(QObject *parent)
{
}

KTextTemplate::Node *XmlRoleNodeFactory::getNode(const QString &tagContent, KTextTemplate::Parser *p) const
{
    QStringList expr = smartSplit(tagContent);
    KTextTemplate::FilterExpression query(expr.at(1), p);
    return new XmlRoleNode(query);
}

XmlRoleNode::XmlRoleNode(const KTextTemplate::FilterExpression &query, QObject *parent)
    : m_query(query)
    , m_count(0)
{
}

static QString unescape(const QString &_input)
{
    QString input = _input;
    input.replace("&lt;", "<");
    input.replace("&gt;", ">");
    input.replace("&quot;", "\"");
    input.replace("&amp;", "&");
    return input;
}

void XmlRoleNode::render(KTextTemplate::OutputStream *stream, KTextTemplate::Context *c) const
{
    QXmlQuery q = c->lookup("_q").value<QXmlQuery>();
    QHash<QString, QVariant> h = c->lookup("_ns").toHash();
    QString ns;
    QHash<QString, QVariant>::const_iterator it = h.constBegin();
    const QHash<QString, QVariant>::const_iterator end = h.constEnd();
    for (; it != end; ++it) {
        if (it.key().isEmpty()) {
            ns += QStringLiteral("declare default element namespace ") + QLatin1String(" \"") + it.value().toString() + QLatin1String("\";\n");
        } else {
            ns += QStringLiteral("declare namespace ") + it.key() + QLatin1String(" = \"") + it.value().toString() + QLatin1String("\";\n");
        }
    }
    q.setQuery(ns + KTextTemplate::getSafeString(m_query.resolve(c)));
    QString s;
    q.evaluateTo(&s);
    (*stream) << unescape(s);
}

XmlNamespaceNodeFactory::XmlNamespaceNodeFactory(QObject *parent)
{
}

KTextTemplate::Node *XmlNamespaceNodeFactory::getNode(const QString &tagContent, KTextTemplate::Parser *p) const
{
    QStringList expr = smartSplit(tagContent);
    KTextTemplate::FilterExpression query(expr.at(1), p);
    QString name;
    if (expr.size() == 4)
        name = expr.at(3);
    return new XmlNamespaceNode(query, name);
}

XmlNamespaceNode::XmlNamespaceNode(const KTextTemplate::FilterExpression &query, const QString &name, QObject *parent)
    : m_query(query)
    , m_name(name)
{
}

void XmlNamespaceNode::render(KTextTemplate::OutputStream *stream, KTextTemplate::Context *c) const
{
    QString q = KTextTemplate::getSafeString(m_query.resolve(c));
    QHash<QString, QVariant> h = c->lookup("_ns").toHash();
    h.insert(m_name, q);
    c->insert("_ns", h);
}

QVariant ResizeFilter::doFilter(const QVariant &input, const QVariant &argument, bool autoescape) const
{
    QString url = KTextTemplate::getSafeString(input);
    url.replace("_s", "_z");
    return url;
}

#include "moc_rssfeed.cpp"

/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef RSS_FEED_H
#define RSS_FEED_H

#include <KTextTemplate/Filter>
#include <KTextTemplate/Node>

namespace KTextTemplate
{
class Parser;
class OutputStream;
class Context;
}

class RssFeedNodeFactory : public KTextTemplate::AbstractNodeFactory
{
    Q_OBJECT
public:
    RssFeedNodeFactory(QObject *parent = 0);

    virtual KTextTemplate::Node *getNode(const QString &tagContent, KTextTemplate::Parser *p) const;
};

class RssFeedNode : public KTextTemplate::Node
{
    Q_OBJECT
public:
    RssFeedNode(const KTextTemplate::FilterExpression &url, const KTextTemplate::FilterExpression &query, QObject *parent = 0);

    void setChildNodes(QList<Node *> childNodes);

    virtual void render(KTextTemplate::OutputStream *stream, KTextTemplate::Context *c) const;

private:
    KTextTemplate::FilterExpression m_url;
    KTextTemplate::FilterExpression m_query;
    QList<Node *> m_childNodes;
};

class XmlRoleNodeFactory : public KTextTemplate::AbstractNodeFactory
{
    Q_OBJECT
public:
    XmlRoleNodeFactory(QObject *parent = 0);

    virtual KTextTemplate::Node *getNode(const QString &tagContent, KTextTemplate::Parser *p) const;
};

class XmlRoleNode : public KTextTemplate::Node
{
    Q_OBJECT
public:
    XmlRoleNode(const KTextTemplate::FilterExpression &query, QObject *parent = 0);

    virtual void render(KTextTemplate::OutputStream *stream, KTextTemplate::Context *c) const;

private:
    KTextTemplate::FilterExpression m_name;
    KTextTemplate::FilterExpression m_query;
    int m_count;
};

class XmlNamespaceNodeFactory : public KTextTemplate::AbstractNodeFactory
{
    Q_OBJECT
public:
    XmlNamespaceNodeFactory(QObject *parent = 0);

    virtual KTextTemplate::Node *getNode(const QString &tagContent, KTextTemplate::Parser *p) const;
};

class XmlNamespaceNode : public KTextTemplate::Node
{
    Q_OBJECT
public:
    XmlNamespaceNode(const KTextTemplate::FilterExpression &query, const QString &name, QObject *parent = 0);

    virtual void render(KTextTemplate::OutputStream *stream, KTextTemplate::Context *c) const;

private:
    QString m_name;
    KTextTemplate::FilterExpression m_query;
};

class ResizeFilter : public KTextTemplate::Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = false) const;
};

#endif

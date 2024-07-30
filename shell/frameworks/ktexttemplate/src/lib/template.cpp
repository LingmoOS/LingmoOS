/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "template.h"
#include "template_p.h"

#include "context.h"
#include "engine.h"
#include "lexer_p.h"
#include "parser.h"
#include "rendercontext.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(KTEXTTEMPLATE_TEMPLATE, "kf.texttemplate")

using namespace KTextTemplate;

NodeList TemplatePrivate::compileString(const QString &str)
{
    Q_Q(TemplateImpl);
    Lexer l(str);
    Parser p(l.tokenize(m_smartTrim ? Lexer::SmartTrim : Lexer::NoSmartTrim), q);

    return p.parse(q);
}

TemplateImpl::TemplateImpl(Engine const *engine, QObject *parent)
    : QObject(parent)
    , d_ptr(new TemplatePrivate(engine, false, this))
{
}

TemplateImpl::TemplateImpl(Engine const *engine, bool smartTrim, QObject *parent)
    : QObject(parent)
    , d_ptr(new TemplatePrivate(engine, smartTrim, this))
{
}

TemplateImpl::~TemplateImpl()
{
    delete d_ptr;
}

void TemplateImpl::setContent(const QString &templateString)
{
    Q_D(Template);
    if (templateString.isEmpty())
        return;

    try {
        d->m_nodeList = d->compileString(templateString);
        d->setError(NoError, QString());
    } catch (KTextTemplate::Exception &e) {
        qCWarning(KTEXTTEMPLATE_TEMPLATE) << e.what();
        d->setError(e.errorCode(), e.what());
    }
}

QString TemplateImpl::render(Context *c) const
{
    QString output;
    QTextStream textStream(&output);
    OutputStream outputStream(&textStream);
    render(&outputStream, c);
    return output;
}

OutputStream *TemplateImpl::render(OutputStream *stream, Context *c) const
{
    Q_D(const Template);

    c->clearExternalMedia();

    c->renderContext()->push();

    try {
        d->m_nodeList.render(stream, c);
        d->setError(NoError, QString());
    } catch (KTextTemplate::Exception &e) {
        qCWarning(KTEXTTEMPLATE_TEMPLATE) << e.what();
        d->setError(e.errorCode(), e.what());
    }

    c->renderContext()->pop();

    return stream;
}

NodeList TemplateImpl::nodeList() const
{
    Q_D(const Template);
    return d->m_nodeList;
}

void TemplateImpl::setNodeList(const NodeList &list)
{
    Q_D(Template);
    d->m_nodeList = list;
}

void TemplatePrivate::setError(Error type, const QString &message) const
{
    m_error = type;
    m_errorString = message;
}

Error TemplateImpl::error() const
{
    Q_D(const Template);
    return d->m_error;
}

QString TemplateImpl::errorString() const
{
    Q_D(const Template);
    return d->m_errorString;
}

Engine const *TemplateImpl::engine() const
{
    Q_D(const Template);
    return d->m_engine.data();
}

#include "moc_template.cpp"

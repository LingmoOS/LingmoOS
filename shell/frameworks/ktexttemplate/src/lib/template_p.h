/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_TEMPLATE_P_H
#define KTEXTTEMPLATE_TEMPLATE_P_H

#include "engine.h"
#include "template.h"

#include <QPointer>

namespace KTextTemplate
{

class Engine;

class TemplatePrivate
{
    TemplatePrivate(Engine const *engine, bool smartTrim, TemplateImpl *t)
        : q_ptr(t)
        , m_error(NoError)
        , m_smartTrim(smartTrim)
        , m_engine(engine)
    {
    }

    void parse();
    NodeList compileString(const QString &str);
    void setError(Error type, const QString &message) const;

    Q_DECLARE_PUBLIC(TemplateImpl)
    TemplateImpl *const q_ptr;

    mutable Error m_error;
    mutable QString m_errorString;
    NodeList m_nodeList;
    bool m_smartTrim;
    QPointer<const Engine> m_engine;

    friend class KTextTemplate::Engine;
    friend class Parser;
};
}

#endif

/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef SCRIPTABLE_FILTER_H
#define SCRIPTABLE_FILTER_H

#include <QJSValue>

#include "filter.h"

using namespace KTextTemplate;

class ScriptableFilter : public Filter
{
public:
    ScriptableFilter(const QJSValue &filterObject, QJSEngine *engine);
    ~ScriptableFilter() override;

    QVariant doFilter(const QVariant &input, const QVariant &argument, bool autoescape = {}) const override;

    bool isSafe() const override;

private:
    QJSValue m_filterObject;
    QJSEngine *m_scriptEngine;
};

#endif

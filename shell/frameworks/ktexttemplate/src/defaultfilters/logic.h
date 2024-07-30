/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef LOGICFILTERS_H
#define LOGICFILTERS_H

#include "filter.h"

using namespace KTextTemplate;

class DefaultFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class DefaultIfNoneFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class DivisibleByFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class YesNoFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

#endif

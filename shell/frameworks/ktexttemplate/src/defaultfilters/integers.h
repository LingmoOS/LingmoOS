/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef INTEGERFILTERS_H
#define INTEGERFILTERS_H

// krazy:excludeall=includes

#include "filter.h"

using namespace KTextTemplate;

class AddFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class GetDigitFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

#endif

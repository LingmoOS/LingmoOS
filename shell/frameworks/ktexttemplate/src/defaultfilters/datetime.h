/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef DATETIMEFILTERS_H
#define DATETIMEFILTERS_H

#include "filter.h"

using namespace KTextTemplate;

class DateFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class TimeFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class TimeSinceFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class TimeUntilFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

#endif

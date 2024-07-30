/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010, 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef LISTFILTERS_H
#define LISTFILTERS_H

// krazy:excludeall=includes

#include "filter.h"

using namespace KTextTemplate;

class JoinFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class LengthFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class LengthIsFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class FirstFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class LastFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class RandomFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class SliceFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class MakeListFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class UnorderedListFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }

protected:
    SafeString processList(const QVariantList &list, int tabs, bool autoescape) const;
};

class DictSortFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return false;
    }
};

#endif

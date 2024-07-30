/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef STRINGFILTERS_H
#define STRINGFILTERS_H

// krazy:excludeall=includes

#include "filter.h"

using namespace KTextTemplate;

class AddSlashesFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class CapFirstFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class EscapeJsFilter : public Filter
{
public:
    EscapeJsFilter();

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

private:
    QList<std::pair<QString, QString>> m_jsEscapes;
};

class FixAmpersandsFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class CutFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class SafeFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class LineNumbersFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class LowerFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class StringFormatFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class TitleFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class TruncateWordsFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class UpperFilter : public Filter
{
public:
    // &amp; may be safe, but it will be changed to &AMP; which is not safe.
    bool isSafe() const override
    {
        return false;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class WordCountFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class LJustFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class RJustFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class CenterFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class EscapeFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class ForceEscapeFilter : public Filter
{
public:
    bool isSafe() const override
    {
        return true;
    }

    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class RemoveTagsFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class StripTagsFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;
};

class WordWrapFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class FloatFormatFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class SafeSequenceFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class LineBreaksFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class LineBreaksBrFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class SlugifyFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = {}, bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class FileSizeFormatFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

class TruncateCharsFilter : public Filter
{
public:
    QVariant doFilter(const QVariant &input, const QVariant &argument = QVariant(), bool autoescape = {}) const override;

    bool isSafe() const override
    {
        return true;
    }
};

#endif

/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "filter.h"

using namespace KTextTemplate;

Filter::~Filter() = default;

void Filter::setStream(KTextTemplate::OutputStream *stream)
{
    m_stream = stream;
}

SafeString Filter::escape(const QString &input) const
{
    return m_stream->escape(input);
}

SafeString Filter::escape(const SafeString &input) const
{
    if (input.isSafe())
        return {m_stream->escape(input), SafeString::IsSafe};
    return m_stream->escape(input);
}

SafeString Filter::conditionalEscape(const SafeString &input) const
{
    if (!input.isSafe())
        return m_stream->escape(input);
    return input;
}

bool Filter::isSafe() const
{
    return false;
}

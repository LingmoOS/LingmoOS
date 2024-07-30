/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "outputstream.h"

#include "safestring.h"

using namespace KTextTemplate;

OutputStream::OutputStream()
    : m_stream(nullptr)
{
}

OutputStream::OutputStream(QTextStream *stream)
    : m_stream(stream)
{
}

OutputStream::~OutputStream() = default;

QString OutputStream::escape(const QString &input) const
{
    // This could be replaced by QString::toHtmlEscaped()
    // but atm it does not escape single quotes
    QString rich;
    const int len = input.length();
    rich.reserve(int(len * 1.1));
    for (int i = 0; i < len; ++i) {
        const QChar ch = input.at(i);
        if (ch == QLatin1Char('<'))
            rich += QLatin1String("&lt;");
        else if (ch == QLatin1Char('>'))
            rich += QLatin1String("&gt;");
        else if (ch == QLatin1Char('&'))
            rich += QLatin1String("&amp;");
        else if (ch == QLatin1Char('"'))
            rich += QLatin1String("&quot;");
        else if (ch == QLatin1Char('\''))
            rich += QLatin1String("&#39;");
        else
            rich += ch;
    }
    rich.squeeze();
    return rich;
}

QString OutputStream::escape(const KTextTemplate::SafeString &input) const
{
    return escape(input.get());
}

QString OutputStream::conditionalEscape(const KTextTemplate::SafeString &input) const
{
    if (!input.isSafe())
        return escape(input.get());
    return input;
}

QSharedPointer<OutputStream> OutputStream::clone(QTextStream *stream) const
{
    return QSharedPointer<OutputStream>(new OutputStream(stream));
}

OutputStream &OutputStream::operator<<(const QString &input)
{
    if (m_stream)
        (*m_stream) << input;
    return *this;
}

OutputStream &OutputStream::operator<<(const KTextTemplate::SafeString &input)
{
    if (m_stream) {
        if (input.needsEscape())
            (*m_stream) << escape(input.get());
        else
            (*m_stream) << input.get();
    }
    return *this;
}
/*
OutputStream& OutputStream::operator<<(const
KTextTemplate::OutputStream::Escape& e)
{
  ( *m_stream ) << escape( e.m_content );
  return *this;
}*/

OutputStream &OutputStream::operator<<(QTextStream *stream)
{
    if (m_stream)
        (*m_stream) << stream->readAll();
    return *this;
}
/*
KTextTemplate::OutputStream::MarkSafe::MarkSafe(const QString& input)
  : m_safe( false ), m_content( input )
{

}

KTextTemplate::OutputStream::MarkSafe::MarkSafe(const KTextTemplate::SafeString&
input) : m_safe( input.isSafe() ), m_content( input.get() )
{

}
*/

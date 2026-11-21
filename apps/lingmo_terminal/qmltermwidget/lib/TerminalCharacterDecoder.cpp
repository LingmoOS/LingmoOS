/*
    This file is part of Konsole, an X terminal.

    Copyright 2006-2008 by Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "TerminalCharacterDecoder.h"

// stdlib
#include <cwctype>

#include <cstdio>

#include "tools.h"

// KDE
//#include <kdebug.h>

// Konsole
#include "konsole_wcwidth.h"

#include <cwctype>

using namespace Konsole;

void TerminalCharacterDecoder::appendWide(std::u16string& output, const std::wstring& text)
{
    // A byte order mark in front is dropped, as Qt's UTF-32 decoder drops it.
    const size_t first = !text.empty() && char32_t(text[0]) == 0xfeff ? 1 : 0;
    for (size_t i = first; i < text.size(); ++i)
    {
        const char32_t ucs4 = char32_t(text[i]);
        if (ucs4 < 0x10000)
        {
            output.push_back(char16_t(ucs4));
        }
        else
        {
            output.push_back(char16_t((ucs4 >> 10) + 0xd7c0));
            output.push_back(char16_t(ucs4 % 0x400 + 0xdc00));
        }
    }
}
PlainTextDecoder::PlainTextDecoder()
 : _output(nullptr)
 , _includeTrailingWhitespace(true)
 , _recordLinePositions(false)
{

}
void PlainTextDecoder::setTrailingWhitespace(bool enable)
{
    _includeTrailingWhitespace = enable;
}
bool PlainTextDecoder::trailingWhitespace() const
{
    return _includeTrailingWhitespace;
}
void PlainTextDecoder::begin(std::u16string* output)
{
   _output = output;
   if (!_linePositions.empty())
       _linePositions.clear();
}
void PlainTextDecoder::end()
{
    _output = nullptr;
}

void PlainTextDecoder::setRecordLinePositions(bool record)
{
    _recordLinePositions = record;
}
std::vector<int> PlainTextDecoder::linePositions() const
{
    return _linePositions;
}
void PlainTextDecoder::decodeLine(const Character* const characters, int count, LineProperty /*properties*/)
{
    assert( _output );

    if (_recordLinePositions)
    {
        int pos = int(_output->size());
        _linePositions.push_back(pos);
    }

    if (characters == nullptr)
    {
        // TODO: So far, this has happened only under kwin_wayland, when the current function
        // is called by TerminalDisplay::inputMethodQuery(). The actual issue should be found.
        return;
    }

    //TODO should we ignore or respect the LINE_WRAPPED line property?

    //note:  we build up a QString and send it to the text stream rather writing into the text
    //stream a character at a time because it is more efficient.
    //(since QTextStream always deals with QStrings internally anyway)
    std::wstring plainText;
    plainText.reserve(count);

    int outputCount = count;

    // if inclusion of trailing whitespace is disabled then find the end of the
    // line
    if ( !_includeTrailingWhitespace )
    {
        for (int i = count-1 ; i >= 0 ; i--)
        {
            if (!characters[i].isSpace())
                break;
            else
                outputCount--;
        }
    }

    for (int i = 0; i < outputCount;)
    {
        if (characters[i].rendition & RE_EXTENDED_CHAR)
        {
            ushort extendedCharLength = 0;
            const uint* chars = ExtendedCharTable::instance.lookupExtendedChar(characters[i].character, extendedCharLength);
            if (chars)
            {
                std::wstring str;
                for (ushort nchar = 0; nchar < extendedCharLength; nchar++)
                {
                    str.push_back(chars[nchar]);
                }
                plainText += str;
                i += kMax(1, string_width(str));
            }
            else
            {
                ++i;
            }
        }
        else
        {
            plainText.push_back(characters[i].character);
            i += kMax(1, konsole_wcwidth(characters[i].character));
        }
    }
    appendWide(*_output, plainText);
}

// QColor::name(): #rrggbb in lower case.
static std::string colorName(const Color& color)
{
    char name[8];
    std::snprintf(name, sizeof(name), "#%02x%02x%02x", color.r, color.g, color.b);
    return name;
}

HTMLDecoder::HTMLDecoder() :
        _output(nullptr)
    ,_colorTable(base_color_table)
       ,_innerSpanOpen(false)
       ,_lastRendition(DEFAULT_RENDITION)
{

}

void HTMLDecoder::begin(std::u16string* output)
{
    _output = output;

    std::wstring text;

    //open monospace span
    openSpan(text,"font-family:monospace");

    appendWide(*output, text);
}

void HTMLDecoder::end()
{
    assert( _output );

    std::wstring text;

    closeSpan(text);

    appendWide(*_output, text);

    _output = nullptr;

}

//TODO: Support for LineProperty (mainly double width , double height)
void HTMLDecoder::decodeLine(const Character* const characters, int count, LineProperty /*properties*/
                            )
{
    assert( _output );

    std::wstring text;

    int spaceCount = 0;

    for (int i=0;i<count;i++)
    {
        //check if appearance of character is different from previous char
        if ( characters[i].rendition != _lastRendition  ||
             characters[i].foregroundColor != _lastForeColor  ||
             characters[i].backgroundColor != _lastBackColor )
        {
            if ( _innerSpanOpen )
                    closeSpan(text);

            _lastRendition = characters[i].rendition;
            _lastForeColor = characters[i].foregroundColor;
            _lastBackColor = characters[i].backgroundColor;

            //build up style string
            std::string style;

            bool useBold;
            ColorEntry::FontWeight weight = characters[i].fontWeight(_colorTable);
            if (weight == ColorEntry::UseCurrentFormat)
                useBold = _lastRendition & RE_BOLD;
            else
                useBold = weight == ColorEntry::Bold;

            if (useBold)
                style.append("font-weight:bold;");

            if ( _lastRendition & RE_UNDERLINE )
                    style.append("font-decoration:underline;");

            //colours - a colour table must have been defined first
            if ( _colorTable )
            {
                style.append( "color:" + colorName(_lastForeColor.color(_colorTable)) + ";" );

                if (!characters[i].isTransparent(_colorTable))
                {
                    style.append( "background-color:" + colorName(_lastBackColor.color(_colorTable)) + ";" );
                }
            }

            //open the span with the current style
            openSpan(text,style);
            _innerSpanOpen = true;
        }

        //handle whitespace
        if (characters[i].isSpace())
            spaceCount++;
        else
            spaceCount = 0;


        //output current character
        if (spaceCount < 2)
        {
            if (characters[i].rendition & RE_EXTENDED_CHAR)
            {
                ushort extendedCharLength = 0;
                const uint* chars = ExtendedCharTable::instance.lookupExtendedChar(characters[i].character, extendedCharLength);
                if (chars)
                {
                    for (ushort nchar = 0; nchar < extendedCharLength; nchar++)
                    {
                        text.push_back(chars[nchar]);
                    }
                }
            }
            else
            {
                //escape HTML tag characters and just display others as they are
                wchar_t ch(characters[i].character);
                if ( ch == '<' )
                {
                    text.append(L"&lt;");
                }
                else if (ch == '>')
                {
                    text.append(L"&gt;");
                }
                else if (ch == '&')
                {
                    text.append(L"&amp;");
                }
                else
                {
                    text.push_back(ch);
                }
            }
        }
        else
        {
            // HTML truncates multiple spaces, so use a space marker instead
            // Use &#160 instead of &nbsp so xmllint will work.
            text.append(L"&#160;");
        }

    }

    //close any remaining open inner spans
    if ( _innerSpanOpen )
        closeSpan(text);

    //start new line
    text.append(L"<br>");

    appendWide(*_output, text);
}
void HTMLDecoder::openSpan(std::wstring& text , const std::string& style)
{
    const std::string span = "<span style=\"" + style + "\">";
    text.append(span.begin(), span.end());
}

void HTMLDecoder::closeSpan(std::wstring& text)
{
    text.append(L"</span>");
}

void HTMLDecoder::setColorTable(const ColorEntry* table)
{
    _colorTable = table;
}

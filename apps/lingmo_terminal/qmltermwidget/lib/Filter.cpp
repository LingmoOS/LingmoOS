/*
    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "Filter.h"

// System
#include <algorithm>
#include <iostream>

#include "base/strings.h"

// Konsole
#include "TerminalCharacterDecoder.h"
#include "konsole_wcwidth.h"

using namespace Konsole;
namespace strings = Lingmo::base::strings;

FilterChain::~FilterChain()
{
    for (Filter* filter : *this)
        delete filter;
    std::vector<Filter*>::clear();
}

RegExpFilter* FilterChain::getRegExpFilter() const
{
    for (Filter* filter : *this)
    {
        if (auto f = dynamic_cast<RegExpFilter*>(filter))
        {
            if (dynamic_cast<UrlFilter*>(filter) == nullptr)
            {
                return f;
            }
        }
    }
    return nullptr;
}

void FilterChain::addFilter(Filter* filter)
{
    push_back(filter);
}
void FilterChain::removeFilter(Filter* filter)
{
    std::erase(*this, filter);
}
bool FilterChain::containsFilter(Filter* filter)
{
    return std::find(begin(), end(), filter) != end();
}
void FilterChain::reset()
{
    for (Filter* filter : *this)
        filter->reset();
}
void FilterChain::setBuffer(const std::u16string* buffer , const std::vector<int>* linePositions)
{
    for (Filter* filter : *this)
        filter->setBuffer(buffer,linePositions);
}
void FilterChain::process()
{
    for (Filter* filter : *this)
        filter->process();
}
void FilterChain::clear()
{
    std::vector<Filter*>::clear();
}
Filter::HotSpot* FilterChain::hotSpotAt(int line , int column) const
{
    for (Filter* filter : *this)
    {
        Filter::HotSpot* spot = filter->hotSpotAt(line,column);
        if ( spot != nullptr )
        {
            return spot;
        }
    }

    return nullptr;
}

std::vector<Filter::HotSpot*> FilterChain::hotSpots() const
{
    std::vector<Filter::HotSpot*> list;
    for (Filter* filter : *this)
    {
        const std::vector<Filter::HotSpot*> spots = filter->hotSpots();
        list.insert(list.end(), spots.begin(), spots.end());
    }
    return list;
}

TerminalImageFilterChain::TerminalImageFilterChain()
: _buffer(nullptr)
, _linePositions(nullptr)
{
}

TerminalImageFilterChain::~TerminalImageFilterChain()
{
    delete _buffer;
    delete _linePositions;
}

void TerminalImageFilterChain::setImage(const Character* const image , int lines , int columns, const std::vector<LineProperty>& lineProperties)
{
    if (empty())
        return;

    // reset all filters and hotspots
    reset();

    PlainTextDecoder decoder;
    // Include trailing whitespace because otherwise, if a string is wrapped at
    // the end of a space, that space will not be taken into account in _buffer.
    decoder.setTrailingWhitespace(true);

    // setup new shared buffers for the filters to process on
    std::u16string* newBuffer = new std::u16string();
    std::vector<int>* newLinePositions = new std::vector<int>();
    setBuffer( newBuffer , newLinePositions );

    // free the old buffers
    delete _buffer;
    delete _linePositions;

    _buffer = newBuffer;
    _linePositions = newLinePositions;

    decoder.begin(_buffer);

    for (int i=0 ; i < lines ; i++)
    {
        _linePositions->push_back(int(_buffer->length()));
        decoder.decodeLine(image + i*columns,columns,LINE_DEFAULT);

        // pretend that each line ends with a newline character.
        // this prevents a link that occurs at the end of one line
        // being treated as part of a link that occurs at the start of the next line
        //
        // the downside is that links which are spread over more than one line are not
        // highlighted.
        //
        // TODO - Use the "line wrapped" attribute associated with lines in a
        // terminal image to avoid adding this imaginary character for wrapped
        // lines
        const LineProperty property = i < int(lineProperties.size()) ? lineProperties[i] : LineProperty(LINE_DEFAULT);
        if ( !(property & LINE_WRAPPED) )
            _buffer->push_back(u'\n');
    }
    decoder.end();
}

Filter::Filter() :
_linePositions(nullptr),
_buffer(nullptr)
{
}

Filter::~Filter()
{
    for (HotSpot* spot : _hotspotList)
        delete spot;
    _hotspotList.clear();
}
void Filter::reset()
{
    for (HotSpot* spot : _hotspotList)
        delete spot;
    _hotspots.clear();
    _hotspotList.clear();
}

void Filter::setBuffer(const std::u16string* buffer , const std::vector<int>* linePositions)
{
    _buffer = buffer;
    _linePositions = linePositions;
}

void Filter::getLineColumn(int position , int& startLine , int& startColumn)
{
    assert( _linePositions );
    assert( _buffer );


    for (int i = 0 ; i < int(_linePositions->size()) ; i++)
    {
        int nextLine = 0;

        if ( i == int(_linePositions->size())-1 )
            nextLine = int(_buffer->length()) + 1;
        else
            nextLine = (*_linePositions)[i+1];

        if ( (*_linePositions)[i] <= position && position < nextLine )
        {
            startLine = i;
            // QString::toStdWString(): UTF-16 to code points, a lone surrogate kept.
            const std::u16string part = buffer()->substr(size_t((*_linePositions)[i]),
                                                         size_t(position - (*_linePositions)[i]));
            std::wstring wide;
            for (size_t unit = 0; unit < part.size(); ++unit)
            {
                const char16_t current = part[unit];
                if ((current & 0xfc00) == 0xd800 && unit + 1 < part.size() && (part[unit + 1] & 0xfc00) == 0xdc00)
                {
                    wide.push_back(wchar_t((char32_t(current) << 10) + part[unit + 1] - 0x35fdc00));
                    ++unit;
                }
                else
                {
                    wide.push_back(wchar_t(current));
                }
            }
            startColumn = string_width(wide);
            return;
        }
    }
}


const std::u16string* Filter::buffer()
{
    return _buffer;
}
Filter::HotSpot::~HotSpot()
{
}
void Filter::addHotSpot(HotSpot* spot)
{
    _hotspotList.push_back(spot);

    for (int line = spot->startLine() ; line <= spot->endLine() ; line++)
    {
        _hotspots[line].push_back(spot);
    }
}
std::vector<Filter::HotSpot*> Filter::hotSpots() const
{
    return _hotspotList;
}
std::vector<Filter::HotSpot*> Filter::hotSpotsAtLine(int line) const
{
    auto found = _hotspots.find(line);
    if (found == _hotspots.end())
        return {};
    return std::vector<HotSpot*>(found->second.rbegin(), found->second.rend());
}

Filter::HotSpot* Filter::hotSpotAt(int line , int column) const
{
    for (HotSpot* spot : hotSpotsAtLine(line))
    {
        if ( spot->startLine() == line && spot->startColumn() > column )
            continue;
        if ( spot->endLine() == line && spot->endColumn() < column )
            continue;

        return spot;
    }

    return nullptr;
}

Filter::HotSpot::HotSpot(int startLine , int startColumn , int endLine , int endColumn)
    : _startLine(startLine)
    , _startColumn(startColumn)
    , _endLine(endLine)
    , _endColumn(endColumn)
    , _type(NotSpecified)
{
}
int Filter::HotSpot::startLine() const
{
    return _startLine;
}
int Filter::HotSpot::endLine() const
{
    return _endLine;
}
int Filter::HotSpot::startColumn() const
{
    return _startColumn;
}
int Filter::HotSpot::endColumn() const
{
    return _endColumn;
}
Filter::HotSpot::Type Filter::HotSpot::type() const
{
    return _type;
}
void Filter::HotSpot::setType(Type type)
{
    _type = type;
}

RegExpFilter::RegExpFilter()
{
}

RegExpFilter::HotSpot::HotSpot(int startLine,int startColumn,int endLine,int endColumn)
    : Filter::HotSpot(startLine,startColumn,endLine,endColumn)
{
    setType(Marker);
}

void RegExpFilter::HotSpot::activate(const std::string&)
{
}

void RegExpFilter::HotSpot::setCapturedTexts(const std::vector<std::u16string>& texts)
{
    _capturedTexts = texts;
}
std::vector<std::u16string> RegExpFilter::HotSpot::capturedTexts() const
{
    return _capturedTexts;
}

void RegExpFilter::setRegExp(const RegExp& regExp)
{
    _searchText = regExp;
}
RegExp RegExpFilter::regExp() const
{
    return _searchText;
}

void RegExpFilter::process()
{
    const std::u16string* text = buffer();

    assert( text );

    // ignore any regular expressions which match an empty string.
    // otherwise the while loop below will run indefinitely
    static const std::u16string emptyString;
    auto match = _searchText.match(emptyString, 0, true);
    if (match.hasMatch)
    {
        return;
    }

    match = _searchText.match(*text);
    while (match.hasMatch) {
        int startLine = 0;
        int endLine = 0;
        int startColumn = 0;
        int endColumn = 0;

        std::vector<std::u16string> captureList;
        for (int i = 0; i <= match.lastCapturedIndex(); i++) {
            captureList.push_back(match.captured(*text, i));
        }

        getLineColumn(int(match.capturedStart()), startLine, startColumn);
        getLineColumn(int(match.capturedEnd()), endLine, endColumn);

        RegExpFilter::HotSpot* spot = newHotSpot(startLine, startColumn, endLine, endColumn);
        spot->setCapturedTexts(captureList);

        addHotSpot(spot);

        // if capturedLength == 0, the program will get stuck in an infinite loop
        if (match.capturedLength() == 0) {
            break;
        }

        match = _searchText.match(*text, match.capturedEnd());
    }
}

RegExpFilter::HotSpot* RegExpFilter::newHotSpot(int startLine,int startColumn,
                                                int endLine,int endColumn)
{
    return new RegExpFilter::HotSpot(startLine,startColumn,
                                                  endLine,endColumn);
}
RegExpFilter::HotSpot* UrlFilter::newHotSpot(int startLine,int startColumn,int endLine,
                                                    int endColumn)
{
    return new UrlFilter::HotSpot(startLine,startColumn,
                                               endLine,endColumn);
}
UrlFilter::HotSpot::HotSpot(int startLine,int startColumn,int endLine,int endColumn)
: RegExpFilter::HotSpot(startLine,startColumn,endLine,endColumn)
{
    setType(Link);
}

UrlFilter::HotSpot::UrlType UrlFilter::HotSpot::urlType() const
{
    std::u16string url = capturedTexts().front();

    if ( FullUrlRegExp.match(url).hasMatch )
        return StandardUrl;
    else if ( EmailAddressRegExp.match(url).hasMatch )
        return Email;
    else
        return Unknown;
}

void UrlFilter::HotSpot::activate(const std::string&)
{
}

// QUrl's normalisation of a link before toString(): the scheme and the host
// in lower case.
static std::string normalizedUrl(const std::string& url)
{
    const size_t colon = url.find(':');
    if (colon == std::string::npos)
        return url;

    std::string result = strings::toAsciiLower(url.substr(0, colon)) + url.substr(colon);
    if (result.compare(colon, 3, "://") != 0)
        return result;

    const size_t authority = colon + 3;
    size_t end = result.find_first_of("/?#", authority);
    if (end == std::string::npos)
        end = result.size();
    size_t host = result.rfind('@', end);
    host = host == std::string::npos || host < authority ? authority : host + 1;
    size_t hostEnd = result.find(':', host);
    if (hostEnd == std::string::npos || hostEnd > end)
        hostEnd = end;
    for (size_t i = host; i < hostEnd; ++i)
        result[i] = strings::toAsciiLower(result[i]);
    return result;
}

std::string UrlFilter::HotSpot::url() const
{
    std::u16string url = capturedTexts().front();
    const UrlType kind = urlType();

    if ( kind == StandardUrl )
    {
        // if the URL path does not include the protocol ( eg. "www.kde.org" ) then
        // prepend http:// ( eg. "www.kde.org" --> "http://www.kde.org" )
        if (url.find(u"://") == std::u16string::npos)
        {
            url.insert(0, u"http://");
        }
    }
    else if ( kind == Email )
    {
        url.insert(0, u"mailto:");
    }
    else
    {
        return std::string();
    }

    return normalizedUrl(strings::utf16ToUtf8(url));
}

// Note:  Altering these regular expressions can have a major effect on the performance of the filters
// used for finding URLs in the text, especially if they are very general and could match very long
// pieces of text.
// Please be careful when altering them.

//regexp matches:
// full url:
// protocolname:// or www. followed by anything other than whitespaces, <, >, ' or ", and ends before whitespaces, <, >, ', ", ], !, comma and dot
const RegExp UrlFilter::FullUrlRegExp(u"(www\\.(?!\\.)|[a-z][a-z0-9+.-]*://)[^\\s<>'\"]+[^!,\\.\\s<>'\"\\]]");
// email address:
// [word chars, dots or dashes]@[word chars, dots or dashes].[word chars]
const RegExp UrlFilter::EmailAddressRegExp(u"\\b(\\w|\\.|-)+@(\\w|\\.|-)+\\.\\w+\\b");

// matches full url or email address
const RegExp UrlFilter::CompleteUrlRegExp(u'(' + FullUrlRegExp.pattern() + u'|' +
                                            EmailAddressRegExp.pattern() + u')');

UrlFilter::UrlFilter()
{
    setRegExp( CompleteUrlRegExp );
}

UrlFilter::HotSpot::~HotSpot()
{
}

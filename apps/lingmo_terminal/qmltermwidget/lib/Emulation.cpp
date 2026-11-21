/*
    Copyright 2007-2008 Robert Knight <robertknight@gmail.com>
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>
    Copyright 1996 by Matthias Ettrich <ettrich@kde.org>

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
#include "Emulation.h"

// System
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string>

#include <cstring>
#include <unordered_set>

// Konsole
#include "KeyboardTranslator.h"
#include "Screen.h"
#include "TerminalCharacterDecoder.h"
#include "ScreenWindow.h"
#include "tools.h"

#include "base/strings.h"

using namespace Konsole;
namespace strings = Lingmo::base::strings;

Emulation::Emulation() :
  _currentScreen(nullptr),
  _keyTranslator(nullptr),
  _usesMouse(false),
  _bracketedPasteMode(false)
{
  // create screens with a default size
  _screen[0] = new Screen(40,80);
  _screen[1] = new Screen(40,80);
  _currentScreen = _screen[0];

  _bulkTimer1.onTimeout = [this] { showBulk(); };
  _bulkTimer2.onTimeout = [this] { showBulk(); };

  // listen for mouse status changes
  _ownConnections.push_back(programUsesMouseChanged.connect([this](bool usesMouse) {
    usesMouseChanged(usesMouse);
  }));
  _ownConnections.push_back(programBracketedPasteModeChanged.connect([this](bool bracketedPasteMode) {
    bracketedPasteModeChanged(bracketedPasteMode);
  }));

  _ownConnections.push_back(cursorChanged.connect([this] (KeyboardCursorShape cursorShape, bool blinkingCursorEnabled) {
    titleChanged( 50, "CursorShape=" + std::to_string(static_cast<int>(cursorShape))
                      + ";BlinkingCursorEnabled=" + (blinkingCursorEnabled ? "1" : "0") );
  }));
}

bool Emulation::programUsesMouse() const
{
    return _usesMouse;
}

void Emulation::usesMouseChanged(bool usesMouse)
{
    _usesMouse = usesMouse;
}

bool Emulation::programBracketedPasteMode() const
{
    return _bracketedPasteMode;
}

void Emulation::bracketedPasteModeChanged(bool bracketedPasteMode)
{
    _bracketedPasteMode = bracketedPasteMode;
}

ScreenWindow* Emulation::createWindow()
{
    ScreenWindow* window = new ScreenWindow();
    window->setScreen(_currentScreen);
    _windows.push_back(window);

    ExtendedCharTable::instance.windows.insert(window);

    _ownConnections.push_back(window->selectionChanged.connect([this] { bufferedUpdate(); }));

    _ownConnections.push_back(outputChanged.connect([window] { window->notifyOutputChanged(); }));

    _ownConnections.push_back(handleCommandFromKeyboard.connect([window](KeyboardTranslator::Command command) {
        window->handleCommandFromKeyboard(command);
    }));
    _ownConnections.push_back(outputFromKeypressEvent.connect([window] { window->cursorRevealRequested(); }));

    return window;
}

Emulation::~Emulation()
{
  _ownConnections.clear();

  for (ScreenWindow* win : _windows)
  {
    ExtendedCharTable::instance.windows.erase(win);
    delete win;
  }

  delete _screen[0];
  delete _screen[1];
}

void Emulation::setScreen(int n)
{
  Screen *old = _currentScreen;
  _currentScreen = _screen[n & 1];
  if (_currentScreen != old)
  {
     // tell all windows onto this emulation to switch to the newly active screen
     for(ScreenWindow* window : _windows)
         window->setScreen(_currentScreen);
  }
}

void Emulation::clearHistory()
{
    _screen[0]->setScroll( _screen[0]->getScroll() , false );

    // Views would keep the old scroll range until the next output otherwise.
    // Scheduled, not shown at once: ESC[3J arrives mid-chunk, and showBulk()
    // would cancel the update for the rest of that chunk.
    bufferedUpdate();
}
bool Emulation::clearScreenAndHistory(bool atPrompt)
{
    // Clearing under a full-screen program would break its idea of the screen.
    if (_currentScreen != _screen[0])
        return false;

    _screen[0]->setScroll( _screen[0]->getScroll() , false );

    if (atPrompt) {
        _screen[0]->eraseScreen();
        // The shell's clear-screen redraws the prompt and any typed input.
        sendData("\x0C", 1);
    } else {
        _screen[0]->eraseAboveCursor();
    }

    bufferedUpdate();
    return true;
}

void Emulation::setHistory(const HistoryType& t)
{
  _screen[0]->setScroll(t);

  showBulk();
}

const HistoryType& Emulation::history() const
{
  return _screen[0]->getScroll();
}

void Emulation::setKeyBindings(const std::string& name)
{
  _keyTranslator = KeyboardTranslatorManager::instance()->findTranslator(name);
  if (!_keyTranslator)
  {
      _keyTranslator = KeyboardTranslatorManager::instance()->defaultTranslator();
  }
}

std::string Emulation::keyBindings() const
{
  return _keyTranslator->name();
}

void Emulation::receiveChar(wchar_t c)
// process application unicode input to terminal
// this is a trivial scanner
{
  c &= 0xff;
  switch (c)
  {
    case '\b'      : _currentScreen->backspace();                 break;
    case '\t'      : _currentScreen->tab();                       break;
    case '\n'      : _currentScreen->newLine();                   break;
    case '\r'      : _currentScreen->toStartOfLine();             break;
    case 0x07      : stateSet(NOTIFYBELL);
                     break;
    default        : _currentScreen->displayCharacter(c);         break;
  };
}

void Emulation::sendKeyEvent(KeyEvent* ev, bool)
{
  stateSet(NOTIFYNORMAL);

  if (!ev->text.empty())
  { // A block of text
    // Note that the text is proper unicode.
    // We should do a conversion here
    sendData(ev->text.c_str(),int(strings::utf8ToUtf16(ev->text).size()));
  }
}

void Emulation::sendString(const char*,int)
{
    // default implementation does nothing
}

void Emulation::sendMouseEvent(int /*buttons*/, int /*column*/, int /*row*/, int /*eventType*/)
{
    // default implementation does nothing
}

/*
   We are doing code conversion from locale to unicode first.
TODO: Character composition from the old code.  See #96536
*/

void Emulation::receiveData(const char* text, int length)
{
    stateSet(NOTIFYACTIVITY);

    bufferedUpdate();

    /* XXX: the following code involves encoding & decoding of "UTF-16
     * surrogate pairs", which does not work with characters higher than
     * U+10FFFF
     * https://unicodebook.readthedocs.io/unicode_encodings.html#surrogates
     */
    _decoded.clear();
    _toUtf16.decode(text, length, _decoded);

    //send characters to terminal emulator; an unpaired surrogate becomes U+FFFD
    const char16_t* unit = _decoded.data();
    const char16_t* end = unit + _decoded.size();
    while (unit < end)
    {
        const char16_t current = *unit++;
        if ((current & 0xf800) == 0xd800)
        {
            if ((current & 0xfc00) == 0xd800 && unit < end && (*unit & 0xfc00) == 0xdc00)
                receiveChar(wchar_t((char32_t(current) << 10) + *unit++ - 0x35fdc00));
            else
                receiveChar(wchar_t(0xfffd));
        }
        else
        {
            receiveChar(wchar_t(current));
        }
    }

    //look for z-modem indicator
    //-- someone who understands more about z-modems that I do may be able to move
    //this check into the above for loop?
    for (int i=0;i<length;i++)
    {
        if (text[i] == '\030')
        {
            if ((length-i-1 > 3) && (strncmp(text+i+1, "B00", 3) == 0))
                zmodemDetected();
        }
    }
}

//OLDER VERSION
//This version of onRcvBlock was commented out because
//    a)  It decoded incoming characters one-by-one, which is slow in the current version of Qt (4.2 tech preview)
//    b)  It messed up decoding of non-ASCII characters, with the result that (for example) chinese characters
//        were not printed properly.
//
//There is something about stopping the _decoder if "we get a control code halfway a multi-byte sequence" (see below)
//which hasn't been ported into the newer function (above).  Hopefully someone who understands this better
//can find an alternative way of handling the check.


/*void Emulation::onRcvBlock(const char *s, int len)
{
  notifySessionState(NOTIFYACTIVITY);

  bufferedUpdate();
  for (int i = 0; i < len; i++)
  {

    QString result = _decoder->toUnicode(&s[i],1);
    int reslen = result.length();

    // If we get a control code halfway a multi-byte sequence
    // we flush the _decoder and continue with the control code.
    if ((s[i] < 32) && (s[i] > 0))
    {
       // Flush _decoder
       while(!result.length())
          result = _decoder->toUnicode(&s[i],1);
       reslen = 1;
       result.resize(reslen);
       result[0] = QChar(s[i]);
    }

    for (int j = 0; j < reslen; j++)
    {
      if (result[j].characterategory() == QChar::Mark_NonSpacing)
         _currentScreen->compose(result.mid(j,1));
      else
         onRcvChar(result[j].unicode());
    }
    if (s[i] == '\030')
    {
      if ((len-i-1 > 3) && (strncmp(s+i+1, "B00", 3) == 0))
          zmodemDetected();
    }
  }
}*/

void Emulation::writeToStream( TerminalCharacterDecoder* _decoder ,
                               int startLine ,
                               int endLine)
{
  _currentScreen->writeLinesToStream(_decoder,startLine,endLine);
}

void Emulation::writeToStream( TerminalCharacterDecoder* _decoder)
{
  _currentScreen->writeLinesToStream(_decoder, 0, _currentScreen->getHistLines());
}

int Emulation::lineCount() const
{
    // sum number of lines currently on _screen plus number of lines in history
    return _currentScreen->getLines() + _currentScreen->getHistLines();
}

void Emulation::showBulk()
{
    _bulkTimer1.stop();
    _bulkTimer2.stop();
    _lastShow.start();

    outputChanged();

    _currentScreen->resetScrolledLines();
    _currentScreen->resetDroppedLines();
}

void Emulation::bufferedUpdate()
{
    static const int BULK_TIMEOUT1 = 10;
    static const int BULK_TIMEOUT2 = 40;
    // Output after a quiet spell, such as an echoed keystroke, is shown almost at
    // once; output that keeps coming is still batched, which keeps throughput up.
    static const int QUIET_TIMEOUT = 2;
    const bool quiet = !_lastShow.isValid() || _lastShow.elapsedMs() > BULK_TIMEOUT2;

   _bulkTimer1.setSingleShot(true);
   _bulkTimer1.start(std::chrono::milliseconds(quiet ? QUIET_TIMEOUT : BULK_TIMEOUT1));
   if (!_bulkTimer2.isActive())
   {
      _bulkTimer2.setSingleShot(true);
      _bulkTimer2.start(std::chrono::milliseconds(BULK_TIMEOUT2));
   }
}

char Emulation::eraseChar() const
{
  return '\b';
}

void Emulation::setImageSize(int lines, int columns)
{
  if ((lines < 1) || (columns < 1))
    return;

  Lingmo::base::Size screenSize[2] = { Lingmo::base::Size(_screen[0]->getColumns(),
                                _screen[0]->getLines()),
                          Lingmo::base::Size(_screen[1]->getColumns(),
                                _screen[1]->getLines()) };
  Lingmo::base::Size newSize(columns,lines);

  if (newSize == screenSize[0] && newSize == screenSize[1])
    return;

  _screen[0]->resizeImage(lines,columns);
  _screen[1]->resizeImage(lines,columns);

  imageSizeChanged(lines,columns);

  bufferedUpdate();
}

Lingmo::base::Size Emulation::imageSize() const
{
  return Lingmo::base::Size(_currentScreen->getColumns(), _currentScreen->getLines());
}

uint ExtendedCharTable::extendedCharHash(uint* unicodePoints , ushort length) const
{
    uint hash = 0;
    for ( ushort i = 0 ; i < length ; i++ )
    {
        hash = 31*hash + unicodePoints[i];
    }
    return hash;
}
bool ExtendedCharTable::extendedCharMatch(uint hash , uint* unicodePoints , ushort length) const
{
    auto found = extendedCharTable.find(hash);
    uint* entry = found == extendedCharTable.end() ? nullptr : found->second;

    // compare given length with stored sequence length ( given as the first ushort in the
    // stored buffer )
    if ( entry == nullptr || entry[0] != length )
       return false;
    // if the lengths match, each character must be checked.  the stored buffer starts at
    // entry[1]
    for ( int i = 0 ; i < length ; i++ )
    {
        if ( entry[i+1] != unicodePoints[i] )
           return false;
    }
    return true;
}
void ExtendedCharTable::pruneUnused()
{
    std::unordered_set<uint> usedExtendedChars;
    for (const auto &w : windows)
    {
        if (w->screen())
        {
            w->screen()->collectUsedExtendedChars(usedExtendedChars);
        }
    }

    auto it = extendedCharTable.begin();
    while (it != extendedCharTable.end())
    {
        if (usedExtendedChars.count(it->first))
        {
            ++it;
        }
        else
        {
            delete[] it->second;
            it = extendedCharTable.erase(it);
        }
    }

    // Still-used sequences stay; pruning again only once the table has doubled
    // keeps a screen full of them from being rescanned on every new one.
    pruneAt = kMax(kPruneThreshold, int(extendedCharTable.size()) * 2);
}

uint ExtendedCharTable::createExtendedChar(uint* unicodePoints , ushort length)
{
    // look for this sequence of points in the table
    uint hash = extendedCharHash(unicodePoints,length);
    // 0 has a special meaning for chars, so a sequence hashing to it moves on.
    if (hash == 0)
        hash = 1;
    const uint initialHash = hash;
    bool triedCleaningSolution = false;

    // The table only ever grew; a long session with many combining
    // sequences keeps every one until the screens are asked which are
    // still in use.
    if (int(extendedCharTable.size()) >= pruneAt)
        pruneUnused();

    // check existing entry for match
    while (extendedCharTable.count(hash) && hash != 0) // 0 has a special meaning for chars so we don't use it
    {
        if (extendedCharMatch(hash, unicodePoints, length))
        {
            // this sequence already has an entry in the table,
            // return its hash
            return hash;
        }
        else
        {
            // if hash is already used by another, different sequence of
            // unicode character points, then try next hash
            hash++;

            if (hash == initialHash)
            {
                if (!triedCleaningSolution)
                {
                    triedCleaningSolution = true;
                    // All the hashes are full, go to all Screens and try to free any
                    // This is slow but should happen very rarely
                    pruneUnused();
                }
                else
                {
                    LM_WARNING(qtermwidgetLogger) << "Using all the extended char hashes, going to miss this extended character";
                    return 0;
                }
            }
        }
    }


     // add the new sequence to the table and
     // return that index
    uint* buffer = new uint[length+1];
    buffer[0] = length;
    for ( int i = 0 ; i < length ; i++ )
       buffer[i+1] = unicodePoints[i];

    extendedCharTable.emplace(hash,buffer);

    return hash;
}

uint* ExtendedCharTable::lookupExtendedChar(uint hash , ushort& length) const
{
    // lookup index in table and if found, set the length
    // argument and return a pointer to the character sequence

    auto found = extendedCharTable.find(hash);
    uint* buffer = found == extendedCharTable.end() ? nullptr : found->second;
    if ( buffer )
    {
        length = buffer[0];
        return buffer+1;
    }
    else
    {
        length = 0;
        return nullptr;
    }
}

ExtendedCharTable::ExtendedCharTable()
{
}
ExtendedCharTable::~ExtendedCharTable()
{
    // free all allocated character buffers
    for (auto& entry : extendedCharTable)
    {
        delete[] entry.second;
    }
}

// global instance
ExtendedCharTable ExtendedCharTable::instance;

// QUtf8Functions::fromUtf8() with QUtf8BaseTraits: the code units for the
// sequence starting with `b`, the number of bytes taken (including `b`), or
// Error or EndOfString.
static const int Utf8Error = -1;
static const int Utf8EndOfString = -2;

static inline bool isContinuationByte(unsigned char b)
{
    return (b & 0xc0) == 0x80;
}

static int fromUtf8(unsigned char b, char16_t*& dst, const unsigned char*& src, const unsigned char* end)
{
    int charsNeeded;
    char32_t min_uc;
    char32_t uc;

    if (b < 0x80) {
        *dst++ = b;
        return 1;
    }

    if (b <= 0xC1) {
        // an UTF-8 first character must be at least 0xC0
        // however, all 0xC0 and 0xC1 first bytes can only produce overlong sequences
        return Utf8Error;
    } else if (b < 0xe0) {
        charsNeeded = 2;
        min_uc = 0x80;
        uc = b & 0x1f;
    } else if (b < 0xf0) {
        charsNeeded = 3;
        min_uc = 0x800;
        uc = b & 0x0f;
    } else if (b < 0xf5) {
        charsNeeded = 4;
        min_uc = 0x10000;
        uc = b & 0x07;
    } else {
        // the last Unicode character is U+10FFFF
        return Utf8Error;
    }

    const long bytesAvailable = end - src;
    if (bytesAvailable < charsNeeded - 1) {
        // it's possible that we have an error instead of just unfinished bytes
        if (bytesAvailable > 0 && !isContinuationByte(src[0]))
            return Utf8Error;
        if (bytesAvailable > 1 && !isContinuationByte(src[1]))
            return Utf8Error;
        return Utf8EndOfString;
    }

    // first continuation character
    b = src[0];
    if (!isContinuationByte(b))
        return Utf8Error;
    uc <<= 6;
    uc |= b & 0x3f;

    if (charsNeeded > 2) {
        // second continuation character
        b = src[1];
        if (!isContinuationByte(b))
            return Utf8Error;
        uc <<= 6;
        uc |= b & 0x3f;

        if (charsNeeded > 3) {
            // third continuation character
            b = src[2];
            if (!isContinuationByte(b))
                return Utf8Error;
            uc <<= 6;
            uc |= b & 0x3f;
        }
    }

    // we've decoded something; safety-check it
    if (uc < min_uc)
        return Utf8Error;
    if ((uc >= 0xd800 && uc <= 0xdfff) || uc > 0x10ffff)
        return Utf8Error;

    // write the UTF-16 sequence
    if (uc < 0x10000) {
        *dst++ = char16_t(uc);
    } else {
        *dst++ = char16_t((uc >> 10) + 0xd7c0);
        *dst++ = char16_t(uc % 0x400 + 0xdc00);
    }

    src += charsNeeded - 1;
    return charsNeeded;
}

// QUtf8::convertToUnicode() with a state and no flags.
void Utf8Decoder::decode(const char* bytes, int length, std::u16string& output)
{
    if (length <= 0)
        return;

    const size_t start = output.size();
    output.resize(start + size_t(length) + size_t(_remainingChars) + 1);
    char16_t* dst = output.data() + start;

    const char16_t replacement = 0xfffd;
    int res;
    unsigned char ch = 0;

    const unsigned char* src = reinterpret_cast<const unsigned char*>(bytes);
    const unsigned char* end = src + length;

    const bool headerdone = _headerDone;
    if (_remainingChars || !headerdone) {
        // handle incoming state first
        unsigned char remainingCharsData[4]; // longest UTF-8 sequence possible
        const int remainingCharsCount = _remainingChars;
        const int newCharsToCopy = int(kMin(long(sizeof(remainingCharsData)) - remainingCharsCount, long(end - src)));

        std::memset(remainingCharsData, 0, sizeof(remainingCharsData));
        std::memcpy(remainingCharsData, _stateData, remainingCharsCount);
        std::memcpy(remainingCharsData + remainingCharsCount, src, newCharsToCopy);

        const unsigned char* begin = &remainingCharsData[1];
        res = fromUtf8(remainingCharsData[0], dst, begin,
                       remainingCharsData + remainingCharsCount + newCharsToCopy);
        if (res == Utf8Error) {
            *dst++ = replacement;
            ++src;
        } else if (res == Utf8EndOfString) {
            // if we got EndOfString again, then there were too few bytes in src;
            // copy to our state and return
            _remainingChars = remainingCharsCount + newCharsToCopy;
            std::memcpy(_stateData, remainingCharsData, _remainingChars);
            output.resize(size_t(dst - output.data()));
            return;
        } else if (!headerdone) {
            // eat the UTF-8 BOM
            if (dst[-1] == 0xfeff)
                --dst;
        }
        _headerDone = true;

        // adjust src now that we have maybe consumed a few chars
        if (res >= 0) {
            assert(res > remainingCharsCount);
            src += res - remainingCharsCount;
        }
    }

    // main body, stateless decoding
    res = 0;
    while (res >= 0 && src < end) {
        while (src < end && *src < 0x80)
            *dst++ = char16_t(*src++);
        if (src == end)
            break;
        ch = *src++;
        res = fromUtf8(ch, dst, src, end);
        if (res == Utf8Error) {
            res = 0;
            *dst++ = replacement;
        }
    }

    if (res == Utf8EndOfString) {
        --src; // unread the byte in ch
        _remainingChars = int(end - src);
        std::memcpy(_stateData, src, size_t(end - src));
    } else {
        _remainingChars = 0;
    }

    output.resize(size_t(dst - output.data()));
}


//#include "Emulation.moc"


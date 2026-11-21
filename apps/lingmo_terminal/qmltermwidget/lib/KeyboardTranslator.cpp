/*
    This source file is part of Konsole, a terminal emulator.

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
#include "KeyboardTranslator.h"

// System
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>

#include <unicode/uchar.h>

#include "base/file.h"
#include "base/strings.h"

#include "RegExp.h"
#include "tools.h"

// KDE
//#include <KDebug>
//#include <KLocale>
//#include <KStandardDirs>

using namespace Konsole;
namespace strings = Lingmo::base::strings;


const std::string KeyboardTranslatorManager::defaultTranslatorText(
"keyboard \"Fallback Key Translator\"\n"
"key Tab : \"\\t\""
);

const uint32_t KeyboardTranslator::CTRL_MOD = ControlModifier;

KeyboardTranslatorManager::KeyboardTranslatorManager()
    : _haveLoadedAll(false)
{
}
KeyboardTranslatorManager::~KeyboardTranslatorManager()
{
    for (auto& translator : _translators)
        delete translator.second;
}
std::string KeyboardTranslatorManager::findTranslatorPath(const std::string& name)
{
    return std::string(get_kb_layout_dir() + name + ".keytab");
    //return KGlobal::dirs()->findResource("data","konsole/"+name+".keytab");
}

void KeyboardTranslatorManager::findTranslators()
{
    // QDir::entryList() order: by name.
    std::vector<std::string> list;
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(get_kb_layout_dir(), error))
    {
        const std::string fileName = entry.path().filename().string();
        if (fileName.size() > 7 && fileName.compare(fileName.size() - 7, 7, ".keytab") == 0)
            list.push_back(fileName);
    }
    std::sort(list.begin(), list.end());
//    QStringList list = KGlobal::dirs()->findAllResources("data",
//                                                         "konsole/*.keytab",
//                                                        KStandardDirs::NoDuplicates);

    // add the name of each translator to the list and associated
    // the name with a null pointer to indicate that the translator
    // has not yet been loaded from disk
    for (const std::string& translatorPath : list)
    {
        // QFileInfo::baseName(): up to the first dot.
        std::string name = translatorPath.substr(0, translatorPath.find('.'));

        if ( !_translators.count(name) )
            _translators.emplace(name,nullptr);
    }

    _haveLoadedAll = true;
}

const KeyboardTranslator* KeyboardTranslatorManager::findTranslator(const std::string& name)
{
    if ( name.empty() )
        return defaultTranslator();

    if ( auto found = _translators.find(name); found != _translators.end() && found->second != nullptr )
        return found->second;

    KeyboardTranslator* translator = loadTranslator(name);

    if ( translator != nullptr )
        _translators[name] = translator;
    else if ( !name.empty() )
        LM_DEBUG(qtermwidgetLogger) << "Unable to load translator" << name;

    return translator;
}

bool KeyboardTranslatorManager::saveTranslator(const KeyboardTranslator* translator)
{
LM_DEBUG(qtermwidgetLogger) << "KeyboardTranslatorManager::saveTranslator" << "unimplemented";
(void)translator;
    return true;
}

KeyboardTranslator* KeyboardTranslatorManager::loadTranslator(const std::string& name)
{
    const std::string& path = findTranslatorPath(name);

    if (name.empty())
        return nullptr;

    std::optional<std::string> source = Lingmo::base::file::read(path);
    if (!source)
        return nullptr;

    return loadTranslator(*source,name);
}

const KeyboardTranslator* KeyboardTranslatorManager::defaultTranslator()
{
    // Try to find the default.keytab file if it exists, otherwise
    // fall back to the hard-coded one
    const KeyboardTranslator* translator = findTranslator("default");
    if (!translator)
    {
        // Kept like a loaded one, so repeated calls share the fallback.
        const std::string fallbackName = "fallback";
        if (auto found = _translators.find(fallbackName); found != _translators.end() && found->second)
            return found->second;
        KeyboardTranslator* fallback = loadTranslator(defaultTranslatorText,fallbackName);
        _translators.insert_or_assign(fallbackName, fallback);
        translator = fallback;
    }
    return translator;
}

KeyboardTranslator* KeyboardTranslatorManager::loadTranslator(const std::string& source,const std::string& name)
{
    KeyboardTranslator* translator = new KeyboardTranslator(name);
    KeyboardTranslatorReader reader(source);
    translator->setDescription( reader.description() );
    while ( reader.hasNextEntry() )
        translator->addEntry(reader.nextEntry());

    if ( !reader.parseError() )
    {
        return translator;
    }
    else
    {
        delete translator;
        return nullptr;
    }
}

KeyboardTranslatorWriter::KeyboardTranslatorWriter(std::string* destination)
: _destination(destination)
{
    assert( destination );
}
KeyboardTranslatorWriter::~KeyboardTranslatorWriter()
{
}
void KeyboardTranslatorWriter::writeHeader( const std::string& description )
{
    *_destination += "keyboard \"" + description + "\"\n";
}
void KeyboardTranslatorWriter::writeEntry( const KeyboardTranslator::Entry& entry )
{
    std::string result;
    if ( entry.command() != KeyboardTranslator::NoCommand )
        result = entry.resultToString();
    else
        result = '\"' + entry.resultToString() + '\"';

    *_destination += "key " + entry.conditionToString() + " : " + result + '\n';
}


// each line of the keyboard translation file is one of:
//
// - keyboard "name"
// - key KeySequence : "characters"
// - key KeySequence : CommandName
//
// KeySequence begins with the name of the key ( taken from the Qt::Key enum )
// and is followed by the keyboard modifiers and state flags ( with + or - in front
// of each modifier or flag to indicate whether it is required ).  All keyboard modifiers
// and flags are optional, if a particular modifier or state is not specified it is
// assumed not to be a part of the sequence.  The key sequence may contain whitespace
//
// eg:  "key Up+Shift : scrollLineUp"
//      "key Next-Shift : "\E[6~"
//
// (lines containing only whitespace are ignored, parseLine assumes that comments have
// already been removed)
//

KeyboardTranslatorReader::KeyboardTranslatorReader( const std::string& source )
    : _hasNext(false)
{
   // Text mode, as the files were opened: line breaks read as "\n".
   _source = strings::replaceAll(source, "\r\n", "\n");

   // read input until we find the description
   while ( _description.empty() && !atEnd() )
   {
        std::vector<Token> tokens = tokenize( strings::sanitizedUtf8(readLine()) );
        if ( !tokens.empty() && tokens.front().type == Token::TitleKeyword )
            _description = tokens[1].text;
   }
   // read first entry (if any)
   readNext();
}
bool KeyboardTranslatorReader::atEnd() const
{
    return _sourcePosition >= _source.size();
}
std::string KeyboardTranslatorReader::readLine()
{
    const size_t end = _source.find('\n', _sourcePosition);
    const size_t next = end == std::string::npos ? _source.size() : end + 1;
    std::string line = _source.substr(_sourcePosition, next - _sourcePosition);
    _sourcePosition = next;
    return line;
}
void KeyboardTranslatorReader::readNext()
{
    // find next entry
    while ( !atEnd() )
    {
        const std::vector<Token>& tokens = tokenize( strings::sanitizedUtf8(readLine()) );
        if ( !tokens.empty() && tokens.front().type == Token::KeyKeyword )
        {
            KeyboardTranslator::States flags = KeyboardTranslator::NoState;
            KeyboardTranslator::States flagMask = KeyboardTranslator::NoState;
            uint32_t modifiers = NoModifier;
            uint32_t modifierMask = NoModifier;

            int keyCode = Key_unknown;

            decodeSequence(strings::toAsciiLower(tokens[1].text),
                           keyCode,
                           modifiers,
                           modifierMask,
                           flags,
                           flagMask);

            KeyboardTranslator::Command command = KeyboardTranslator::NoCommand;
            std::string text;

            // get text or command
            if ( tokens[2].type == Token::OutputText )
            {
                text = tokens[2].text;
            }
            else if ( tokens[2].type == Token::Command )
            {
                // identify command
                if (!parseAsCommand(tokens[2].text,command))
                    LM_DEBUG(qtermwidgetLogger) << "Command" << tokens[2].text << "not understood.";
            }

            KeyboardTranslator::Entry newEntry;
            newEntry.setKeyCode( keyCode );
            newEntry.setState( flags );
            newEntry.setStateMask( flagMask );
            newEntry.setModifiers( modifiers );
            newEntry.setModifierMask( modifierMask );
            newEntry.setText( text );
            newEntry.setCommand( command );

            _nextEntry = newEntry;

            _hasNext = true;

            return;
        }
    }

    _hasNext = false;
}

bool KeyboardTranslatorReader::parseAsCommand(const std::string& text,KeyboardTranslator::Command& command)
{
    if ( strings::equalsIgnoreAsciiCase(text, "erase") )
        command = KeyboardTranslator::EraseCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrollpageup") )
        command = KeyboardTranslator::ScrollPageUpCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrollpagedown") )
        command = KeyboardTranslator::ScrollPageDownCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrolllineup") )
        command = KeyboardTranslator::ScrollLineUpCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrolllinedown") )
        command = KeyboardTranslator::ScrollLineDownCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrolllock") )
        command = KeyboardTranslator::ScrollLockCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrolluptotop") )
        command = KeyboardTranslator::ScrollUpToTopCommand;
    else if ( strings::equalsIgnoreAsciiCase(text, "scrolldowntobottom") )
        command = KeyboardTranslator::ScrollDownToBottomCommand;
    else
        return false;

    return true;
}

// QChar::isLetterOrNumber().
static bool isLetterOrNumber(char16_t ch)
{
    switch (u_charType(ch))
    {
    case U_UPPERCASE_LETTER:
    case U_LOWERCASE_LETTER:
    case U_TITLECASE_LETTER:
    case U_MODIFIER_LETTER:
    case U_OTHER_LETTER:
    case U_DECIMAL_DIGIT_NUMBER:
    case U_LETTER_NUMBER:
    case U_OTHER_NUMBER:
        return true;
    default:
        return false;
    }
}

bool KeyboardTranslatorReader::decodeSequence(const std::string& utf8Text,
                                              int& keyCode,
                                              uint32_t& modifiers,
                                              uint32_t& modifierMask,
                                              KeyboardTranslator::States& flags,
                                              KeyboardTranslator::States& flagMask)
{
    bool isWanted = true;
    bool endOfItem = false;
    std::u16string buffer;
    const std::u16string text = strings::utf8ToUtf16(utf8Text);

    uint32_t tempModifiers = modifiers;
    uint32_t tempModifierMask = modifierMask;
    KeyboardTranslator::States tempFlags = flags;
    KeyboardTranslator::States tempFlagMask = flagMask;

    for ( int i = 0 ; i < int(text.size()) ; i++ )
    {
        const char16_t& ch = text[i];
        bool isFirstLetter = i == 0;
        bool isLastLetter = ( i == int(text.size())-1 );
        endOfItem = true;
        if ( isLetterOrNumber(ch) )
        {
            endOfItem = false;
            buffer.push_back(ch);
        } else if ( isFirstLetter )
        {
            buffer.push_back(ch);
        }

        if ( (endOfItem || isLastLetter) && !buffer.empty() )
        {
            uint32_t itemModifier = NoModifier;
            int itemKeyCode = 0;
            KeyboardTranslator::State itemFlag = KeyboardTranslator::NoState;
            const std::string item = strings::utf16ToUtf8(buffer);

            if ( parseAsModifier(item,itemModifier) )
            {
                tempModifierMask |= itemModifier;

                if ( isWanted )
                    tempModifiers |= itemModifier;
            }
            else if ( parseAsStateFlag(item,itemFlag) )
            {
                tempFlagMask |= itemFlag;

                if ( isWanted )
                    tempFlags |= itemFlag;
            }
            else if ( parseAsKeyCode(item,itemKeyCode) )
                keyCode = itemKeyCode;
            else
                LM_DEBUG(qtermwidgetLogger) << "Unable to parse key binding item:" << item;

            buffer.clear();
        }

        // check if this is a wanted / not-wanted flag and update the
        // state ready for the next item
        if ( ch == u'+' )
           isWanted = true;
        else if ( ch == u'-' )
           isWanted = false;
    }

    modifiers = tempModifiers;
    modifierMask = tempModifierMask;
    flags = tempFlags;
    flagMask = tempFlagMask;

    return true;
}

bool KeyboardTranslatorReader::parseAsModifier(const std::string& item , uint32_t& modifier)
{
    if ( item == "shift" )
        modifier = ShiftModifier;
    else if ( item == "ctrl" || item == "control" )
        modifier = ControlModifier;
    else if ( item == "alt" )
        modifier = AltModifier;
    else if ( item == "meta" )
        modifier = MetaModifier;
    else if ( item == "keypad" )
        modifier = KeypadModifier;
    else
        return false;

    return true;
}
bool KeyboardTranslatorReader::parseAsStateFlag(const std::string& item , KeyboardTranslator::State& flag)
{
    if ( item == "appcukeys" || item == "appcursorkeys" )
        flag = KeyboardTranslator::CursorKeysState;
    else if ( item == "ansi" )
        flag = KeyboardTranslator::AnsiState;
    else if ( item == "newline" )
        flag = KeyboardTranslator::NewLineState;
    else if ( item == "appscreen" )
        flag = KeyboardTranslator::AlternateScreenState;
    else if ( item == "anymod" || item == "anymodifier" )
        flag = KeyboardTranslator::AnyModifierState;
    else if ( item == "appkeypad" )
        flag = KeyboardTranslator::ApplicationKeypadState;
    else
        return false;

    return true;
}
bool KeyboardTranslatorReader::parseAsKeyCode(const std::string& item , int& keyCode)
{
    // QKeySequence::fromString() of a non-empty name is never empty: a name it
    // does not know gives Key_unknown.
    if ( !item.empty() )
    {
        keyCode = keyCodeFromName(item);
    }
    // additional cases implemented for backwards compatibility with KDE 3
    else if ( item == "prior" )
        keyCode = Key_PageUp;
    else if ( item == "next" )
        keyCode = Key_PageDown;
    else
        return false;

    return true;
}

std::string KeyboardTranslatorReader::description() const
{
    return _description;
}
bool KeyboardTranslatorReader::hasNextEntry() const
{
    return _hasNext;
}
KeyboardTranslator::Entry KeyboardTranslatorReader::createEntry( const std::string& condition ,
                                                                 const std::string& result )
{
    std::string entryString = "keyboard \"temporary\"\nkey ";
    entryString.append(condition);
    entryString.append(" : ");

    // if 'result' is the name of a command then the entry result will be that command,
    // otherwise the result will be treated as a string to echo when the key sequence
    // specified by 'condition' is pressed
    KeyboardTranslator::Command command;
    if (parseAsCommand(result,command))
        entryString.append(result);
    else
        entryString.append('\"' + result + '\"');

    KeyboardTranslatorReader reader(entryString);

    KeyboardTranslator::Entry entry;
    if ( reader.hasNextEntry() )
        entry = reader.nextEntry();

    return entry;
}

KeyboardTranslator::Entry KeyboardTranslatorReader::nextEntry()
{
    assert( _hasNext );
    KeyboardTranslator::Entry entry = _nextEntry;
    readNext();
    return entry;
}
bool KeyboardTranslatorReader::parseError()
{
    return false;
}
std::vector<KeyboardTranslatorReader::Token> KeyboardTranslatorReader::tokenize(const std::string& line)
{
    std::u16string text = strings::utf8ToUtf16(line);

    // remove comments
    bool inQuotes = false;
    int commentPos = -1;
    for (int i=int(text.length())-1;i>=0;i--)
    {
        char16_t ch = text[i];
        if (ch == u'\"')
            inQuotes = !inQuotes;
        else if (ch == u'#' && !inQuotes)
            commentPos = i;
    }
    if (commentPos != -1)
        text.erase(commentPos);

    text = strings::utf8ToUtf16(strings::simplified(strings::utf16ToUtf8(text)));

    // title line: keyboard "title"
    static const RegExp title(u"keyboard\\s+\"(.*)\"");
    // key line: key KeySequence : "output"
    // key line: key KeySequence : command
    static const RegExp key(u"key\\s+([\\w\\+\\s\\-\\*\\.]+)\\s*:\\s*(\"(.*)\"|\\w+)");

    std::vector<Token> list;
    if ( text.empty() )
    {
        return list;
    }

    const auto titleMatch = title.match(text);
    const auto keyMatch = key.match(text);

    if ( titleMatch.hasMatch )
    {
        Token titleToken = { Token::TitleKeyword , std::string() };
        Token textToken = { Token::TitleText , strings::utf16ToUtf8(titleMatch.captured(text, 1)) };

        list.push_back(titleToken);
        list.push_back(textToken);
    }
    else if  ( keyMatch.hasMatch )
    {
        Token keyToken = { Token::KeyKeyword , std::string() };
        Token sequenceToken = { Token::KeySequence , strings::replaceAll(strings::utf16ToUtf8(keyMatch.captured(text, 1)), " ", "") };

        list.push_back(keyToken);
        list.push_back(sequenceToken);

        if ( keyMatch.captured(text, 3).empty() )
        {
            // capturedTexts()[2] is a command
            Token commandToken = { Token::Command , strings::utf16ToUtf8(keyMatch.captured(text, 2)) };
            list.push_back(commandToken);
        }
        else
        {
            // capturedTexts()[3] is the output string
           Token outputToken = { Token::OutputText , strings::utf16ToUtf8(keyMatch.captured(text, 3)) };
           list.push_back(outputToken);
        }
    }
    else
    {
        LM_DEBUG(qtermwidgetLogger) << "Line in keyboard translator file could not be understood:" << strings::utf16ToUtf8(text);
    }

    return list;
}

std::vector<std::string> KeyboardTranslatorManager::allTranslators()
{
    if ( !_haveLoadedAll )
    {
        findTranslators();
    }

    std::vector<std::string> names;
    for (const auto& translator : _translators)
        names.push_back(translator.first);
    return names;
}

KeyboardTranslator::Entry::Entry()
: _keyCode(0)
, _modifiers(NoModifier)
, _modifierMask(NoModifier)
, _state(NoState)
, _stateMask(NoState)
, _command(NoCommand)
{
}

bool KeyboardTranslator::Entry::operator==(const Entry& rhs) const
{
    return _keyCode == rhs._keyCode &&
           _modifiers == rhs._modifiers &&
           _modifierMask == rhs._modifierMask &&
           _state == rhs._state &&
           _stateMask == rhs._stateMask &&
           _command == rhs._command &&
           _text == rhs._text;
}

bool KeyboardTranslator::Entry::matches(int keyCode ,
                                        uint32_t modifiers,
                                        States testState) const
{
    if ( _keyCode != keyCode )
        return false;

    if ( (modifiers & _modifierMask) != (_modifiers & _modifierMask) )
        return false;

    // if modifiers is non-zero, the 'any modifier' state is implicit
    if ( (modifiers & ~KeypadModifier) != 0 )
        testState |= AnyModifierState;

    if ( (testState & _stateMask) != (_state & _stateMask) )
        return false;

    // special handling for the 'Any Modifier' state, which checks for the presence of
    // any or no modifiers.  In this context, the 'keypad' modifier does not count.
    bool anyModifiersSet = modifiers != 0 && modifiers != KeypadModifier;
    bool wantAnyModifier = _state & KeyboardTranslator::AnyModifierState;
    if ( _stateMask & KeyboardTranslator::AnyModifierState )
    {
        if ( wantAnyModifier != anyModifiersSet )
           return false;
    }

    return true;
}

// QChar::isPrint() of a Latin-1 character.
static bool isPrintLatin1(char ch)
{
    switch (u_charType(UChar32(static_cast<unsigned char>(ch))))
    {
    case U_CONTROL_CHAR:
    case U_FORMAT_CHAR:
    case U_SURROGATE:
    case U_PRIVATE_USE_CHAR:
    case U_UNASSIGNED:
        return false;
    default:
        return true;
    }
}

std::string KeyboardTranslator::Entry::escapedText(bool expandWildCards,uint32_t modifiers) const
{
    std::string result(text(expandWildCards,modifiers));

    for ( int i = 0 ; i < int(result.size()) ; i++ )
    {
        char ch = result[i];
        char replacement = 0;

        switch ( ch )
        {
            case 27 : replacement = 'E'; break;
            case 8  : replacement = 'b'; break;
            case 12 : replacement = 'f'; break;
            case 9  : replacement = 't'; break;
            case 13 : replacement = 'r'; break;
            case 10 : replacement = 'n'; break;
            default:
                // any character which is not printable is replaced by an equivalent
                // \xhh escape sequence (where 'hh' are the corresponding hex digits)
                if ( !isPrintLatin1(ch) )
                    replacement = 'x';
        }

        if ( replacement == 'x' )
        {
            std::string escaped("\\x");
            escaped += strings::toHex(std::string(1,ch));
            result.replace(i, 1, escaped);
        } else if ( replacement != 0 )
        {
            result.erase(i,1);
            result.insert(result.begin() + i,'\\');
            result.insert(result.begin() + i + 1,replacement);
        }
    }

    return result;
}
std::string KeyboardTranslator::Entry::unescape(const std::string& input) const
{
    std::string result(input);

    for ( int i = 0 ; i < int(result.size())-1 ; i++ )
    {

        char ch = result[i];
        if ( ch == '\\' )
        {
           char replacement[2] = {0,0};
           int charsToRemove = 2;
           bool escapedChar = true;

           switch ( result[i+1] )
           {
              case 'E' : replacement[0] = 27; break;
              case 'b' : replacement[0] = 8 ; break;
              case 'f' : replacement[0] = 12; break;
              case 't' : replacement[0] = 9 ; break;
              case 'r' : replacement[0] = 13; break;
              case 'n' : replacement[0] = 10; break;
              case 'x' :
                 {
                    // format is \xh or \xhh where 'h' is a hexadecimal
                    // digit from 0-9 or A-F which should be replaced
                    // with the corresponding character value
                    char hexDigits[3] = {0};

                    if ( (i < int(result.size())-2) && isxdigit(result[i+2]) )
                            hexDigits[0] = result[i+2];
                    if ( (i < int(result.size())-3) && isxdigit(result[i+3]) )
                            hexDigits[1] = result[i+3];

                    unsigned charValue = 0;
                    sscanf(hexDigits,"%x",&charValue);

                    replacement[0] = (char)charValue;
                    charsToRemove = 2 + strlen(hexDigits);
                  }
              break;
              default:
                  escapedChar = false;
           }

           if ( escapedChar )
               result.replace(i,charsToRemove,replacement,1);
        }
    }

    return result;
}

void KeyboardTranslator::Entry::insertModifier( std::string& item , int modifier ) const
{
    if ( !(modifier & _modifierMask) )
        return;

    if ( modifier & _modifiers )
        item += '+';
    else
        item += '-';

    if ( modifier == ShiftModifier )
        item += "Shift";
    else if ( modifier == ControlModifier )
        item += "Ctrl";
    else if ( modifier == AltModifier )
        item += "Alt";
    else if ( modifier == MetaModifier )
        item += "Meta";
    else if ( modifier == KeypadModifier )
        item += "KeyPad";
}
void KeyboardTranslator::Entry::insertState( std::string& item , int state ) const
{
    if ( !(state & _stateMask) )
        return;

    if ( state & _state )
        item += '+' ;
    else
        item += '-' ;

    if ( state == KeyboardTranslator::AlternateScreenState )
        item += "AppScreen";
    else if ( state == KeyboardTranslator::NewLineState )
        item += "NewLine";
    else if ( state == KeyboardTranslator::AnsiState )
        item += "Ansi";
    else if ( state == KeyboardTranslator::CursorKeysState )
        item += "AppCursorKeys";
    else if ( state == KeyboardTranslator::AnyModifierState )
        item += "AnyModifier";
    else if ( state == KeyboardTranslator::ApplicationKeypadState )
        item += "AppKeypad";
}
std::string KeyboardTranslator::Entry::resultToString(bool expandWildCards,uint32_t modifiers) const
{
    if ( !_text.empty() )
        return strings::latin1ToUtf8(escapedText(expandWildCards,modifiers));
    else if ( _command == EraseCommand )
        return "Erase";
    else if ( _command == ScrollPageUpCommand )
        return "ScrollPageUp";
    else if ( _command == ScrollPageDownCommand )
        return "ScrollPageDown";
    else if ( _command == ScrollLineUpCommand )
        return "ScrollLineUp";
    else if ( _command == ScrollLineDownCommand )
        return "ScrollLineDown";
    else if ( _command == ScrollLockCommand )
        return "ScrollLock";
    else if (_command == ScrollUpToTopCommand)
        return "ScrollUpToTop";
    else if (_command == ScrollDownToBottomCommand)
        return "ScrollDownToBottom";

    return std::string();
}
std::string KeyboardTranslator::Entry::conditionToString() const
{
    std::string result = keyCodeName(_keyCode);

    insertModifier( result , ShiftModifier );
    insertModifier( result , ControlModifier );
    insertModifier( result , AltModifier );
    insertModifier( result , MetaModifier );
    insertModifier( result , KeypadModifier );

    insertState( result , KeyboardTranslator::AlternateScreenState );
    insertState( result , KeyboardTranslator::NewLineState );
    insertState( result , KeyboardTranslator::AnsiState );
    insertState( result , KeyboardTranslator::CursorKeysState );
    insertState( result , KeyboardTranslator::AnyModifierState );
    insertState( result , KeyboardTranslator::ApplicationKeypadState );

    return result;
}

KeyboardTranslator::KeyboardTranslator(const std::string& name)
: _name(name)
{
}

void KeyboardTranslator::setDescription(const std::string& description)
{
    _description = description;
}
std::string KeyboardTranslator::description() const
{
    return _description;
}
void KeyboardTranslator::setName(const std::string& name)
{
    _name = name;
}
std::string KeyboardTranslator::name() const
{
    return _name;
}

std::vector<KeyboardTranslator::Entry> KeyboardTranslator::entries() const
{
    std::vector<Entry> list;
    for (const auto& entries : _entries)
        list.insert(list.end(), entries.second.rbegin(), entries.second.rend());
    return list;
}

void KeyboardTranslator::addEntry(const Entry& entry)
{
    const int keyCode = entry.keyCode();
    _entries[keyCode].push_back(entry);
}
// QMultiHash::remove(key, value): every entry equal to it.
static void removeEntries(std::unordered_map<int,std::vector<KeyboardTranslator::Entry>>& entries,
                          const KeyboardTranslator::Entry& entry)
{
    auto found = entries.find(entry.keyCode());
    if (found == entries.end())
        return;
    std::erase(found->second, entry);
    if (found->second.empty())
        entries.erase(found);
}
void KeyboardTranslator::replaceEntry(const Entry& existing , const Entry& replacement)
{
    if ( !existing.isNull() )
        removeEntries(_entries, existing);
    _entries[replacement.keyCode()].push_back(replacement);
}
void KeyboardTranslator::removeEntry(const Entry& entry)
{
    removeEntries(_entries, entry);
}
KeyboardTranslator::Entry KeyboardTranslator::findEntry(int keyCode, uint32_t modifiers, States state) const
{
    auto found = _entries.find(keyCode);
    if (found == _entries.end())
        return Entry(); // entry not found

    for (auto it = found->second.rbegin(); it != found->second.rend(); ++it)
    {
        if ( it->matches(keyCode,modifiers,state) )
            return *it;
    }
    return Entry(); // entry not found
}
void KeyboardTranslatorManager::addTranslator(KeyboardTranslator* translator)
{
    _translators.insert_or_assign(translator->name(),translator);

    if ( !saveTranslator(translator) )
        LM_DEBUG(qtermwidgetLogger) << "Unable to save translator" << translator->name()
                   << "to disk.";
}
bool KeyboardTranslatorManager::deleteTranslator(const std::string& name)
{
    assert( _translators.count(name) );

    // locate and delete
    std::string path = findTranslatorPath(name);
    std::error_code error;
    if ( std::filesystem::remove(path, error) )
    {
        _translators.erase(name);
        return true;
    }
    else
    {
        LM_DEBUG(qtermwidgetLogger) << "Failed to remove translator - " << path;
        return false;
    }
}
KeyboardTranslatorManager* KeyboardTranslatorManager::instance()
{
    static KeyboardTranslatorManager theKeyboardTranslatorManager;
    return &theKeyboardTranslatorManager;
}

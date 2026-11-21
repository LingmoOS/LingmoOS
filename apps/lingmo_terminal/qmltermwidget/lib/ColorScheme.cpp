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
#include "ColorScheme.h"
#include "tools.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <random>

#include "base/settings.h"
#include "base/strings.h"


// KDE
//#include <KColorScheme>
//#include <KConfig>
//#include <KLocale>
//#include <KDebug>
//#include <KConfigGroup>
//#include <KStandardDirs>

using namespace Konsole;
namespace strings = Lingmo::base::strings;

const ColorEntry ColorScheme::defaultTable[TABLE_COLORS] =
 // The following are almost IBM standard color codes, with some slight
 // gamma correction for the dim colors to compensate for bright X screens.
 // It contains the 8 ansiterm/xterm colors in 2 intensities.
{
    ColorEntry( Color{0x00,0x00,0x00}, false), ColorEntry(
Color{0xFF,0xFF,0xFF}, true), // Dfore, Dback
    ColorEntry( Color{0x00,0x00,0x00}, false), ColorEntry(
Color{0xB2,0x18,0x18}, false), // Black, Red
    ColorEntry( Color{0x18,0xB2,0x18}, false), ColorEntry(
Color{0xB2,0x68,0x18}, false), // Green, Yellow
    ColorEntry( Color{0x18,0x18,0xB2}, false), ColorEntry(
Color{0xB2,0x18,0xB2}, false), // Blue, Magenta
    ColorEntry( Color{0x18,0xB2,0xB2}, false), ColorEntry(
Color{0xB2,0xB2,0xB2}, false), // Cyan, White
    // intensive
    ColorEntry( Color{0x00,0x00,0x00}, false), ColorEntry(
Color{0xFF,0xFF,0xFF}, true),
    ColorEntry( Color{0x68,0x68,0x68}, false), ColorEntry(
Color{0xFF,0x54,0x54}, false),
    ColorEntry( Color{0x54,0xFF,0x54}, false), ColorEntry(
Color{0xFF,0xFF,0x54}, false),
    ColorEntry( Color{0x54,0x54,0xFF}, false), ColorEntry(
Color{0xFF,0x54,0xFF}, false),
    ColorEntry( Color{0x54,0xFF,0xFF}, false), ColorEntry(
Color{0xFF,0xFF,0xFF}, false)
};

const ColorEntry Konsole::base_color_table[TABLE_COLORS] =
// The following are almost IBM standard color codes, with some slight
// gamma correction for the dim colors to compensate for bright X screens.
// It contains the 8 ansiterm/xterm colors in 2 intensities.
{
  // Fixme: could add faint colors here, also.
  // normal
  ColorEntry(Color{0x00,0x00,0x00}, false), ColorEntry( Color{0xB2,0xB2,0xB2}, true), // Dfore, Dback
  ColorEntry(Color{0x00,0x00,0x00}, false), ColorEntry( Color{0xB2,0x18,0x18}, false), // Black, Red
  ColorEntry(Color{0x18,0xB2,0x18}, false), ColorEntry( Color{0xB2,0x68,0x18}, false), // Green, Yellow
  ColorEntry(Color{0x18,0x18,0xB2}, false), ColorEntry( Color{0xB2,0x18,0xB2}, false), // Blue, Magenta
  ColorEntry(Color{0x18,0xB2,0xB2}, false), ColorEntry( Color{0xB2,0xB2,0xB2}, false), // Cyan, White
  // intensiv
  ColorEntry(Color{0x00,0x00,0x00}, false), ColorEntry( Color{0xFF,0xFF,0xFF}, true),
  ColorEntry(Color{0x68,0x68,0x68}, false), ColorEntry( Color{0xFF,0x54,0x54}, false),
  ColorEntry(Color{0x54,0xFF,0x54}, false), ColorEntry( Color{0xFF,0xFF,0x54}, false),
  ColorEntry(Color{0x54,0x54,0xFF}, false), ColorEntry( Color{0xFF,0x54,0xFF}, false),
  ColorEntry(Color{0x54,0xFF,0xFF}, false), ColorEntry( Color{0xFF,0xFF,0xFF}, false)
};

const char* const ColorScheme::colorNames[TABLE_COLORS] =
{
  "Foreground",
  "Background",
  "Color0",
  "Color1",
  "Color2",
  "Color3",
  "Color4",
  "Color5",
  "Color6",
  "Color7",
  "ForegroundIntense",
  "BackgroundIntense",
  "Color0Intense",
  "Color1Intense",
  "Color2Intense",
  "Color3Intense",
  "Color4Intense",
  "Color5Intense",
  "Color6Intense",
  "Color7Intense"
};
// dummy silently comment out the tr_NOOP
#define tr_NOOP
const char* const ColorScheme::translatedColorNames[TABLE_COLORS] =
{
    tr_NOOP("Foreground"),
    tr_NOOP("Background"),
    tr_NOOP("Color 1"),
    tr_NOOP("Color 2"),
    tr_NOOP("Color 3"),
    tr_NOOP("Color 4"),
    tr_NOOP("Color 5"),
    tr_NOOP("Color 6"),
    tr_NOOP("Color 7"),
    tr_NOOP("Color 8"),
    tr_NOOP("Foreground (Intense)"),
    tr_NOOP("Background (Intense)"),
    tr_NOOP("Color 1 (Intense)"),
    tr_NOOP("Color 2 (Intense)"),
    tr_NOOP("Color 3 (Intense)"),
    tr_NOOP("Color 4 (Intense)"),
    tr_NOOP("Color 5 (Intense)"),
    tr_NOOP("Color 6 (Intense)"),
    tr_NOOP("Color 7 (Intense)"),
    tr_NOOP("Color 8 (Intense)")
};

ColorScheme::ColorScheme()
{
    _table = nullptr;
    _randomTable = nullptr;
    _opacity = 1.0;
}
ColorScheme::ColorScheme(const ColorScheme& other)
      : _opacity(other._opacity)
       ,_table(nullptr)
       ,_randomTable(nullptr)
{
    setName(other.name());
    setDescription(other.description());

    if ( other._table != nullptr )
    {
        for ( int i = 0 ; i < TABLE_COLORS ; i++ )
            setColorTableEntry(i,other._table[i]);
    }

    if ( other._randomTable != nullptr )
    {
        for ( int i = 0 ; i < TABLE_COLORS ; i++ )
        {
            const RandomizationRange& range = other._randomTable[i];
            setRandomizationRange(i,range.hue,range.saturation,range.value);
        }
    }
}
ColorScheme::~ColorScheme()
{
    delete[] _table;
    delete[] _randomTable;
}

void ColorScheme::setDescription(const std::string& description) { _description = description; }
std::string ColorScheme::description() const { return _description; }

void ColorScheme::setName(const std::string& name) { _name = name; }
std::string ColorScheme::name() const { return _name; }

void ColorScheme::setColorTableEntry(int index , const ColorEntry& entry)
{
    assert( index >= 0 && index < TABLE_COLORS );

    if ( !_table )
    {
        _table = new ColorEntry[TABLE_COLORS];

        for (int i=0;i<TABLE_COLORS;i++)
            _table[i] = defaultTable[i];
    }

    _table[index] = entry;
}
// QColor's HSV, in whole numbers: hue 0-359 (-1 for grey), saturation and value 0-255.
static void toHsv(const Color& color, int& hue, int& saturation, int& value)
{
    const int r = color.r, g = color.g, b = color.b;
    const int max = std::max({r, g, b});
    const int min = std::min({r, g, b});
    const int delta = max - min;
    value = max;
    saturation = max == 0 ? 0 : (255 * delta + max / 2) / max;
    if (delta == 0) {
        hue = -1;
        return;
    }
    double h;
    if (max == r)
        h = double(g - b) / delta;
    else if (max == g)
        h = 2 + double(b - r) / delta;
    else
        h = 4 + double(r - g) / delta;
    h *= 60;
    if (h < 0)
        h += 360;
    hue = int(std::lround(h)) % 360;
}

static Color fromHsv(int hue, int saturation, int value)
{
    if (saturation == 0 || hue < 0)
        return Color{uint8_t(value), uint8_t(value), uint8_t(value)};
    const double h = (hue % 360) / 60.0;
    const double s = saturation / 255.0;
    const double v = value / 255.0;
    const int i = int(h);
    const double f = h - i;
    const double p = v * (1 - s);
    const double q = v * (1 - s * f);
    const double t = v * (1 - s * (1 - f));
    double r, g, b;
    switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
    }
    return Color{uint8_t(std::lround(r * 255)), uint8_t(std::lround(g * 255)), uint8_t(std::lround(b * 255))};
}

static int randomBounded(int highest)
{
    static std::mt19937 generator{std::random_device{}()};
    return std::uniform_int_distribution<int>(0, highest - 1)(generator);
}

ColorEntry ColorScheme::colorEntry(int index) const
{
    assert( index >= 0 && index < TABLE_COLORS );

    ColorEntry entry = colorTable()[index];

    if ( _randomTable != nullptr &&
        !_randomTable[index].isNull() )
    {
        const RandomizationRange& range = _randomTable[index];


        int hueDifference = range.hue ? randomBounded(range.hue) - range.hue/2 : 0;
        int saturationDifference = range.saturation ? randomBounded(range.saturation) - range.saturation/2 : 0;
        int valueDifference = range.value ? randomBounded(range.value) - range.value/2 : 0;

        Color& color = entry.color;
        int hue, saturation, value;
        toHsv(color, hue, saturation, value);

        int newHue = std::abs( (hue + hueDifference) % MAX_HUE );
        int newValue = kMin( std::abs(value + valueDifference) , 255 );
        int newSaturation = kMin( std::abs(saturation + saturationDifference) , 255 );

        color = fromHsv(newHue,newSaturation,newValue);
    }

    return entry;
}
void ColorScheme::getColorTable(ColorEntry* table) const
{
    for ( int i = 0 ; i < TABLE_COLORS ; i++ )
        table[i] = colorEntry(i);
}
bool ColorScheme::randomizedBackgroundColor() const
{
    return _randomTable == nullptr ? false : !_randomTable[1].isNull();
}
void ColorScheme::setRandomizedBackgroundColor(bool randomize)
{
    // the hue of the background colour is allowed to be randomly
    // adjusted as much as possible.
    //
    // the value and saturation are left alone to maintain read-ability
    if ( randomize )
    {
        setRandomizationRange( 1 /* background color index */ , MAX_HUE , 255 , 0 );
    }
    else
    {
        if ( _randomTable )
            setRandomizationRange( 1 /* background color index */ , 0 , 0 , 0 );
    }
}

void ColorScheme::setRandomizationRange( int index , uint16_t hue , uint8_t saturation ,
                                         uint8_t value )
{
    assert( hue <= MAX_HUE );
    assert( index >= 0 && index < TABLE_COLORS );

    if ( _randomTable == nullptr )
        _randomTable = new RandomizationRange[TABLE_COLORS];

    _randomTable[index].hue = hue;
    _randomTable[index].value = value;
    _randomTable[index].saturation = saturation;
}

const ColorEntry* ColorScheme::colorTable() const
{
    if ( _table )
        return _table;
    else
        return defaultTable;
}
Color ColorScheme::foregroundColor() const
{
    return colorTable()[0].color;
}
Color ColorScheme::backgroundColor() const
{
    return colorTable()[1].color;
}
bool ColorScheme::hasDarkBackground() const
{
    // value can range from 0 - 255, with larger values indicating higher brightness.
    // so 127 is in the middle, anything less is deemed 'dark'
    int hue, saturation, value;
    toHsv(backgroundColor(), hue, saturation, value);
    return value < 127;
}
void ColorScheme::setOpacity(double opacity) { _opacity = opacity; }
double ColorScheme::opacity() const { return _opacity; }

void ColorScheme::read(const std::string & fileName)
{
    Lingmo::base::Settings s(fileName);

    // An INI [General] section reads as the top level, not as a group.
    _description = s.stringValue("Description", "Un-named Color Scheme");
    _opacity = s.doubleValue("Opacity", 1.0);

    for (int i=0 ; i < TABLE_COLORS ; i++)
    {
        readColorEntry(&s, i);
    }
}
#if 0
// implemented upstream - user apps
void ColorScheme::read(KConfig& config)
{
    KConfigGroup configGroup = config.group("General");

    QString description = configGroup.readEntry("Description", QObject::tr("Un-named Color Scheme"));

    _description = tr(description.toUtf8());
    _opacity = configGroup.readEntry("Opacity",qreal(1.0));

    for (int i=0 ; i < TABLE_COLORS ; i++)
    {
        readColorEntry(config,i);
    }
}
void ColorScheme::write(KConfig& config) const
{
    KConfigGroup configGroup = config.group("General");

    configGroup.writeEntry("Description",_description);
    configGroup.writeEntry("Opacity",_opacity);

    for (int i=0 ; i < TABLE_COLORS ; i++)
    {
        RandomizationRange random = _randomTable != 0 ? _randomTable[i] : RandomizationRange();
        writeColorEntry(config,colorNameForIndex(i),colorTable()[i],random);
    }
}
#endif

std::string ColorScheme::colorNameForIndex(int index)
{
    assert( index >= 0 && index < TABLE_COLORS );

    return std::string(colorNames[index]);
}
std::string ColorScheme::translatedColorNameForIndex(int index)
{
    assert( index >= 0 && index < TABLE_COLORS );

    return std::string(translatedColorNames[index]);
}

void ColorScheme::readColorEntry(Lingmo::base::Settings * s , int index)
{
    const std::string colorName = colorNameForIndex(index);
    const std::string group = colorName + "/";

    ColorEntry entry;

    std::string colorStr;
    int r = 0, g = 0, b = 0;
    bool ok = false;
    // A value with commas reads as a list, others as a string.
    static const std::string notString = "\x01";
    const std::string colorValue = s->stringValue(group + "Color", notString);
    if (colorValue == notString && s->contains(group + "Color"))
    {
        const std::vector<std::string> rgbList = s->stringListValue(group + "Color");
        colorStr = strings::join(rgbList, ",");
        if (rgbList.size() == 3)
        {
            const auto red = strings::toNumber<int>(rgbList[0]);
            const auto green = strings::toNumber<int>(rgbList[1]);
            const auto blue = strings::toNumber<int>(rgbList[2]);
            r = red.value_or(0);
            g = green.value_or(0);
            b = blue.value_or(0);
            ok = red && (r >= 0 && r <= 0xff) && green && (g >= 0 && g <= 0xff) && blue && (b >= 0 && b <= 0xff);
        }
    }
    else
    {
        colorStr = colorValue == notString ? std::string() : colorValue;
        const bool hex = colorStr.size() == 7 && colorStr[0] == '#'
                && std::all_of(colorStr.begin() + 1, colorStr.end(), [](char c) { return std::isxdigit((unsigned char)c); });
        if (hex)
        {
            r = strings::toNumber<int>(colorStr.substr(1, 2), 16).value_or(0);
            g = strings::toNumber<int>(colorStr.substr(3, 2), 16).value_or(0);
            b = strings::toNumber<int>(colorStr.substr(5, 2), 16).value_or(0);
            ok = true;
        }
    }

    if (!ok)
    {
        LM_WARNING(qtermwidgetLogger) << "Invalid color value" << colorStr << "for" << colorName << ". Fallback to black.";
        r = g = b = 0;
    }
    entry.color = Color{uint8_t(r), uint8_t(g), uint8_t(b)};

    entry.transparent = s->boolValue(group + "Transparent",false);

    // Deprecated key from KDE 4.0 which set 'Bold' to true to force
    // a color to be bold or false to use the current format
    //
    // TODO - Add a new tri-state key which allows for bold, normal or
    // current format
    if (s->contains(group + "Bold"))
        entry.fontWeight = s->boolValue(group + "Bold",false) ? ColorEntry::Bold :
                                                                 ColorEntry::UseCurrentFormat;

    uint16_t hue = uint16_t(s->intValue(group + "MaxRandomHue",0));
    uint8_t value = uint8_t(s->intValue(group + "MaxRandomValue",0));
    uint8_t saturation = uint8_t(s->intValue(group + "MaxRandomSaturation",0));

    setColorTableEntry( index , entry );

    if ( hue != 0 || value != 0 || saturation != 0 )
       setRandomizationRange( index , hue , saturation , value );
}
#if 0
// implemented upstream - user apps
void ColorScheme::writeColorEntry(KConfig& config , const QString& colorName, const ColorEntry& entry , const RandomizationRange& random) const
{
    KConfigGroup configGroup(&config,colorName);

    configGroup.writeEntry("Color",entry.color);
    configGroup.writeEntry("Transparency",(bool)entry.transparent);
    if (entry.fontWeight != ColorEntry::UseCurrentFormat)
    {
        configGroup.writeEntry("Bold",entry.fontWeight == ColorEntry::Bold);
    }

    // record randomization if this color has randomization or
    // if one of the keys already exists
    if ( !random.isNull() || configGroup.hasKey("MaxRandomHue") )
    {
        configGroup.writeEntry("MaxRandomHue",static_cast<int>(random.hue));
        configGroup.writeEntry("MaxRandomValue",static_cast<int>(random.value));
        configGroup.writeEntry("MaxRandomSaturation",static_cast<int>(random.saturation));
    }
}
#endif

//
// Work In Progress - A color scheme for use on KDE setups for users
// with visual disabilities which means that they may have trouble
// reading text with the supplied color schemes.
//
// This color scheme uses only the 'safe' colors defined by the
// KColorScheme class.
//
// A complication this introduces is that each color provided by
// KColorScheme is defined as a 'background' or 'foreground' color.
// Only foreground colors are allowed to be used to render text and
// only background colors are allowed to be used for backgrounds.
//
// The ColorEntry and TerminalDisplay classes do not currently
// support this restriction.
//
// Requirements:
//  - A color scheme which uses only colors from the KColorScheme class
//  - Ability to restrict which colors the TerminalDisplay widget
//    uses as foreground and background color
//  - Make use of KGlobalSettings::allowDefaultBackgroundImages() as
//    a hint to determine whether this accessible color scheme should
//    be used by default.
//
//
// -- Robert Knight <robertknight@gmail.com> 21/07/2007
//
AccessibleColorScheme::AccessibleColorScheme()
    : ColorScheme()
{
#if 0
// It's not finished in konsole and it breaks Qt4 compilation as well
    // basic attributes
    setName("accessible");
    setDescription(QObject::tr("Accessible Color Scheme"));

    // setup colors
    const int ColorRoleCount = 8;

    const KColorScheme colorScheme(QPalette::Active);

    QBrush colors[ColorRoleCount] =
    {
        colorScheme.foreground( colorScheme.NormalText ),
        colorScheme.background( colorScheme.NormalBackground ),

        colorScheme.foreground( colorScheme.InactiveText ),
        colorScheme.foreground( colorScheme.ActiveText ),
        colorScheme.foreground( colorScheme.LinkText ),
        colorScheme.foreground( colorScheme.VisitedText ),
        colorScheme.foreground( colorScheme.NegativeText ),
        colorScheme.foreground( colorScheme.NeutralText )
    };

    for ( int i = 0 ; i < TABLE_COLORS ; i++ )
    {
        ColorEntry entry;
        entry.color = colors[ i % ColorRoleCount ].color();

        setColorTableEntry( i , entry );
    }
#endif
}

ColorSchemeManager::ColorSchemeManager()
    : _haveLoadedAll(false)
{
}
ColorSchemeManager::~ColorSchemeManager()
{
    for (auto& scheme : _colorSchemes)
        delete scheme.second;
}
void ColorSchemeManager::loadAllColorSchemes()
{
    int failed = 0;

    for (const std::string& path : listColorSchemes())
    {
        if ( !loadColorScheme( path ) )
            failed++;
    }

    if ( failed > 0 )
        LM_DEBUG(qtermwidgetLogger) << "failed to load " << failed << " color schemes.";

    _haveLoadedAll = true;
}
std::vector<const ColorScheme*> ColorSchemeManager::allColorSchemes()
{
    if ( !_haveLoadedAll )
    {
        loadAllColorSchemes();
    }

    std::vector<const ColorScheme*> schemes;
    for (const auto& scheme : _colorSchemes)
        schemes.push_back(scheme.second);
    return schemes;
}
#if 0
void ColorSchemeManager::addColorScheme(ColorScheme* scheme)
{
    _colorSchemes.insert(scheme->name(),scheme);

    // save changes to disk
    QString path = KGlobal::dirs()->saveLocation("data","konsole/") + scheme->name() + ".colorscheme";
    KConfig config(path , KConfig::NoGlobals);

    scheme->write(config);
}
#endif

bool ColorSchemeManager::loadCustomColorScheme(const std::string& path)
{
    if (strings::endsWith(path, ".colorscheme"))
        return loadColorScheme(path);

    return false;
}

void ColorSchemeManager::addCustomColorSchemeDir(const std::string& custom_dir)
{
    add_custom_color_scheme_dir(custom_dir);
}

bool ColorSchemeManager::loadColorScheme(const std::string& filePath)
{
    std::error_code error;
    if ( !strings::endsWith(filePath, ".colorscheme") || !std::filesystem::exists(filePath, error) )
        return false;

    // QFileInfo::baseName(): the file name up to its first dot.
    const std::string fileName = std::filesystem::path(filePath).filename().string();
    const std::string schemeName = fileName.substr(0, fileName.find('.'));

    ColorScheme* scheme = new ColorScheme();
    scheme->setName(schemeName);
    scheme->read(filePath);

    if (scheme->name().empty())
    {
        delete scheme;
        return false;
    }

    if ( !_colorSchemes.count(schemeName) )
    {
        _colorSchemes.emplace(schemeName,scheme);
    }
    else
    {
        delete scheme;
    }

    return true;
}
std::vector<std::string> ColorSchemeManager::listColorSchemes()
{
    std::vector<std::string> ret;
    for (const std::string &scheme_dir : get_color_schemes_dirs())
    {
        // QDir::entryList(): sorted by name.
        std::vector<std::string> list;
        std::error_code error;
        for (const auto& entry : std::filesystem::directory_iterator(scheme_dir, error))
        {
            const std::string name = entry.path().filename().string();
            if (strings::endsWith(name, ".colorscheme"))
                list.push_back(name);
        }
        std::sort(list.begin(), list.end());
        for (const std::string &i : list)
            ret.push_back(scheme_dir + '/' + i);
    }

    return ret;
}
const ColorScheme ColorSchemeManager::_defaultColorScheme;
const ColorScheme* ColorSchemeManager::defaultColorScheme() const
{
    return &_defaultColorScheme;
}
bool ColorSchemeManager::deleteColorScheme(const std::string& name)
{
    assert( _colorSchemes.count(name) );

    // lookup the path and delete
    std::string path = findColorSchemePath(name);
    std::error_code error;
    if ( std::filesystem::remove(path, error) )
    {
        delete _colorSchemes[name];
        _colorSchemes.erase(name);
        return true;
    }
    else
    {
        return false;
    }
}
std::string ColorSchemeManager::findColorSchemePath(const std::string& name) const
{
    const std::vector<std::string> dirs = get_color_schemes_dirs();
    if ( dirs.empty() )
        return std::string();

    const std::string dir = dirs.front();
    std::string path(dir + '/' + name + ".colorscheme");
    if ( !path.empty() )
        return path;

    path = dir + '/' + name + ".schema";

    return path;
}
const ColorScheme* ColorSchemeManager::findColorScheme(const std::string& name)
{
    if ( name.empty() )
        return defaultColorScheme();

    if ( auto found = _colorSchemes.find(name); found != _colorSchemes.end() )
        return found->second;
    else
    {
        // look for this color scheme
        std::string path = findColorSchemePath(name);
        if ( !path.empty() && loadColorScheme(path) )
        {
            return findColorScheme(name);
        }

        return nullptr;
    }
}
ColorSchemeManager* ColorSchemeManager::instance()
{
    static ColorSchemeManager theColorSchemeManager;
    return &theColorSchemeManager;
}

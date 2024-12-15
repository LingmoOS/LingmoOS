// This file is part of the SpeedCrunch project
// Copyright (C) 2004, 2005, 2007, 2008 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2005-2006 Johan Thelin <e8johan@gmail.com>
// Copyright (C) 2007-2016 @heldercorreia
// Copyright (C) 2015 Pol Welter <polwelter@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "settings.h"

#include "../math/floatconfig.h"

#include <QDir>
#include <QLocale>
#include <QSettings>
#include <QApplication>
#include <QFont>
#include <QtCore/QStandardPaths>

#ifdef Q_OS_WIN
# define WIN32_LEAN_AND_MEAN
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# include <windows.h>
# include <shlobj.h>
#endif


// The current config revision. Based on the application version number
// ('1200' corresponds to 0.12.0, '10300' would be 1.3.0 etc.). When making
// a backwards-incompatible change to the config format, bump this number to
// the next release (if not already happened), then update the migration code
// in createQSettings. Don't bump the config version unnecessarily for
// releases that don't contain incompatible changes.
static const int ConfigVersion = 1200;


static const char *DefaultColorScheme = "Terminal";

QString Settings::getConfigPath()
{
#ifdef SPEEDCRUNCH_PORTABLE
    return QApplication::applicationDirPath();
#elif defined(Q_OS_WIN)
    // On Windows, use AppData/Roaming/SpeedCrunch, the same path as getDataPath.
    return getDataPath();
#else
    // Everywhere else, use `QStandardPaths::ConfigLocation`/SpeedCrunch:
    // * OSX: ~/Library/Preferences/SpeedCrunch
    // * Linux: ~/.config/SpeedCrunch
    return QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
                                QCoreApplication::applicationName());
#endif
}

QString Settings::getDataPath()
{
#ifdef SPEEDCRUNCH_PORTABLE
    return QApplication::applicationDirPath();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#elif defined(Q_OS_WIN)
    // We can't use AppDataLocation, so we simply use the Win32 API to emulate it.
    WCHAR w32path[MAX_PATH];
    HRESULT result = SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, w32path);
    Q_ASSERT(SUCCEEDED(result));
    QString path = QString::fromWCharArray(w32path);
    QString orgName = QCoreApplication::organizationName();
    QString appName = QCoreApplication::applicationName();
    if (!orgName.isEmpty()) {
        path.append('\\');
        path.append(orgName);
    }
    if (!appName.isEmpty()) {
        path.append('\\');
        path.append(appName);
    }
    return QDir::fromNativeSeparators(path);
#else
    // Any non-Windows with Qt < 5.4. Since DataLocation and AppDataLocation are
    // equivalent outside of Windows, that should be fine.
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#endif
}

QString Settings::getCachePath()
{
#ifdef SPEEDCRUNCH_PORTABLE
    return QApplication::applicationDirPath();
#else
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#endif
}

static Settings *s_settingsInstance = 0;
static char s_radixCharacter = 0;

static void s_deleteSettings()
{
    delete s_settingsInstance;
}

static QSettings *createQSettings(const QString &key);

Settings *Settings::instance()
{
    if (!s_settingsInstance) {
        s_settingsInstance = new Settings;
        s_settingsInstance->load();
        qAddPostRoutine(s_deleteSettings);
    }

    return s_settingsInstance;
}

Settings::Settings()
{
}

void Settings::load()
{
    const char *KEY = "SpeedCrunch";

    QSettings *settings = createQSettings(KEY);
    if (!settings)
        return;

    QString key = QString::fromLatin1(KEY) + QLatin1String("/General/");

    // Angle mode special case.
    QString angleUnitStr;
    angleUnitStr = settings->value(key + QLatin1String("AngleMode"), "r").toString();
    if (angleUnitStr != QLatin1String("r") && angleUnitStr != QLatin1String("d") && angleUnitStr != QLatin1String("g"))
        angleUnit = 'r';
    else
        angleUnit = angleUnitStr.at(0).toLatin1();

    //进制转换 add jingzhou 20201104，默认进制是10进制
    programmerBase = settings->value(key + QLatin1String("ProgrammerBase"), 0).toInt();
    //改变数据类型 add jingzhou 20201119,默认为QWORD
    proBitLength = settings->value(key + QLatin1String("proBitLength"), 64).toInt();
    //带进位循环移位标志位 add jingzhou 20201205,默认为0
    proRotateCarry = settings->value(key + QLatin1String("proRotateCarry"), "00").toString();

    // Radix character special case.
    QString radixCharStr;
    radixCharStr = settings->value(key + QLatin1String("RadixCharacter"), "*").toString();
    setRadixCharacter(radixCharStr.at(0).toLatin1());

    autoAns = settings->value(key + QLatin1String("AutoAns"), true).toBool();
    autoCalc = settings->value(key + QLatin1String("AutoCalc"), true).toBool();
    autoCompletion = settings->value(key + QLatin1String("AutoCompletion"), true).toBool();
    sessionSave = settings->value(key + QLatin1String("SessionSave"), true).toBool();
    leaveLastExpression = settings->value(key + QLatin1String("LeaveLastExpression"), false).toBool();
    language = settings->value(key + QLatin1String("Language"), "C").toString();
    syntaxHighlighting = settings->value(key + QLatin1String("SyntaxHighlighting"), true).toBool();
    autoResultToClipboard = settings->value(key + QLatin1String("AutoResultToClipboard"), false).toBool();
    windowPositionSave = settings->value(key + QLatin1String("WindowPositionSave"), true).toBool();
    complexNumbers = settings->value(key + QLatin1String("ComplexNumbers"), false).toBool();

    digitGrouping = settings->value(key + QLatin1String("DigitGrouping"), 0).toInt();
    digitGrouping = std::min(3, std::max(0, digitGrouping));

    key = KEY + QLatin1String("/Format/");

    // Format special case.
    QString format;
    format = settings->value(key + QLatin1String("Type"), 'f').toString();
    if (format != "g" && format != "f" && format != "e" && format != "n" && format != "h" && format != "o" && format != "b")
        resultFormat = 'f';
    else
        resultFormat = format.at(0).toLatin1();

    // Complex format special case.
    QString cmplxFormat;
    cmplxFormat = settings->value(key + QLatin1String("ComplexForm"), 'c').toString();
    if (cmplxFormat != "c" && cmplxFormat != "p")
        resultFormatComplex = 'c';
    else
        resultFormatComplex = cmplxFormat.at(0).toLatin1();

    resultPrecision = settings->value(key + QLatin1String("Precision"), -1).toInt();

    if (resultPrecision > DECPRECISION)
        resultPrecision = DECPRECISION;

    key = KEY + QLatin1String("/Layout/");
    windowOnfullScreen = settings->value(key + QLatin1String("WindowOnFullScreen"), false).toBool();
    historyDockVisible = settings->value(key + QLatin1String("HistoryDockVisible"), false).toBool();
    keypadVisible = settings->value(key + QLatin1String("KeypadVisible"), false).toBool();
    statusBarVisible = settings->value(key + QLatin1String("StatusBarVisible"), false).toBool();
    functionsDockVisible = settings->value(key + QLatin1String("FunctionsDockVisible"), false).toBool();
    variablesDockVisible = settings->value(key + QLatin1String("VariablesDockVisible"), false).toBool();
    userFunctionsDockVisible = settings->value(key + QLatin1String("UserFunctionsDockVisible"), false).toBool();
    formulaBookDockVisible = settings->value(key + QLatin1String("FormulaBookDockVisible"), false).toBool();
    constantsDockVisible = settings->value(key + QLatin1String("ConstantsDockVisible"), false).toBool();
    windowAlwaysOnTop = settings->value(key + QLatin1String("WindowAlwaysOnTop"), false).toBool();
    bitfieldVisible = settings->value(key + QLatin1String("BitfieldVisible"), false).toBool();

    windowState = settings->value(key + QLatin1String("State")).toByteArray();
    windowGeometry = settings->value(key + QLatin1String("WindowGeometry")).toByteArray();
    manualWindowGeometry = settings->value(key + QLatin1String("ManualWindowGeometry")).toByteArray();

    key = KEY + QLatin1String("/Display/");
    displayFont = settings->value(key + QLatin1String("DisplayFont"), QFont().toString()).toString();
    colorScheme = settings->value(key + QLatin1String("ColorSchemeName"), DefaultColorScheme).toString();

    delete settings;
}

void Settings::save()
{
    const QString KEY = QString::fromLatin1("SpeedCrunch");

    QSettings *settings = createQSettings(KEY);
    if (!settings)
        return;

    QString key = KEY + QLatin1String("/General/");

    settings->setValue(key + QLatin1String("SessionSave"), sessionSave);
    settings->setValue(key + QLatin1String("LeaveLastExpression"), leaveLastExpression);
    settings->setValue(key + QLatin1String("AutoCompletion"), autoCompletion);
    settings->setValue(key + QLatin1String("AutoAns"), autoAns);
    settings->setValue(key + QLatin1String("AutoCalc"), autoCalc);
    settings->setValue(key + QLatin1String("SyntaxHighlighting"), syntaxHighlighting);
    settings->setValue(key + QLatin1String("DigitGrouping"), digitGrouping);
    settings->setValue(key + QLatin1String("AutoResultToClipboard"), autoResultToClipboard);
    settings->setValue(key + QLatin1String("Language"), language);
    settings->setValue(key + QLatin1String("WindowPositionSave"), windowPositionSave);
    settings->setValue(key + QLatin1String("ComplexNumbers"), complexNumbers);

    settings->setValue(key + QLatin1String("AngleMode"), QString(QChar(angleUnit)));
    settings->setValue(key + QLatin1String("ProgrammerBase"), programmerBase);
    settings->setValue(key + QLatin1String("proBitLength"), proBitLength);
    settings->setValue(key + QLatin1String("proRotateCarry"), proRotateCarry);

    char c = 'C';
    if (s_radixCharacter != 0)
        c = s_radixCharacter;
    settings->setValue(key + QLatin1String("RadixCharacter"), QString(QChar(c)));

    key = KEY + QLatin1String("/Format/");

    settings->setValue(key + QLatin1String("Type"), QString(QChar(resultFormat)));
    settings->setValue(key + QLatin1String("ComplexForm"), QString(QChar(resultFormatComplex)));
    settings->setValue(key + QLatin1String("Precision"), resultPrecision);

    key = KEY + QLatin1String("/Layout/");

    settings->setValue(key + QLatin1String("FormulaBookDockVisible"), formulaBookDockVisible);
    settings->setValue(key + QLatin1String("ConstantsDockVisible"), constantsDockVisible);
    settings->setValue(key + QLatin1String("FunctionsDockVisible"), functionsDockVisible);
    settings->setValue(key + QLatin1String("HistoryDockVisible"), historyDockVisible);
    settings->setValue(key + QLatin1String("WindowOnFullScreen"), windowOnfullScreen);
    settings->setValue(key + QLatin1String("KeypadVisible"), keypadVisible);
    settings->setValue(key + QLatin1String("StatusBarVisible"), statusBarVisible);
    settings->setValue(key + QLatin1String("VariablesDockVisible"), variablesDockVisible);
    settings->setValue(key + QLatin1String("UserFunctionsDockVisible"), userFunctionsDockVisible);
    settings->setValue(key + QLatin1String("WindowAlwaysOnTop"), windowAlwaysOnTop);
    settings->setValue(key + QLatin1String("State"), windowState);
    settings->setValue(key + QLatin1String("WindowGeometry"), windowGeometry);
    settings->setValue(key + QLatin1String("ManualWindowGeometry"), manualWindowGeometry);
    settings->setValue(key + QLatin1String("BitfieldVisible"), bitfieldVisible);

    key = KEY + QLatin1String("/Display/");

    settings->setValue(key + QLatin1String("DisplayFont"), displayFont);
    settings->setValue(key + QLatin1String("ColorSchemeName"), colorScheme);


    delete settings;
}

char Settings::radixCharacter() const
{
    if (isRadixCharacterAuto() || isRadixCharacterBoth())
        return QLocale().decimalPoint().toLatin1();

    return s_radixCharacter;
}

bool Settings::isRadixCharacterAuto() const
{
    return s_radixCharacter == 0;
}

bool Settings::isRadixCharacterBoth() const
{
    return s_radixCharacter == '*';
}

void Settings::setRadixCharacter(char c)
{
    s_radixCharacter = (c != ',' && c != '.' && c != '*') ? 0 : c;
}


// Settings migration from legacy (0.11 and before) to 0.12 (ConfigVersion 1200).
static void migrateSettings_legacyTo1200(QSettings *settings, const QString &KEY)
{
#ifdef SPEEDCRUNCH_PORTABLE
    // This is the same as the new path, but let's cut down
    QSettings *legacy = new QSettings(Settings::getConfigPath() + "/" + KEY + ".ini", QSettings::IniFormat);
#else
    QSettings *legacy = new QSettings(QSettings::NativeFormat, QSettings::UserScope, KEY, KEY);
#endif

    QString legacyFile = legacy->fileName();
    if (legacyFile != settings->fileName()) {
        // If the file names are different, we assume the legacy settings were in a
        // different location (most were, but e.g. not on the portable version) so we
        // copy everything over, then delete the old settings. On Windows, the file
        // name may also be a registry path, but the same reasoning applies.
        for (auto &key : legacy->allKeys())
            settings->setValue(key, legacy->value(key));

#ifdef Q_OS_WIN
        // On Windows, check if the legacy settings were in the registry; if so, just clear()
        // them.
        if (legacy->format() == QSettings::NativeFormat) {
            legacy->clear();
            delete legacy;
        } else {
            delete legacy;
            QFile::remove(legacyFile);
        }
#else
        // On other platforms, delete the legacy settings file.
        delete legacy;
        QFile::remove(legacyFile);
#endif
    } else {
        // If both settings objects point to the same location, removing the old stuff
        // may well break the new settings.
        delete legacy;
    }


    // ColorScheme -> ColorSchemeName
    QString colorSchemeName;
    switch (settings->value("SpeedCrunch/Display/ColorScheme", -1).toInt()) {
    case 0:
        colorSchemeName = "Terminal";
        break;
    case 1:
        colorSchemeName = "Standard";
        break;
    case 2:
        colorSchemeName = "Sublime";
        break;
    default:
        colorSchemeName = DefaultColorScheme;
        break;
    }
    settings->setValue("SpeedCrunch/Display/ColorSchemeName", colorSchemeName);
    settings->remove("SpeedCrunch/Display/ColorScheme");

    // DigitGrouping (bool) -> DigitGrouping (int)
    bool groupDigits = settings->value("SpeedCrunch/General/DigitGrouping", false).toBool();
    settings->setValue("SpeedCrunch/General/DigitGrouping", groupDigits ? 1 : 0);
}


QSettings *createQSettings(const QString &KEY)
{
    QSettings *settings = new QSettings(Settings::getConfigPath() + "/" + KEY + ".ini", QSettings::IniFormat);
    int ver = settings->value("ConfigVersion", 0).toInt();
    switch (ver) {
    case 0:
        migrateSettings_legacyTo1200(settings, KEY);
        break;
    }
    settings->setValue("ConfigVersion", ConfigVersion);
    return settings;
}

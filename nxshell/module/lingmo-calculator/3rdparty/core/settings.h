// This file is part of the SpeedCrunch project
// Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
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

#ifndef CORE_SETTINGS_H
#define CORE_SETTINGS_H

#include <QtCore/QPoint>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QList>

class Settings
{
public:
    static Settings *instance();
    static QString getConfigPath();
    static QString getDataPath();
    static QString getCachePath();

    void load();
    void save();

    char radixCharacter() const; // 0 or '*': Automatic.
    void setRadixCharacter(char c = 0);
    bool isRadixCharacterAuto() const;
    bool isRadixCharacterBoth() const;

    bool complexNumbers;

    char angleUnit; // 'r': radian; 'd': degree; 'g': gradian.
    int programmerBase; //进制转换-2,8,10,16,非程序员计算器为0
    int proBitLength; //数据的长度byte-8 word-16 Dword-32 Qword-64
    QString proRotateCarry; //带进位循环移位标志位

    char resultFormat;
    int resultPrecision; // See HMath documentation.
    char resultFormatComplex; // 'c' cartesian; 'p' polar.

    bool autoAns;
    bool autoCalc;
    bool autoCompletion;
    int digitGrouping;
    bool sessionSave;
    bool leaveLastExpression;
    bool syntaxHighlighting;
    bool windowAlwaysOnTop;
    bool autoResultToClipboard;
    bool windowPositionSave;

    bool constantsDockVisible;
    bool functionsDockVisible;
    bool historyDockVisible;
    bool keypadVisible;
    bool formulaBookDockVisible;
    bool statusBarVisible;
    bool variablesDockVisible;
    bool userFunctionsDockVisible;
    bool windowOnfullScreen;
    bool bitfieldVisible;

    QString colorScheme;
    QString displayFont;

    QString language;

    QByteArray windowState;
    QByteArray windowGeometry;
    QByteArray manualWindowGeometry;

private:
    Q_DISABLE_COPY(Settings)
    Settings();
};

#endif

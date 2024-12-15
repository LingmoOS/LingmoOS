/*
 Copyright (C) 2006 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _KKEYSERVER_MAC_H
#define _KKEYSERVER_MAC_H

#include <kwindowsystem_export.h>

#include <QList>

namespace KKeyServer
{
/**
 * Extracts the symbol from the given Qt key, and converts it to an OSX symbol.
 * @param keyQt the qt key code
 * @param sym if successful, the symbol will be written here
 * @return true if successful, false otherwise
 */
KWINDOWSYSTEM_EXPORT bool keyQtToSymMac(int keyQt, int &sym);

/**
 * Extracts all the scancodes from the given Qt key. The returned values can change if a different
 * keyboard layout is selected.
 * @param keyQt the qt key code
 * @param keyCodes if successful, a list of scancodes will be written here
 * @return true if successful, false otherwise
 */
KWINDOWSYSTEM_EXPORT bool keyQtToCodeMac(int keyQt, QList<uint> &keyCodes);

/**
 * Extracts the modifiers from the given Qt key and converts them in a mask of OSX modifiers.
 * @param keyQt the qt key code
 * @param mod if successful, the modifiers will be written here
 * @return true if successful, false otherwise
 */
KWINDOWSYSTEM_EXPORT bool keyQtToModMac(int keyQt, uint &mod);
}

#endif // !_KKEY_SERVER_MAC_H


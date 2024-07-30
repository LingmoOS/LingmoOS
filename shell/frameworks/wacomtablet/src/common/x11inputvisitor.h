/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef X11INPUTVISITOR_H
#define X11INPUTVISITOR_H

#include "x11inputdevice.h"

namespace Wacom
{
/**
 * @brief XInput device visitor. Used by X11TabletFinder class.
 *
 * This class allows easy access to xinput devices when using
 * \a X11Input::scanDevices().
 */
class X11InputVisitor
{
public:
    virtual ~X11InputVisitor()
    {
    }

    /**
     * Called by X11Input when iterating the X11 input devices.
     *
     * @return True to end the current iteration, else false.
     */
    virtual bool visit(X11InputDevice &device) = 0;
};
}
#endif

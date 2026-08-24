/*
 * SPDX-FileCopyrightText: 2022 Paul A McAuley <kde@paulmcauley.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "stylelingmo.h"

namespace Breeze
{

void RenderStyleLingmo18By18::renderCloseIcon()
{
    renderCloseIconAtSquareMaximizeSize();
}

void RenderStyleLingmo18By18::renderMaximizeIcon()
{
    renderSquareMaximizeIcon(false);
}

void RenderStyleLingmo18By18::renderRestoreIcon()
{
    renderOverlappingWindowsIcon();
}

void RenderStyleLingmo18By18::renderMinimizeIcon()
{
    renderTinySquareMinimizeIcon();
}


// For consistency with breeze icon set
void RenderStyleLingmo18By18::renderKeepBehindIcon()
{
    renderKeepBehindIconAsFromBreezeIcons();
}

void RenderStyleLingmo18By18::renderKeepInFrontIcon()
{
    renderKeepInFrontIconAsFromBreezeIcons();
}

void RenderStyleLingmo18By18::renderContextHelpIcon()
{
    renderRounderAndBolderContextHelpIcon();
}

}

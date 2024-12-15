// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "scissorwindow.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(ScissorWindow,
                              "metadata.json.stripped",
                              return ScissorWindow::supported();)

} // namespace KWin

#include "main.moc"

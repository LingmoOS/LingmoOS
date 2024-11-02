/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <lingmosdk/applications/kaboutdialog.h>
#include "about.h"

About::About(QWidget *parent) :
    kdk::KAboutDialog(parent)

{
    // ui->setupUi(this);

    setAppName(tr("Notes"));
    setAppIcon(QIcon::fromTheme("lingmo-notebook"));
    setAppVersion("3.2.1.1");
}

About::~About()
{

}
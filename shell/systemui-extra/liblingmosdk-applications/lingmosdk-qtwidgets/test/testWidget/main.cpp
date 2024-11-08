/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include <QApplication>
#include <QStyleFactory>
#include <QTranslator>
#include <QProgressDialog>
#include "testwidget.h"
#include "kwidget.h"
#include "kprogressdialog.h"
#include "kdialog.h"
#include "kwidgetutils.h"

using namespace kdk;
int main(int argc, char *argv[])
{
    kdk::KWidgetUtils::highDpiScaling();
    QApplication a(argc, argv);
    QTranslator trans;
    QString locale = QLocale::system().name();
    if(locale == "zh_CN")
    {
        if(trans.load(":/translations/gui_zh_CN.qm"))
        {
            a.installTranslator(&trans);
        }
    }
    if(locale == "bo_CN")
    {
        if(trans.load(":/translations/gui_bo_CN.qm"))
        {
            a.installTranslator(&trans);
        }
    }
    TestWidget widget;
    widget.setWidgetName("TestWidget");
    widget.setIcon("lingmo-music");
    widget.show();
    return a.exec();
}

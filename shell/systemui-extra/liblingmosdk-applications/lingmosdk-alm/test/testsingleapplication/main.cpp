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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QApplication>
#include <QDebug>
#include <QWidget>

#include "singleapplication.h"

int main(int argc, char *argv[])
{
    kdk::QtSingleApplication app(argc, argv);

    /* 设置单例 */
    if (app.isRunning()) {
        qDebug() << "app is running";
        app.sendMessage("running", 4000);
        return 0;
    }

    QWidget widget;

    /* 设置最小化拉起 */
    app.setActivationWindow(&widget);

    widget.show();

    return app.exec();
}

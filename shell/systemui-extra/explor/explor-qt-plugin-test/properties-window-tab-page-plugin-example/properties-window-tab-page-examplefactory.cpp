/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "properties-window-tab-page-examplefactory.h"
#include <QLabel>

PropertiesWindowTabPageExampleFactory::PropertiesWindowTabPageExampleFactory(QObject *parent) : QObject (parent)
{
}

PropertiesWindowTabPageExampleFactory::~PropertiesWindowTabPageExampleFactory()
{

}

QWidget *PropertiesWindowTabPageExampleFactory::createTabPage(const QStringList &uris)
{
    QStringList l = uris;
    auto label = new QLabel(tr("This page is an external plugin page.\n"
                               "uris:\n"
                               "%1").arg(l.join(",\n")));
    label->setWordWrap(true);
    return label;
}

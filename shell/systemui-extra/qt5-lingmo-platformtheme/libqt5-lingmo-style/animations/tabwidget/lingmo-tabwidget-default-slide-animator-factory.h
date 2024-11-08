/*
 * Qt5-LINGMO's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef LINGMOTABWIDGETDEFAULTSLIDEANIMATORFACTORY_H
#define LINGMOTABWIDGETDEFAULTSLIDEANIMATORFACTORY_H

#include <QObject>
#include "lingmo-tabwidget-animator-plugin-iface.h"

namespace LINGMO {

namespace TabWidget {

/*!
 * \brief The DefaultSlideAnimatorFactory class
 * \details
 * This class is an internal plugin. It provides a default tabwidget
 * switch animation for QTabWidget and its drived class.
 *
 * \note
 * Note that it used in lingmo-style, but you can also use its api in other
 * desktop environment.
 */
class DefaultSlideAnimatorFactory : public QObject, public LINGMOTabWidgetAnimatorPluginIface
{
    Q_OBJECT
public:
    explicit DefaultSlideAnimatorFactory(QObject *parent = nullptr);
    ~DefaultSlideAnimatorFactory(){}

    const QString id() {return tr("Default Slide");}
    const QString brief() {return tr("Let tab widget switch with a slide animation.");}

    const QString key() {return "tab_slide";}
    const QString description() {return brief();}
    AnimatorPluginType pluginType() {return TabWidget;}

    const QString inhertKey() {return "QTabWidget";}
    const QStringList excludeKeys() {return QStringList()<<"Peony::DirectoryWidget";}
    bool isParallel() {return false;}

    LINGMOTabWidgetAnimatorIface *createAnimator();
};

}

}

#endif // LINGMOTABWIDGETDEFAULTSLIDEANIMATORFACTORY_H

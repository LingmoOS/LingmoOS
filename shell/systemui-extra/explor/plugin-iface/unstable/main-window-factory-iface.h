/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#ifndef MAINWINDOWFACTORYIFACE_H
#define MAINWINDOWFACTORYIFACE_H

#include <QObject>
#include "FMWindowIface.h"
#include "explor-core_global.h"
#include <QMainWindow>

#define MainWindowFactoryIface_iid "org.lingmo.explor-qt.MainWindowFactoryIface"
namespace Peony {

/*!
 * \brief The FMWindowFactory class
 * \deprecated
 */
class PEONYCORESHARED_EXPORT MainWindowFactoryIface
{
public:
    virtual const QString name() = 0;

    ~MainWindowFactoryIface()  {}
    virtual QWidget *createWindow(const QString &uri = nullptr) = 0;
    virtual QWidget *createWindow(const QStringList &uris) = 0;
    virtual QWidget *createWindow(const QString &uri, QStringList selectUris) = 0;

};

}
Q_DECLARE_INTERFACE (Peony::MainWindowFactoryIface, MainWindowFactoryIface_iid)
#endif // FMWINDOWFACTORY_H

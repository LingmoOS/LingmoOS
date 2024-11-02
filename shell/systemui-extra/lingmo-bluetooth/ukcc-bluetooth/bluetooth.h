/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QTimer>
#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QTranslator>
#include <QApplication>
#include <lingmo-log4qt.h>
#include <ukcc/interface/interface.h>

#include "bluetoothmainwindow.h"
#include "ukccbluetoothconfig.h"

class Bluetooth : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)
public:
    Bluetooth();
    ~Bluetooth();

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const          Q_DECL_OVERRIDE;
    bool isEnable() const       Q_DECL_OVERRIDE;

    void plugin_leave()   Q_DECL_OVERRIDE;

    //static bool isIntel_bt = CommonInterface::isIntel();
    //注意：ukcc-bluetooth_%1.ts
    QString translationPath() const {     // 获取多语言文件路径，用于搜索
            qDebug() << Q_FUNC_INFO << QStringLiteral("/usr/share/lingmo-bluetooth/translations/%1.ts");
            return QStringLiteral("/usr/share/lingmo-bluetooth/translations/ukcc-bluetooth_%1.ts");
        }
private:
    QString pluginName;
    int pluginType;
    BlueToothMainWindow *pluginWidget;
    bool mFirstLoad;

};

#endif // BLUETOOTH_H

/***************************************************************************
 *   This file was modified by Uniontech Technology Co., Ltd.              *
 *   Copyright (C) 2011~2011 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef DUMMYCONFIG_H
#define DUMMYCONFIG_H

#include <fcitx-config/fcitx-config.h>
#include <stdio.h>
#include <QMap>
#include <QString>

namespace Fcitx {

/**
 * @class DummyConfig
 * @brief 配置文件读取类
 */
class DummyConfig
{
public:
    DummyConfig();
    ~DummyConfig();
    void setConfigDesc(FcitxConfigFileDesc *cfdesc);

    FcitxGenericConfig *genericConfig();

    /**
     *@brief 读取配置文件，填入结构体
     */
    void load(FILE *fp);

    /**
     *@brief 使用结构体绑定值
     */
    void bind(char *group, char *option, FcitxSyncFilter filter = nullptr, void *arg = nullptr);
    bool isValid();

    /**
     *@brief 文件修改后调用此函数，刷新界面
     */
    void configSyncToUi();

    /**
     *@brief 获取配置文件结构体指针
     */
    FcitxConfigFile *getConfigFile();
private:
    QMap<QString, void *> m_dummyValue;
    FcitxConfigFileDesc *m_cfdesc;
    FcitxConfigFile *m_cfile {nullptr};
    FcitxGenericConfig m_config;
};

}

#endif // DUMMYCONFIG_H

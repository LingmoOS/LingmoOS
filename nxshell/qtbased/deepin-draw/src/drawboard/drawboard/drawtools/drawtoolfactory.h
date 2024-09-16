// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CDRAWTOOLFACTORY_H
#define CDRAWTOOLFACTORY_H

#include "globaldefine.h"
class DrawTool;
/**
 * @brief The DrawToolFactory class
 * 工具创建工厂类
 */
typedef DrawTool *(*generatedInterface)() ;
typedef QMap<int, generatedInterface>  ToolCreatFunMap;
class DRAWLIB_EXPORT DrawToolFactory
{
public:
    /**
     * @brief Create 创建工具对象
     * @param mode 创建的工具类型
     * @return 创建的工具
     */
    static DrawTool *createTool(int mode);

    static void loadAllToolPlugins();

    static bool loadToolPlugin(const QString &pluginPath);

    static void registTool(int toolType, generatedInterface f);

    static ToolCreatFunMap genInterfacesForPluginTool();

private:
    static ToolCreatFunMap  s_pluginToolCreatFuns;
};

#endif // CDRAWTOOLFACTORY_H

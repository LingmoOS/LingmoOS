// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cdrawtoolfactory.h"
#include "cselecttool.h"
#include "crecttool.h"
#include "cellipsetool.h"
#include "clinetool.h"
#include "ctriangletool.h"
#include "ctexttool.h"
#include "cpolygontool.h"
#include "cpolygonalstartool.h"
#include "cpentool.h"
#include "ccuttool.h"
#include "cmasicotool.h"
#include "cpicturetool.h"
#include "cviewmanagement.h"
#include "cgraphicsview.h"
#include "cerasertool.h"

#include <QObject>

#include <stdio.h>
#include <dlfcn.h>

CDrawToolFactory::CDrawToolsMap CDrawToolFactory::s_tools = CDrawToolFactory::CDrawToolsMap();
IDrawTool *CDrawToolFactory::Create(EDrawToolMode mode)
{
    IDrawTool *pTool = nullptr;

    switch (mode) {
    case selection:
        pTool = new CSelectTool();
        break;
    case line:
        pTool = new CLineTool();
        break;
    case rectangle:
        pTool = new CRectTool();
        break;
    case ellipse:
        pTool = new CEllipseTool();
        break;
    case triangle:
        pTool = new CTriangleTool();
        break;
    case text:
        pTool = new CTextTool();
        break;
    case polygon:
        pTool = new CPolygonTool();
        break;
    case polygonalStar:
        pTool = new CPolygonalStarTool();
        break;
    case pen:
        pTool = new CPenTool();
        break;
    case eraser:
        pTool = new CEraserTool();
        break;
    case cut:
        pTool = new CCutTool();
        break;
    case blur:
        pTool = new /*CMasicoTool*/IBlurTool();
        break;
    case picture:
        pTool = new CPictureTool();
        break;
    default:
        break;
    }

    return pTool;
}

//bool blocked = false;
void CDrawToolFactory::installTool(IDrawTool *tool)
{
    if (tool == nullptr) {
        qWarning() << "tool is null !!!!!!!!";
        return;
    }


    s_tools.insert(tool->getDrawToolMode(), tool);

//    QObject::connect(tool->toolButton(), &QAbstractButton::toggled, tool->toolButton(), [ = ](bool checked) {
//        if (checked) {
//            setCurrentTool(tool);
//        }
//    });
}

IDrawTool *CDrawToolFactory::tool(int toolId)
{
    auto itfind = s_tools.find(toolId);
    if (itfind != s_tools.end()) {
        return itfind.value();
    }
    return nullptr;
}

CDrawToolFactory::CDrawToolsMap &CDrawToolFactory::allTools()
{
    return s_tools;
}

IDrawTool *CDrawToolFactory::loadToolPlugin(const QString &pluginPath)
{
    //plugins
    //手动加载指定位置的so动态库
    std::string string = pluginPath.toStdString();

    void *handle = dlopen(string.c_str(), RTLD_NOW);

    qWarning() << "load plugin handle = " << handle;
    if (handle == nullptr) {
        std::string errmsg = std::string(dlerror());
        qWarning() << "load plugin error = " << QString::fromStdString(errmsg);
        return nullptr;
    }

    typedef IDrawTool*(*Fun)() ;

    /*根据动态链接库操作句柄与符号，返回符号对应的地址*/
    Fun addres  = (Fun)dlsym(handle, "creatTool");

    if (addres == nullptr)
        return nullptr;

    IDrawTool *result = addres();

    //dlclose(handle);

    return result;
}

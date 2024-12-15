// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawtoolfactory.h"
#include "selecttool.h"
#include "recttool.h"
#include "ellipsetool.h"
#include "linetool.h"
#include "triangletool.h"
#include "texttool.h"
#include "polygontool.h"
#include "startool.h"
#include "pentool.h"
#include "blurtool.h"
#include "imageloadtool.h"
#include "erasertool.h"
#include "buttontool.h"
#include "config.h"

#include <QDirIterator>
#include <QTranslator>
#include <QCoreApplication>

#include <QObject>
#include <stdio.h>
#include <QLibrary>
#ifdef Q_OS_LINUX
#include <dlfcn.h>
#endif

QMap<int, generatedInterface>  DrawToolFactory::s_pluginToolCreatFuns = QMap<int, generatedInterface>();
DrawTool *DrawToolFactory::createTool(int mode)
{
    DrawTool *pTool = nullptr;

    switch (mode) {
    case selection:
        pTool = new SelectTool();
        break;
    case line:
        pTool = new LineTool();
        break;
    case rectangle:
        pTool = new RectTool();
        break;
    case ellipse:
        pTool = new EllipseTool();
        break;
    case triangle:
        pTool = new TriangleTool();
        break;
    case text:
        pTool = new TextTool();
        break;
    case polygon:
        pTool = new PolygonTool();
        break;
    case polygonalStar:
        pTool = new StarTool();
        break;
    case EDrawComItemTool:
        pTool = new DrawComItemTool;
        break;
    case pen:
        pTool = new PenTool();
        break;
    case eraser:
        pTool = new EraserTool();
        break;
    //    case cut:
    //        pTool = new CCutTool();
    //        break;
    case EUndoTool:
        pTool = new UndoTool;
        break;
    case ERedoTool:
        pTool = new RedoTool;
        break;
    case EClearTool:
        pTool = new ClearTool;
        break;
    case ESaveTool:
        pTool = new SaveTool;
        break;
    case EOpenTool:
        pTool = new OpenTool;
        break;
    case blur:
        pTool = new BlurTool();
        break;
    case picture:
        pTool = new ImageLoadTool();
        break;
    case ECloseTool:
        pTool = new CloseTool();
        break;
    default:
        break;
    }

    if (pTool == nullptr) {
        auto findIt = s_pluginToolCreatFuns.find(mode);
        if (findIt != s_pluginToolCreatFuns.end()) {
            generatedInterface cF = findIt.value();
            pTool = cF();
        }
    }

    return pTool;
}

void DrawToolFactory::loadAllToolPlugins()
{
    //1.load translations first
    QDir dir(TOOLPLUGINTRANSPATH);
    if (dir.exists()) {
        QDirIterator qmIt(TOOLPLUGINTRANSPATH, QStringList() << QString("*%1.qm").arg(QLocale::system().name()), QDir::Files);
        while (qmIt.hasNext()) {
            qmIt.next();
            QFileInfo finfo = qmIt.fileInfo();
            QTranslator *translator = new QTranslator;
            if (translator->load(finfo.baseName(), finfo.absolutePath())) {
                QCoreApplication::installTranslator(translator);
            }
        }
    }

    //2.load plugins
    QDir pluginLibDir(TOOLPLUGINPATH);
    if (pluginLibDir.exists()) {
#ifdef Q_OS_LINUX
        QStringList filter = QStringList() << "*.so";
#else
#ifdef Q_OS_WIN
        QStringList filter = QStringList() << "*.dll";
#endif
#endif
        QDirIterator soIt(TOOLPLUGINPATH, filter, QDir::Files);
        while (soIt.hasNext()) {
            soIt.next();
            QFileInfo finfo = soIt.fileInfo();
            DrawToolFactory::loadToolPlugin(finfo.filePath());
        }
    }
}

bool DrawToolFactory::loadToolPlugin(const QString &pluginPath)
{
#ifdef Q_OS_LINUX
    //plugins
    //手动加载指定位置的so动态库
    std::string string = pluginPath.toStdString();

    void *handle = dlopen(string.c_str(), RTLD_NOW);

    qWarning() << "load plugin handle = " << handle;
    if (handle == nullptr) {
        std::string errmsg = std::string(dlerror());
        qWarning() << "load plugin error = " << QString::fromStdString(errmsg);
        return false;
    }

    /*根据动态链接库操作句柄与符号，返回符号对应的地址*/
    generatedInterface addres  = (generatedInterface)dlsym(handle, "creatTool");

    if (addres == nullptr)
        return false;

    bool checkAddresSuccess = false;
    DrawTool *result = addres();

    if (result != nullptr) {
        checkAddresSuccess = true;
        s_pluginToolCreatFuns.insert(result->toolType(), addres);
        delete result;
    }
    //dlclose(handle);
    return checkAddresSuccess;
#else
#ifdef Q_OS_WIN
    QLibrary library(pluginPath);
    if (library.load()) {
        QFunctionPointer func = library.resolve("creatTool");

        if (func != nullptr) {
            generatedInterface addres  = generatedInterface(func);
            bool checkAddresSuccess = false;
            DrawTool *result = addres();

            if (result != nullptr) {
                checkAddresSuccess = true;
                s_pluginToolCreatFuns.insert(result->toolType(), addres);
                delete result;
            }
            return checkAddresSuccess;
        }
        return false;
    }
    return false;

#endif
    return false;
#endif

}

void DrawToolFactory::registTool(int toolType, generatedInterface f)
{
    if (f != nullptr)
        s_pluginToolCreatFuns.insert(toolType, f);
}

ToolCreatFunMap DrawToolFactory::genInterfacesForPluginTool()
{
    return s_pluginToolCreatFuns;
}

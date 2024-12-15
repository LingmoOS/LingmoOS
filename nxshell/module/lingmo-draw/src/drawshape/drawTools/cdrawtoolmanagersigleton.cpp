// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cdrawtoolmanagersigleton.h"
#include "idrawtool.h"
#include <QDebug>
#include "cdrawtoolfactory.h"

CDrawToolManagerSigleton *CDrawToolManagerSigleton::m_pInstance = nullptr;

CDrawToolManagerSigleton::CDrawToolManagerSigleton()
{
    //m_hashDrawTool.clear();
}

//void CDrawToolManagerSigleton::toolManagerDeconstruction()
//{
////    foreach (IDrawTool *tool, m_hashDrawTool.values()) {
////        delete tool;
////        tool = nullptr;
////    }
////    m_hashDrawTool.clear();
////    if (m_pInstance) {
////        delete m_pInstance;
////        m_pInstance = nullptr;
////    }
//}

CDrawToolManagerSigleton *CDrawToolManagerSigleton::GetInstance()
{
    if (m_pInstance == nullptr) {
        m_pInstance  = new CDrawToolManagerSigleton();
    }
    return m_pInstance;
}

//void CDrawToolManagerSigleton::insertDrawTool(EDrawToolMode mode, IDrawTool *tool)
//{
//    //m_hashDrawTool.insert(mode, tool);
//    CDrawToolFactory::installTool(tool);
//}

IDrawTool *CDrawToolManagerSigleton::getDrawTool(EDrawToolMode mode) const
{
    //return m_hashDrawTool[mode];
    return CDrawToolFactory::tool(mode);
}


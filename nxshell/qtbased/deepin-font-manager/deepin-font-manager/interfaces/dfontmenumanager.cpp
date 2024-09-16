// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "interfaces/dfontmenumanager.h"

#include <DApplicationHelper>
#include <DApplication>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

DFontMenuManager *DFontMenuManager::instance = nullptr;

//构造函数
DFontMenuManager::DFontMenuManager(QObject *parent)
    : QObject(parent)
{
    initMenuData();
}

/*************************************************************************
 <Function>      getInstance
 <Description>   获取单例
 <Author>        null
 <Input>
    <param1>     null                         Description:null
 <Return>        DFontMenuManager *           Description:需要获取到的单例类
 <Note>
*************************************************************************/
DFontMenuManager *DFontMenuManager::getInstance()
{
    if (nullptr == instance) {
        instance = new DFontMenuManager();
    }

    return instance;
}

/*************************************************************************
 <Function>      DFontMenuManager
 <Description>   对菜单进行初始化
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
*************************************************************************/
void DFontMenuManager::initMenuData()
{
    // ToDo:
    //    Need to localize the menu string

    // Tools bar menu & Right key menu.
    m_fontToolBarMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Add font"), MenuAction::M_AddFont));
    m_fontToolBarMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));

    // Right key menu data
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Add font"), MenuAction::M_AddFont));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Enable"), MenuAction::M_EnableOrDisable));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Delete"), MenuAction::M_DeleteFont));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Export"), MenuAction::M_ExportFont));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Favorite"), MenuAction::M_Faverator));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Details"), MenuAction::M_FontInfo));
    m_fontRightMenuData.push_back(new FMenuItem("", MenuAction::M_Separator));
    m_fontRightMenuData.push_back(
        new FMenuItem(DApplication::translate("Menu", "Display in file manager"), MenuAction::M_ShowFontPostion));
}


/*************************************************************************
 <Function>      createToolBarSettingsMenu
 <Description>   新建工具栏菜单(已弃用)
 <Author>        null
 <Input>
    <param1>     FMenuActionTriggle  actionTriggle    Description:无用参数
 <Return>        null            Description:null
*************************************************************************/
QMenu *DFontMenuManager::createToolBarSettingsMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    DMenu *mainMenu = new DMenu();

    for (auto it : m_fontToolBarMenuData) {
        QAction *newAction = nullptr;

        if (it->actionId == MenuAction::M_Separator) {
            mainMenu->addSeparator();
            continue;
        }

        if (!it->fHaveSubMenu) {
            newAction = mainMenu->addAction(it->actionName);
            newAction->setData(it->actionId);
            it->action = newAction;

            m_fontToolsBarMenus.insert(it->actionId, it);
        } else {
            DMenu *subMenu = mainMenu->addMenu(it->actionName);

            // Create a action group for group menu
            if (it->fGroupSubMenu) {
                it->actionGroup = new QActionGroup(subMenu);
            }

            for (auto iter : it->subMenulist) {
                newAction = subMenu->addAction(iter->actionName);
                newAction->setData(iter->actionId);
                iter->action = newAction;

                if (it->fGroupSubMenu) {
                    newAction->setCheckable(true);
                    it->actionGroup->addAction(newAction);
                }

                m_fontToolsBarMenus.insert(iter->actionId, iter);
            }
        }
    }
    return mainMenu;
}

/*************************************************************************
 <Function>      createRightKeyMenu
 <Description>   新建右键菜单
 <Author>        null
 <Input>
    <param1>     FMenuActionTriggle  actionTriggle    Description:无用参数
 <Return>        null            Description:null
*************************************************************************/
QMenu *DFontMenuManager::createRightKeyMenu(FMenuActionTriggle actionTriggle)
{
    Q_UNUSED(actionTriggle);

    DMenu *rightKeyMenu = new DMenu();

    for (auto it : m_fontRightMenuData) {
        QAction *newAction = nullptr;

        if (it->actionId == MenuAction::M_Separator) {
            rightKeyMenu->addSeparator();
            continue;
        }

        if (!it->fHaveSubMenu) {
            newAction = rightKeyMenu->addAction(it->actionName);
            newAction->setData(it->actionId);
            it->action = newAction;

            m_fontRightKeyMenus.insert(it->actionId, it);
        } else {
            DMenu *subMenu = rightKeyMenu->addMenu(it->actionName);

            for (auto iter : it->subMenulist) {
                newAction = subMenu->addAction(iter->actionName);
                newAction->setData(iter->actionId);
                iter->action = newAction;

                m_fontRightKeyMenus.insert(iter->actionId, iter);
            }
        }
    }

    return rightKeyMenu;
}

/*************************************************************************
 <Function>      getActionByMenuAction
 <Description>   通过传入参数获取到菜单的操作
 <Author>        null
 <Input>
    <param1>     MenuAction maction   Description:菜单操作类型
    <param2>     MenuType menuType    Description:菜单的种类
 <Return>        QAction *            menuType种类的菜单中maction类型的操作
 <Note>          null
*************************************************************************/
QAction *DFontMenuManager::getActionByMenuAction(MenuAction maction, MenuType menuType)
{
    QAction *action = nullptr;

    if (MenuType::ToolBarMenu == menuType) {
        auto it = m_fontToolsBarMenus.find(maction);
        if (it != m_fontToolsBarMenus.end()) {
            action = it.value()->action;
        }
    } else if (MenuType::RightKeyMenu == menuType) {
        auto it = m_fontRightKeyMenus.find(maction);
        if (it != m_fontRightKeyMenus.end()) {
            action = it.value()->action;
        }
    } else {
        qDebug() << __FUNCTION__ << " Unknow menu type = " << menuType << " for MenuAction=" << maction;
    }

    return action;
}

/*******************************************************************************************************
 <Function>      onRightKeyMenuPopup
 <Description>   右键菜单弹出时进行判断,使菜单的显示内容根据不同情况进行改变
 <Author>        null
 <Input>
    <param1>     const DFontPreviewItemData &fontData  Description:唤起菜单时作为判断的预览字体列表项的数据
    <param2>     bool hasUser                          Description:是否可以删除(只有用户字体可以删除)
    <param3>     bool enableDisable                    Description:是否可以禁用
    <param4>     bool hasCurFont                       Description:是否包含当前使用字体
 <Return>        null
 <Note>          null
*******************************************************************************************************/
void DFontMenuManager::onRightKeyMenuPopup(const DFontPreviewItemData &itemData, bool hasUser, bool enableDisable, bool hasCurFont)
{
    // Disable delete menu for system font
    QAction *delAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                             DFontMenuManager::M_DeleteFont, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *exportAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                                DFontMenuManager::M_ExportFont, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *faveriteAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                                  DFontMenuManager::M_Faverator, DFontMenuManager::MenuType::RightKeyMenu);

    QAction *enableOrDisableAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                                         DFontMenuManager::M_EnableOrDisable, DFontMenuManager::MenuType::RightKeyMenu);

    // Disable delete menu on system font
    if (nullptr != delAction) {
        if (hasUser) {
            delAction->setDisabled(false);
        } else {
            delAction->setDisabled(true);
        }
    }

    // Export menu on system font
    if (nullptr != exportAction) {
        if (hasUser || hasCurFont) {
            exportAction->setDisabled(false);
        } else {
            exportAction->setDisabled(true);
        }
    }


    // Favarite font Menu
    if (nullptr != faveriteAction) {
        if (itemData.fontData.isCollected()) {
            faveriteAction->setText(DApplication::translate("Menu", "Unfavorite"));
        } else {
            faveriteAction->setText(DApplication::translate("Menu", "Favorite"));
        }
    }


    // Enable/Disable Menu
    if (nullptr != enableOrDisableAction) {
        if (itemData.fontData.isEnabled()) {
            if (enableDisable) {
                enableOrDisableAction->setEnabled(true);
            } else {
                enableOrDisableAction->setEnabled(false);
            }
            enableOrDisableAction->setText(DApplication::translate("Menu", "Disable"));
        } else {
            enableOrDisableAction->setEnabled(true);
            enableOrDisableAction->setText(DApplication::translate("Menu", "Enable"));
        }
    }

}

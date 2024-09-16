// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTMENUMANAGER_H
#define DFONTMENUMANAGER_H
#include "dfontpreviewitemdef.h"

#include <DMenu>

DWIDGET_USE_NAMESPACE

struct DFontPreviewItemData;

/*************************************************************************
 <Class>         DFontMenuManager
 <Description>   菜单管理的单例类,提供了获取菜单,设置菜单的函数.
 <Author>
 <Note>          null
*************************************************************************/
class DFontMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit DFontMenuManager(QObject *parent = nullptr);
    //获取单例
    static DFontMenuManager *getInstance();

    /*************************************************************************
     <Enum>      MenuType
     <Description>   描述菜单类型的枚举
     <Author>        null
     <Value>
        <Value1>     ToolBarMenu     Description:工具栏菜单(已废弃不用)
        <Value2>     RightKeyMenu    Description:右键菜单
     <Note>          null
    *************************************************************************/
    enum MenuType {
        ToolBarMenu,
        RightKeyMenu,
    };

    /*************************************************************************
     <Enum>      MenuAction
     <Description>  描述菜单选项的枚举
     <Author>        null
     <Value>
        <Value1> M_Separator             Description:菜单分割线
        <Value2> M_AddFont               Description:添加字体
        <Value3> M_EnableOrDisable       Description:禁用/启用字体
        <Value4> M_DeleteFont            Description:删除字体
        <Value5> M_ExportFont            Description:导出字体
        <Value6> M_Faverator             Description:我的收藏
        <Value7> M_FontInfo              Description:字体信息
        <Value8> M_ShowFontPostion       Description:在文件夹显示
     <Note>          null
    *************************************************************************/
    enum MenuAction {
        M_Separator,
        M_AddFont,
        M_EnableOrDisable,
        M_DeleteFont,
        M_ExportFont,
        M_Faverator,
        M_FontInfo,
        M_ShowFontPostion,
    };

    /*************************************************************************
       <Struct> FMenuItem
       <Description> 用于储存菜单中每一项的信息的结构体
       <Author>        null
       <Member>
          <Member1>   QString actionName                     Description:菜单项的名字
          <Member2>   MenuAction actionId                    Description:菜单项的类型
          <Member3>   QAction  action                        Description:菜单项的操作
          <Member4>   bool fHaveSubMenu                      Description:菜单项是否含有二级菜单
          <Member5>   bool fGroupSubMenu                     Description:菜单项的操作是否分组
          <Member6>   QActionGroup *actionGroup              Description:菜单项的操作分组
          <Member7>   QVector<FMenuItem *> subMenulist       Description:次级菜单项Vector
       <Note>          null
    *************************************************************************/
    struct FMenuItem {
        FMenuItem(QString name, MenuAction id, bool haveSubMenu = false, bool group = false)
            : actionName(name)
            , actionId(id)
            , fHaveSubMenu(haveSubMenu)
            , fGroupSubMenu(group)
        {
        }

        QString actionName;
        MenuAction actionId;
        QAction *action {nullptr};
        bool fHaveSubMenu {false};
        bool fGroupSubMenu {false};
        QActionGroup *actionGroup {nullptr};
        QVector<FMenuItem *> subMenulist;
    };

    typedef void (*FMenuActionTriggle)(QAction *);

    //创建工具条设置菜单
    DMenu *createToolBarSettingsMenu(FMenuActionTriggle actionTriggle = nullptr);

    //创建鼠标右键菜单
    DMenu *createRightKeyMenu(FMenuActionTriggle actionTriggle = nullptr);

    //根据提供的类型获取菜单操作action
    QAction *getActionByMenuAction(MenuAction maction, MenuType menuType);

private:
    //菜单数据初始化
    void initMenuData();

    QMap<MenuAction, FMenuItem *> m_fontToolsBarMenus;
    QMap<MenuAction, FMenuItem *> m_fontRightKeyMenus;

    QVector<FMenuItem *> m_fontToolBarMenuData;
    QVector<FMenuItem *> m_fontRightMenuData;

private:
    static DFontMenuManager *instance;
signals:

public slots:
    //右键菜单弹出时进行判断,使菜单的显示内容根据不同情况进行改变
    void onRightKeyMenuPopup(const DFontPreviewItemData &fontData, bool hasUser, bool enableDisable, bool hasCurFont);
};

#endif  // DFONTMENUMANAGER_H

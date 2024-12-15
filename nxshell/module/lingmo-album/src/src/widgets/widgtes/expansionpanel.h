// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPANSIONPANEL_H
#define EXPANSIONPANEL_H

#include "toolbutton.h"

#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <DBlurEffectWidget>
#include <DToolButton>
#include <DWidget>
#include <DGuiApplicationHelper>
#include <QMap>

#include "unionimage/unionimage_global.h"

class ExpansionPanel : public DBlurEffectWidget
{
    Q_OBJECT
public:
    //所选数据
    struct FilteData {
        QIcon icon_l_light = QIcon();//左边图标浅
        QIcon icon_l_dark = QIcon(); //左边图标深
        QIcon icon_r_light = QIcon();//右边图标浅
        QIcon icon_r_dark = QIcon(); //右边图标深
        QString icon_r_path;
        QString text;
        ItemType type = ItemType::ItemTypeNull;
        QString className = "";
    };

public:
    /**
      * @brief ExpansionPanel 构造函数
      * @param parent
      */
    explicit ExpansionPanel(QWidget *parent = nullptr);

    /**
      * @brief getUngroupButton 主题颜色改变
      * @param parent
      */
    void changeTheme(Dtk::Gui::DGuiApplicationHelper::ColorType themeType);

    //动态添加按钮
    void addNewButton(FilteData &data);

signals:
    /**
     * @description: signalItemGroup 组合框选图元信号
    */
    void signalItemGroup();


    /**
     * @description: signalItemGroup 释放组合框选图元信号
    */
    void signalItemgUngroup();

    void currentItemChanged(FilteData &data);

protected:
    void focusOutEvent(QFocusEvent *e) override;

private slots:
    void onButtonClicked(FilteData data);

private:
    //阴影背景
    QGraphicsDropShadowEffect *shadow_effect = nullptr;
    //内置按钮组
    QMap<int, ToolButton *> buttons;
    //按钮数量
    int buttonCount = 0;
    //主图层
    QVBoxLayout *layout;
};
Q_DECLARE_METATYPE(ExpansionPanel::FilteData)
#endif // EXPANSIONPANEL_H

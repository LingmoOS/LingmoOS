// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMSETTINGWINDOW_H
#define IMSETTINGWINDOW_H

#include "fcitxInterface/global.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxSettingsGroup;
class FcitxSettingsHead;
class FcitxComBoboxSettingsItem;
class FcitxPushButtonSettingsItem;
class FcitxKeySettingsItem;
class FcitxIMActivityItem;
} // namespace widgets
} // namespace dcc_fcitx_configtool

namespace Dtk {
namespace Widget {
class DListView;
class DCommandLinkButton;
class DFloatingButton;

} // namespace Widget
} // namespace Dtk

class IMSettingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit IMSettingWindow(QWidget *parent = nullptr);
    virtual ~IMSettingWindow();
    void updateUI(); //刷新界面
signals:
    void popIMAddWindow(); //弹出添加输入法界面
    void popShortKeyListWindow(const QString &curName, const QStringList &list, QString &name); //弹出快捷键冲突界面
    void popShortKeyStrWindow(const QString &curName, const QString &str, QString &name); //弹出快捷键冲突界面
    void availWidgetAdd(const FcitxQtInputMethodItem &item);
    void requestNextPage(QWidget * const w) const;
private:
    void initUI(); //初始化界面
    void initConnect(); //初始化信号槽
    void readConfig(); //读取配置文件
    void itemSwap(const FcitxQtInputMethodItem &item, const bool &isUp = true);
private slots:
    void onEditBtnClicked(const bool &flag); //启用编辑
    void onCurIMChanged(const FcitxQtInputMethodItemList &list);
    void onAddBtnCilcked();
    void onItemUp(const FcitxQtInputMethodItem &item);
    void onItemDown(const FcitxQtInputMethodItem &item);
    void onItemDelete(const FcitxQtInputMethodItem &item);
    void doReloadConfigUI();
    void onReloadConnect();
private:
    dcc_fcitx_configtool::widgets::FcitxSettingsGroup *m_IMListGroup {nullptr}; //输入法列表容器
    dcc_fcitx_configtool::widgets::FcitxSettingsGroup *m_shortcutGroup {nullptr}; //输入法快捷键容器
    dcc_fcitx_configtool::widgets::FcitxSettingsHead *m_editHead {nullptr}; //编辑按钮
    dcc_fcitx_configtool::widgets::FcitxComBoboxSettingsItem *m_imSwitchCbox {nullptr}; //切换输入法（快捷键）
    dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *m_defaultIMKey {nullptr}; //默认输入法（快捷键）
    QPushButton* m_advSetKey  {nullptr}; //高级设置
    Dtk::Widget::DCommandLinkButton* m_resetBtn {nullptr}; //重置输入法（快捷键）
    Dtk::Widget::DFloatingButton *m_addIMBtn {nullptr}; //添加输入法
    QVBoxLayout *m_mainLayout;
};

#endif // IMSETTINGWINDOW_H

// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_MODULE_IMSETTINGWINDOW_H
#define DCC_MODULE_IMSETTINGWINDOW_H

#include <widgets/comboxwidget.h>
#include <widgets/settingsgroup.h>
#include <widgets/settingshead.h>

#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxSettingsGroup;
class FcitxPushButtonSettingsItem;
class FcitxKeySettingsItem;
} // namespace widgets
} // namespace dcc_fcitx_configtool

namespace Dtk {
namespace Widget {
class DListView;
class DCommandLinkButton;
class DFloatingButton;
class DIconButton;
} // namespace Widget
} // namespace Dtk

namespace fcitx::kcm {
class DBusProvider;
class ConfigWidget;
class IMConfig;
class FilteredIMModel;
} // namespace fcitx::kcm

class ConfigSettings;
class QStandardItemModel;
class ConfigProxy;

class IMSettingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit IMSettingWindow(fcitx::kcm::DBusProvider *dbus, QWidget *parent = nullptr);
    virtual ~IMSettingWindow();
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void requestNextPage(QWidget *const w) const;

private:
    void initUI();      // 初始化界面
    void initConnect(); // 初始化信号槽

    void updateActions();

private Q_SLOTS:
    void onCurIMChanged(fcitx::kcm::FilteredIMModel *model);
    void onItemUp(int row);
    void onItemDown(int row);
    void onItemDelete(int row);
    void onItemConfig(int row);

private:
    Dtk::Widget::DListView *m_IMListGroup; // 输入法列表容器
    QStandardItemModel *m_IMListModel;
    DCC_NAMESPACE::SettingsGroup *m_shortcutGroup; // 输入法快捷键容器
    DCC_NAMESPACE::SettingsHead *m_editHead;       // 编辑按钮
    Dtk::Widget::DIconButton *m_deleteBtn;
    DCC_NAMESPACE::ComboxWidget *m_enumerateForwardKeysComboBox; // 切换输入法（快捷键）
    dcc_fcitx_configtool::widgets::FcitxKeySettingsItem *m_defaultIMKey{
        nullptr
    };                                                      // 默认输入法（快捷键）
    QPushButton *m_advSetKey{ nullptr };                    // 高级设置
    Dtk::Widget::DCommandLinkButton *m_resetBtn{ nullptr }; // 重置输入法（快捷键）
    Dtk::Widget::DFloatingButton *m_addIMBtn{ nullptr };    // 添加输入法
    QVBoxLayout *m_mainLayout;
    fcitx::kcm::DBusProvider *m_dbus;
    fcitx::kcm::IMConfig *m_config;
    fcitx::kcm::ConfigWidget *m_configProxy;
    int m_hoveredRow = -1;
};

#endif // !DCC_MODULE_IMSETTINGWINDOW_H

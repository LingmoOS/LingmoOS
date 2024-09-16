// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FCITXCONFIGPAGE_H
#define FCITXCONFIGPAGE_H

// Qt
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMap>
#include <QLabel>

// Fcitx
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/hotkey.h>
#include <fcitx/addon.h>

//self
#include "fcitxInterface/dummyconfig.h"

class QCheckBox;
class QVBoxLayout;
class QStandardItemModel;
struct _FcitxConfigFileDesc;

class QTabWidget;
namespace dcc_fcitx_configtool {
namespace widgets {
class FcitxCheckBoxSettingsItem;
}
}
namespace Fcitx {
class Global;
class AddonSelector;

/**
 * @class AdvancedSettingWidget
 * @brief 高级设置界面
 */
class AdvancedSettingWidget : public QWidget
{
    Q_OBJECT

    enum UIType {
        CW_Simple = 0x1,
        CW_Full = 0x2,
        CW_NoShow = 0x0
    };

public:
    explicit AdvancedSettingWidget(QWidget *parent = nullptr);
    virtual ~AdvancedSettingWidget();
private:
    /**
     *@brief 创建全局设置界面
     *@return 全局设置界面指针
     */
    QWidget *createglobalSettingsUi();

    /**
     *@brief 创建附加组件界面
     *@return 附加组件界面指针
     */
    QWidget *createAddOnsUi();

    /**
     *@brief 创建界面元素
     */
    void setupConfigUi();

    /**
     *@brief 创建界面元素
     */
    void createConfigOptionWidget(FcitxConfigGroupDesc *cgdesc, FcitxConfigOptionDesc *codesc, QString &label, QString &tooltip, QWidget *&inputWidget, void *&newarg);

    /**
     *@brief 读取配置文件
     *@param name: 文件名称
     */
    void getConfigDesc(const QString &name);

    /**
     *@brief 界面设置项改变，发送重新加载消息
     */
    void sendReloadMessage();

    /**
     *@brief 加载配置文件，并刷新界面
     */
    void loadFile();

private slots:
    /**
     *@brief 界面风格改变
     */
    void onPalettetypechanged();

private:
    QHash<QString, FcitxConfigFileDesc *> *m_hash;
    struct _FcitxConfigFileDesc *m_cfdesc;
    QString m_prefix;
    QString m_name;
    QString m_addonName;
    QVBoxLayout *m_globalSettingsLayout;
    QVBoxLayout *m_addOnsLayout;
    QWidget *m_widget {nullptr};
    QWidget *m_fullWidget {nullptr};
    DummyConfig m_config;
    UIType m_simpleUiType;
    UIType m_fullUiType;
    QMap<QString, void *> m_argsMap;
    UT_array *m_addons;
    AddonSelector *m_addonSelector;
    bool m_isSelfSend {false};
    QList<dcc_fcitx_configtool::widgets::FcitxCheckBoxSettingsItem *> m_addonsList;
};

/**
 * @class arrowButton
 * @brief 高级设置界面的箭头按键
 */
class ArrowButton : public QLabel
{
    Q_OBJECT

public:
    explicit ArrowButton(QWidget *parent = nullptr);

    /**
     *@brief 设置界面是否隐藏
     */
    void setContentHidden(bool hide);

    /**
     *@brief 设置文字，现实不全自动加tooltip
     */
    void setOriginText(const QString &text);

protected:
    void mousePressEvent(QMouseEvent *ev) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    /**
     *@brief 按下信号
     */
    void pressed(bool isHidden);

private:
    bool m_hide{false};
    QString m_originText;
};

}

#endif

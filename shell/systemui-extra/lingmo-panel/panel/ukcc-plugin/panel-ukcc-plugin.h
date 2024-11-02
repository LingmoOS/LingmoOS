/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_PANEL_PANEL_UKCC_PLUGIN_H
#define LINGMO_PANEL_PANEL_UKCC_PLUGIN_H

#include <QObject>
#include <QVBoxLayout>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <ukcc/interface/interface.h>
#include <ukcc/interface/interface.h>
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/comboxframe.h>
#include <ukcc/widgets/fixlabel.h>
#include <kswitchbutton.h>
#include "rep_remote-config_replica.h"

class PanelUkccPlugin : public QObject, public CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit PanelUkccPlugin(QObject *parent = nullptr);

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    void initUI();
    QFrame * setLine(QFrame *parent = nullptr);
    void connectToSource();
    void initData();
    void sourceStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState);
    QRemoteObjectNode m_qroNode;
    RemoteConfigReplica *m_configReplica = nullptr;

    //main page
    QWidget *m_widget = nullptr;

    //"panel"
    QVBoxLayout *m_mainLayout = nullptr;
    TitleLabel *m_titleLabel1 = nullptr;
    QFrame *m_frame1 = nullptr;
    QVBoxLayout *m_vLayout1 = nullptr;
    //merge icons
    QFrame *m_mergeIconsFrame = nullptr;
    QHBoxLayout *m_mergeIconsLayout = nullptr;
    QLabel *m_mergeIconsLabel = nullptr;
    QComboBox *m_mergeIcons = nullptr;
    //panel location
    QFrame *m_panelLocationFrame = nullptr;
    QHBoxLayout *m_panelLocationLayout = nullptr;
    QLabel *m_panelLocationLabel = nullptr;
    QComboBox *m_panelLocation= nullptr;
    //panel size
    QFrame *m_panelSizePolicyFrame = nullptr;
    QHBoxLayout *m_panelSizePolicyLayout = nullptr;
    QLabel *m_panelSizePolicyLabel = nullptr;
    QComboBox *m_panelSizePolicy = nullptr;
    //panel auto hide
    QFrame *m_panelAutoHideFrame = nullptr;
    QHBoxLayout *m_panelAutoHideLayout = nullptr;
    QLabel *m_panelAutoHideLabel = nullptr;
    kdk::KSwitchButton *m_panelAutoHide = nullptr;
    //panel lock
    QFrame *m_panelLockFrame = nullptr;
    QHBoxLayout *m_panelLockLayout = nullptr;
    QLabel *m_panelLockLabel = nullptr;
    kdk::KSwitchButton *m_panelLock = nullptr;

    //show panel on all screens
    QFrame *m_showPanelOnAllScreensFrame = nullptr;
    QHBoxLayout *m_showPanelOnAllScreensLayout = nullptr;
    QLabel *m_showPanelOnAllScreensLabel = nullptr;
    kdk::KSwitchButton *m_showPanelOnAllScreens = nullptr;

    //show system tray on all panel
    QFrame *m_showSystemTrayOnAllPanelsFrame = nullptr;
    QHBoxLayout *m_showSystemTrayOnAllPanelsLayout = nullptr;
    QLabel *m_showSystemTrayOnAllPanelsLabel = nullptr;
    kdk::KSwitchButton *m_showSystemTrayOnAllPanels = nullptr;

    //icons showed on witch panel
    QFrame *m_iconsShowedOnFrame = nullptr;
    QHBoxLayout *m_iconsShowedOnLayout = nullptr;
    QLabel *m_iconsShowedOnLabel = nullptr;
    QComboBox *m_iconsShowedOn = nullptr;

    //"widgets showed on panel"
    TitleLabel *m_titleLabel2{};
    QFrame *m_frame2{};
    QVBoxLayout *m_vLayout2{};

    //task view
    QFrame *m_showTaskViewFrame = nullptr;
    QHBoxLayout *m_showTaskViewLayout = nullptr;
    QLabel *m_showTaskViewLabel = nullptr;
    kdk::KSwitchButton *m_showTaskView = nullptr;

    //show search
    QFrame *m_showSearchFrame = nullptr;
    QHBoxLayout *m_showSearchLayout = nullptr;
    QLabel *m_showSearchLabel = nullptr;
    kdk::KSwitchButton *m_showSearch = nullptr;

    //"widgets showed on panel"
    TitleLabel *m_titleLabel3{};
    QFrame *m_frame3{};
    QVBoxLayout *m_vLayout3{};

    QStringList m_systemTrayIconId = {
            "lingmo-power-manager-tray",
            "lingmo-volume-control-applet-qt",
            "lingmo-nm",
            "lingmo-bluetooth"
    };
    QMap<QString, QString> m_systemTrayIconIdName;
};


#endif //LINGMO_PANEL_PANEL_UKCC_PLUGIN_H

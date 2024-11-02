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

#ifndef LINGMO_PANEL_REMOTE_CONFIG_H
#define LINGMO_PANEL_REMOTE_CONFIG_H

#include "rep_remote-config_source.h"

namespace LingmoUIQuick {
    class Config;
}

class RemoteConfig : public RemoteConfigSource
{
    Q_OBJECT
public:
    explicit RemoteConfig(QObject *parent = nullptr);
    ~RemoteConfig() override = default;
    GeneralConfigDefine::MergeIcons mergeIcons() const override { return m_mergeIcons; }
    GeneralConfigDefine::PanelLocation panelLocation() const override { return m_panelLocation; }
    GeneralConfigDefine::PanelSizePolicy panelSizePolicy() const override { return m_panelSizePolicy; }
    bool panelAutoHide() const override { return m_panelAutoHide; }
    bool panelLock() const override { return m_panelLock; }
    QStringList panelWidgets() const override { return m_panelWidgets; }
    QStringList disabledWidgets() const override;
    QStringList trayIconsInhibited() const override {return m_trayIconsInhibited; }
    GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn() const override { return m_taskBarIconsShowedOn; };
    bool showPanelOnAllScreens() const override { return m_showPanelOnAllScreens; }
    bool showSystemTrayOnAllPanels() const override { return m_showSystemTrayOnAllPanels; }

    void setMergeIcons(GeneralConfigDefine::MergeIcons mergeIcons) override;
    void setPanelLocation(GeneralConfigDefine::PanelLocation panelLocation) override;
    void setPanelSizePolicy(GeneralConfigDefine::PanelSizePolicy panelSizePolicy) override;
    void setPanelAutoHide(bool panelAutoHide) override;
    void setPanelLock(bool panelLock) override;
    void setPanelWidgets(QStringList panelWidgets) override;
    void setTrayIconsInhibited(QStringList trayIconsInhibited) override;
    void setTaskBarIconsShowedOn(GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn) override;
    void setShowPanelOnAllScreens(bool showPanelOnAllScreens) override;
    void setShowSystemTrayOnAllPanels(bool showSystemTrayOnAllPanels) override;

public Q_SLOTS:
    void disableWidget(const QString &id, bool disable) override;

private:
    void onPanelConfigChanged(const QString &key);
    void onPanelViewConfigChanged(const QString &key);
    void onTaskManagerConfigChanged(const QString &key);
    void onSystemTrayConfigChanged(const QString &key);
    void updatePanelLocation();

    GeneralConfigDefine::MergeIcons m_mergeIcons;
    GeneralConfigDefine::PanelLocation m_panelLocation;
    GeneralConfigDefine::PanelSizePolicy m_panelSizePolicy;
    bool m_panelAutoHide;
    bool m_panelLock;
    QStringList m_panelWidgets;
    QStringList m_disabledWidgets;
    QStringList m_trayIconsInhibited;

    GeneralConfigDefine::TaskBarIconsShowedOn m_taskBarIconsShowedOn;
    bool m_showPanelOnAllScreens;
    bool m_showSystemTrayOnAllPanels;

    LingmoUIQuick::Config *m_panelConfig = nullptr;
    LingmoUIQuick::Config *m_mainViewConfig = nullptr;
    LingmoUIQuick::Config *m_taskManagerConfig = nullptr;
    LingmoUIQuick::Config *m_systemTrayConfig = nullptr;
};


#endif //LINGMO_PANEL_REMOTE_CONFIG_H

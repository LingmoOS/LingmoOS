// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDE_DISPLAY_CONFIG_H
#define DDE_DISPLAY_CONFIG_H

#include "../common/control.h"

#include <kscreen/config.h>

namespace dde {
namespace display {

class ConfigHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConfigHandler(QObject *parent = nullptr);
    ~ConfigHandler() override = default;

    void setConfig(KScreen::ConfigPtr config);
    void updateInitialData();

    QSize normalizeScreen();

    KScreen::ConfigPtr config() const
    {
        return m_config;
    }

    KScreen::ConfigPtr initialConfig() const
    {
        return m_initialConfig;
    }

    void revertConfig()
    {
        m_config = m_previousConfig->clone();
    }

    int retention() const;
    void setRetention(int retention);

    qreal scale(const KScreen::OutputPtr &output) const;
    void setScale(KScreen::OutputPtr &output, qreal scale);

    KScreen::OutputPtr replicationSource(const KScreen::OutputPtr &output) const;
    void setReplicationSource(KScreen::OutputPtr &output, const KScreen::OutputPtr &source);

    bool autoRotate(const KScreen::OutputPtr &output) const;
    void setAutoRotate(KScreen::OutputPtr &output, bool autoRotate);
    bool autoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const;
    void setAutoRotateOnlyInTabletMode(KScreen::OutputPtr &output, bool value);

    uint32_t overscan(const KScreen::OutputPtr &output) const;
    void setOverscan(const KScreen::OutputPtr &output, uint32_t value);

    KScreen::Output::VrrPolicy vrrPolicy(const KScreen::OutputPtr &output) const;
    void setVrrPolicy(const KScreen::OutputPtr &output, KScreen::Output::VrrPolicy value);

    KScreen::Output::RgbRange rgbRange(const KScreen::OutputPtr &output) const;
    void setRgbRange(const KScreen::OutputPtr &output, KScreen::Output::RgbRange value);

    void writeControl();

    void checkNeedsSave();
    bool shouldTestNewSettings();

Q_SIGNALS:
    void outputModelChanged();
    void changed();
    void screenNormalizationUpdate(bool normalized);
    void needsSaveChecked(bool need);
    void retentionChanged();
    void outputConnect(bool connected);
    void addMonitor(const KScreen::OutputPtr &output);
    void removeMonitor(const KScreen::OutputPtr &output);
    void monitorChanged(const KScreen::OutputPtr &output);

private:
    void checkScreenNormalization();
    QSize screenSize() const;
    Control::OutputRetention getRetention() const;
    void primaryOutputSelected(int index);
    void primaryOutputChanged(const KScreen::OutputPtr &output);
    void initOutput(const KScreen::OutputPtr &output);
    void resetScale(const KScreen::OutputPtr &output);
    /**
     * @brief checkSaveandTestCommon - compairs common config changes that would make the config dirty and needed to have the config checked when applied.
     * @param isSaveCheck - True  if your checking to see if the changes should request a save.
     *                      False if you want to check if you should test the config when applied.
     * @return true, if you should check for a save or test the new configuration
     */
    bool checkSaveandTestCommon(bool isSaveCheck);

    KScreen::ConfigPtr m_config = nullptr;
    KScreen::ConfigPtr m_initialConfig;
    KScreen::ConfigPtr m_previousConfig = nullptr;

    std::unique_ptr<ControlConfig> m_control;
    std::unique_ptr<ControlConfig> m_initialControl;
    Control::OutputRetention m_initialRetention = Control::OutputRetention::Undefined;
    QSize m_lastNormalizedScreenSize;
};

}
}

#endif // DDE_DISPLAY_CONFIG_H

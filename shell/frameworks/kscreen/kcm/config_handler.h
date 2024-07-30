/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "../common/control.h"

#include <kscreen/config.h>

#include <memory>

class OutputModel;

class ConfigHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConfigHandler(QObject *parent = nullptr);
    ~ConfigHandler() override = default;

    void setConfig(KScreen::ConfigPtr config);
    void updateInitialData();

    OutputModel *outputModel() const
    {
        return m_outputModel;
    }

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
        m_config = (m_previousConfig ? m_previousConfig : m_initialConfig)->clone();
    }

    KScreen::OutputPtr replicationSource(const KScreen::OutputPtr &output) const;
    void setReplicationSource(KScreen::OutputPtr &output, const KScreen::OutputPtr &source);

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
    void outputConnect(bool connected);

private:
    void checkScreenNormalization();
    QSize screenSize() const;
    void outputPrioritiesChanged();
    void initOutput(const KScreen::OutputPtr &output);
    /**
     * @brief checkSaveandTestCommon - compairs common config changes that would make the config dirty and needed to have the config checked when applied.
     * @param isSaveCheck - True  if your checking to see if the changes should request a save.
     *                      False if you want to check if you should test the config when applied.
     * @return true, if you should check for a save or test the new configuration
     */
    bool checkSaveandTestCommon(bool isSaveCheck);
    bool checkPrioritiesNeedSave();

    KScreen::ConfigPtr m_config = nullptr;
    KScreen::ConfigPtr m_initialConfig;
    KScreen::ConfigPtr m_previousConfig = nullptr;
    OutputModel *m_outputModel = nullptr;

    std::unique_ptr<ControlConfig> m_control;
    std::unique_ptr<ControlConfig> m_initialControl;
    QSize m_lastNormalizedScreenSize;
};

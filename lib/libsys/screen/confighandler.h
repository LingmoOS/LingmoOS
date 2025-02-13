/********************************************************************
Copyright © 2019 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef CONFIGHANDLER_H
#define CONFIGHANDLER_H

#include "./common/control.h"

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
        return m_outputs;
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

    void writeControl();

    void checkNeedsSave();

Q_SIGNALS:
    void outputModelChanged();
    void changed();
    void screenNormalizationUpdate(bool normalized);
    void needsSaveChecked(bool need);
    void retentionChanged();
    void outputConnect(bool connected);

private:
    void checkScreenNormalization();
    QSize screenSize() const;
    Control::OutputRetention getRetention() const;
    void primaryOutputSelected(int index);
    void outputPrioritiesChanged();
    void initOutput(const KScreen::OutputPtr &output);
    void resetScale(const KScreen::OutputPtr &output);

    KScreen::ConfigPtr m_config = nullptr;
    KScreen::ConfigPtr m_initialConfig;
    OutputModel *m_outputs = nullptr;

    std::unique_ptr<ControlConfig> m_control;
    std::unique_ptr<ControlConfig> m_initialControl;
    Control::OutputRetention m_initialRetention = Control::OutputRetention::Undefined;
    QSize m_lastNormalizedScreenSize;
};

#endif

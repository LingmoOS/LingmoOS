/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KDED_CONFIG_H
#define KDED_CONFIG_H

#include <kscreen/config.h>

#include <QOrientationReading>

#include <memory>

class ControlConfig;

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(KScreen::ConfigPtr config, QObject *parent = nullptr);
    ~Config() = default;

    QString id() const;

    bool fileExists() const;
    std::unique_ptr<Config> readFile();
    std::unique_ptr<Config> readOpenLidFile();
    bool writeFile();
    bool writeOpenLidFile();
    static QString configsDirPath();

    KScreen::ConfigPtr data() const
    {
        return m_data;
    }

    void activateControlWatching();
    bool autoRotationRequested() const;
    void setDeviceOrientation(QOrientationReading::Orientation orientation);
    bool getAutoRotate() const;
    void setAutoRotate(bool value);
    void log();

    void setValidityFlags(KScreen::Config::ValidityFlags flags)
    {
        m_validityFlags = flags;
    }

    bool canBeApplied() const;

Q_SIGNALS:
    void controlChanged();

private:
    friend class TestConfig;

    QString filePath() const;
    std::unique_ptr<Config> readFile(const QString &fileName);
    bool writeFile(const QString &filePath);

    bool canBeApplied(KScreen::ConfigPtr config) const;

    KScreen::ConfigPtr m_data;
    KScreen::Config::ValidityFlags m_validityFlags;
    ControlConfig *m_control;

    static QString s_configsDirName;
    static QString s_fixedConfigFileName;

};

#endif

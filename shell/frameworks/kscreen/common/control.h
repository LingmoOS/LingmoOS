/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <kscreen/output.h>
#include <kscreen/types.h>

#include <QList>
#include <QObject>
#include <QVariantMap>

class KDirWatch;

class Control : public QObject
{
    Q_OBJECT
public:
    explicit Control(QObject *parent = nullptr);

    ~Control() override = default;

    virtual bool writeFile();
    virtual void activateWatcher();

Q_SIGNALS:
    void changed();

protected:
    virtual QString dirPath() const;
    virtual QString filePath() const = 0;
    QString filePathFromHash(const QString &hash) const;
    void readFile();
    QVariantMap &info();
    const QVariantMap &constInfo() const;
    KDirWatch *watcher() const;

private:
    static QString s_dirName;
    QVariantMap m_info;
    KDirWatch *m_watcher = nullptr;
};

class ControlOutput;

class ControlConfig : public Control
{
    Q_OBJECT
public:
    explicit ControlConfig(KScreen::ConfigPtr config, QObject *parent = nullptr);

    qreal getScale(const KScreen::OutputPtr &output) const;
    void setScale(const KScreen::OutputPtr &output, qreal value);

    KScreen::OutputPtr getReplicationSource(const KScreen::OutputPtr &output) const;
    void setReplicationSource(const KScreen::OutputPtr &output, const KScreen::OutputPtr &source);

    uint32_t getOverscan(const KScreen::OutputPtr &output) const;
    void setOverscan(const KScreen::OutputPtr &output, const uint32_t value);

    KScreen::Output::VrrPolicy getVrrPolicy(const KScreen::OutputPtr &output) const;
    void setVrrPolicy(const KScreen::OutputPtr &output, const KScreen::Output::VrrPolicy value);

    KScreen::Output::RgbRange getRgbRange(const KScreen::OutputPtr &output) const;
    void setRgbRange(const KScreen::OutputPtr &output, const KScreen::Output::RgbRange value);

    QString dirPath() const override;
    QString filePath() const override;

    bool writeFile() override;
    void activateWatcher() override;

private:
    QVariantList getOutputs() const;
    void setOutputs(QVariantList outputsInfo);
    bool infoIsOutput(const QVariantMap &info, const QString &outputId, const QString &outputName) const;
    ControlOutput *getOutputControl(const QString &outputId, const QString &outputName) const;

    template<typename T, typename F>
    T get(const KScreen::OutputPtr &output, F globalRetentionFunc, T defaultValue) const;
    template<typename T, typename F, typename V>
    void set(const KScreen::OutputPtr &output, const QString &name, F globalRetentionFunc, V value);

    KScreen::ConfigPtr m_config;
    QStringList m_duplicateOutputIds;
    QList<ControlOutput *> m_outputsControls;
};

class ControlOutput : public Control
{
    Q_OBJECT
public:
    explicit ControlOutput(KScreen::OutputPtr output, QObject *parent = nullptr);

    QString id() const;
    QString name() const;

    // TODO: scale auto value

    qreal getScale() const;
    void setScale(qreal value);

    uint32_t overscan() const;
    void setOverscan(uint32_t value);

    KScreen::Output::VrrPolicy vrrPolicy() const;
    void setVrrPolicy(KScreen::Output::VrrPolicy value);

    KScreen::Output::RgbRange rgbRange() const;
    void setRgbRange(KScreen::Output::RgbRange value);

    QString dirPath() const override;
    QString filePath() const override;

private:
    KScreen::OutputPtr m_output;
};

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_CONTROL_H
#define COMMON_CONTROL_H

#include <kscreen/output.h>
#include <kscreen/types.h>

#include <QObject>
#include <QVariantMap>
#include <QVector>

class KDirWatch;

class Control : public QObject
{
    Q_OBJECT
public:
    enum class OutputRetention {
        Undefined = -1,
        Global = 0,
        Individual = 1,
    };
    Q_ENUM(OutputRetention)

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

    static OutputRetention convertVariantToOutputRetention(QVariant variant);

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

    OutputRetention getOutputRetention(const KScreen::OutputPtr &output) const;
    OutputRetention getOutputRetention(const QString &outputId, const QString &outputName) const;
    void setOutputRetention(const KScreen::OutputPtr &output, OutputRetention value);
    void setOutputRetention(const QString &outputId, const QString &outputName, OutputRetention value);

    qreal getScale(const KScreen::OutputPtr &output) const;
    void setScale(const KScreen::OutputPtr &output, qreal value);

    bool getAutoRotate(const KScreen::OutputPtr &output) const;
    void setAutoRotate(const KScreen::OutputPtr &output, bool value);

    bool getAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const;
    void setAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output, bool value);

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
    T get(const KScreen::OutputPtr &output, const QString &name, F globalRetentionFunc, T defaultValue) const;
    template<typename T, typename F, typename V>
    void set(const KScreen::OutputPtr &output, const QString &name, F globalRetentionFunc, V value);

    KScreen::ConfigPtr m_config;
    QStringList m_duplicateOutputIds;
    QVector<ControlOutput *> m_outputsControls;
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

    bool getAutoRotate() const;
    void setAutoRotate(bool value);

    bool getAutoRotateOnlyInTabletMode() const;
    void setAutoRotateOnlyInTabletMode(bool value);

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

#endif // COMMON_CONTROL_H

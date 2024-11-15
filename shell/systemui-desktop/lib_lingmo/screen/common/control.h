/********************************************************************
Copyright 2019 Roman Gilg <subdiff@gmail.com>

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

#ifndef COMMON_CONTROL_H
#define COMMON_CONTROL_H

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
    qreal getScale(const QString &outputId, const QString &outputName) const;
    void setScale(const KScreen::OutputPtr &output, qreal value);
    void setScale(const QString &outputId, const QString &outputName, qreal value);

    bool getAutoRotate(const KScreen::OutputPtr &output) const;
    bool getAutoRotate(const QString &outputId, const QString &outputName) const;
    void setAutoRotate(const KScreen::OutputPtr &output, bool value);
    void setAutoRotate(const QString &outputId, const QString &outputName, bool value);

    bool getAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output) const;
    bool getAutoRotateOnlyInTabletMode(const QString &outputId, const QString &outputName) const;
    void setAutoRotateOnlyInTabletMode(const KScreen::OutputPtr &output, bool value);
    void setAutoRotateOnlyInTabletMode(const QString &outputId, const QString &outputName, bool value);

    KScreen::OutputPtr getReplicationSource(const KScreen::OutputPtr &output) const;
    KScreen::OutputPtr getReplicationSource(const QString &outputId, const QString &outputName) const;
    void setReplicationSource(const KScreen::OutputPtr &output, const KScreen::OutputPtr &source);
    void setReplicationSource(const QString &outputId, const QString &outputName, const KScreen::OutputPtr &source);

    QString dirPath() const override;
    QString filePath() const override;

    bool writeFile() override;
    void activateWatcher() override;

private:
    QVariantList getOutputs() const;
    void setOutputs(QVariantList outputsInfo);
    bool infoIsOutput(const QVariantMap &info, const QString &outputId, const QString &outputName) const;
    ControlOutput *getOutputControl(const QString &outputId, const QString &outputName) const;

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

    QString dirPath() const override;
    QString filePath() const override;

private:
    KScreen::OutputPtr m_output;
};

#endif

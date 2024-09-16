// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESOLUTIONS_H_
#define RESOLUTIONS_H_

#include "resolution.h"
#include "graphicsdeviceinfo.h"

#include <QList>
#include <QObject>

class Resolutions
{
    friend class ResolutionsBuilder;

public:
    enum Type
    {
        NoResolution = 0,
        AMD,
        NVIDIA,
        INTEL,
        INTEL_NVIDIA,
        INTEL_NVIDIA_USE_INTEL,
        INTEL_NVIDIA_USE_NVIDIA
    };
    Q_DECLARE_FLAGS(ResolutionsType, Type)

    inline const int type() const { return m_type; }
    inline const QString name() const { return m_name; }
    inline const QString iconName() const { return m_iconName; }
    inline const QString statusScript() const { return m_statusScript; }
    QList<Resolution> resolutions() const { return m_resolutions; }
    Resolution getResolutionByName(QString name);
private:
    explicit Resolutions();

private:
    ResolutionsType m_type;
    QString m_name;
    QString m_iconName;
    QString m_statusScript;
    QList<Resolution> m_resolutions;
};

class ResolutionsBuilder
{
public:
    explicit ResolutionsBuilder(const GraphicsDeviceInfo &devInfo);

    ResolutionsBuilder config(const QString &config);
    Resolutions build();

private:
    QString m_config;
    GraphicsDeviceInfo m_devInfo;
};

#endif
// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resolutions.h"

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>



Resolutions::Resolutions()
{

}

Resolution Resolutions::getResolutionByName(QString name){
    Resolution resolution;
    foreach (const Resolution &resl, m_resolutions){
        if( name == resl.name()){
            resolution = resl;
        } 
    }
    return resolution;
}

ResolutionsBuilder::ResolutionsBuilder(const GraphicsDeviceInfo &devInfo) :
    m_config(RESOLUTIONS_DIR"/resolutions.json"),
    m_devInfo(devInfo)
{
    qDebug() <<"Resolutions json file:"<< RESOLUTIONS_DIR"/resolutions.json";
}

ResolutionsBuilder ResolutionsBuilder::config(const QString &config)
{
    m_config = config;

    return *this;
}

Resolutions ResolutionsBuilder::build()
{
    QFile config(m_config);
    const bool b = config.open(QIODevice::ReadOnly);
    Q_ASSERT_X(b, Q_FUNC_INFO, "read config file error");

    const int deviceType = m_devInfo.deviceFlag();
    const auto doc = QJsonDocument::fromJson(config.readAll());

    Resolutions r;
    for (const auto res : doc.array())
    {
        const auto object = res.toObject();
        for (const auto resl : object["resolutions"].toArray()) {
            Resolution solution(resl.toObject());
            switch (deviceType)
            {
            case (DeviceFlag::INTEL | DeviceFlag::AMD):
                if (DeviceFlag::INTEL == m_devInfo.curDevice().m_flag){
                    if(object["name"].toString() == "intel"){
                        if(r.m_type == Resolutions::NoResolution){
                            r.m_type = Resolutions::INTEL;
                            r.m_name = object["name"].toString();
                            r.m_iconName = object["icon_name"].toString();
                            r.m_statusScript = object["status"].toString();
                        }
                        //根据白名单确认使用glamor、uxa、sna三者中的一个，目前这了默认使用glamor
                        if(solution.name() == "glamor"){
                            r.m_resolutions.append(solution);
                        }
                    }
                }else{
                    if(object["name"].toString() == "amd"){
                        if(r.m_type == Resolutions::NoResolution){
                            r.m_type = Resolutions::AMD;
                            r.m_name = object["name"].toString();
                            r.m_iconName = object["icon_name"].toString();
                            r.m_statusScript = object["status"].toString();
                        }
                        if (m_devInfo.curDevice().m_driver == "amdgpu"){
                            if(solution.name() == "amdgpu") r.m_resolutions.append(solution);
                        }else {
                            if(solution.name() == "radeon") r.m_resolutions.append(solution);
                        }
                    }
                }
                break;
            case DeviceFlag::AMD:
                if(object["name"].toString() == "amd"){
                    if(r.m_type == Resolutions::NoResolution){
                        r.m_type = Resolutions::AMD;
                        r.m_name = object["name"].toString();
                        r.m_iconName = object["icon_name"].toString();
                        r.m_statusScript = object["status"].toString();
                    }
                    if (m_devInfo.curDevice().m_driver == "amdgpu"){
                        if(solution.name() == "amdgpu") r.m_resolutions.append(solution);
                    }else {
                        if(solution.name() == "radeon") r.m_resolutions.append(solution);
                    }
                }
                break;
            case DeviceFlag::INTEL:
                if(object["name"].toString() == "intel"){
                    if(r.m_type == Resolutions::NoResolution){
                        r.m_type = Resolutions::INTEL;
                        r.m_name = object["name"].toString();
                        r.m_iconName = object["icon_name"].toString();
                        r.m_statusScript = object["status"].toString();
                    }
                    //根据白名单确认使用glamor、uxa、sna三者中的一个，目前这了默认使用glamor
                    if(solution.name() == "glamor"){
                        r.m_resolutions.append(solution);
                    }
                }
                break;
            case DeviceFlag::NVIDIA:
                if(object["name"].toString() == "nvidia"){
                    if(r.m_type == Resolutions::NoResolution){
                        r.m_type = Resolutions::NVIDIA;
                        r.m_name = object["name"].toString();
                        r.m_iconName = object["icon_name"].toString();
                        r.m_statusScript = object["status"].toString();
                    }
                    if(solution.name() == "nvidia" || solution.name() == "nouveau"){
                        r.m_resolutions.append(solution);
                    }
                }
                break;
            case (DeviceFlag::INTEL | DeviceFlag::NVIDIA):
                if(!m_devInfo.isNotebook())
                {
                    if (DeviceFlag::INTEL == m_devInfo.curDevice().m_flag)
                    {
                        if(object["name"].toString() == "intel"){
                            if(r.m_type == Resolutions::NoResolution){
                                r.m_type = Resolutions::INTEL_NVIDIA_USE_INTEL;
                                r.m_name = object["name"].toString();
                                r.m_iconName = object["icon_name"].toString();
                                r.m_statusScript = object["status"].toString();
                            }
                            //根据白名单确认使用glamor、uxa、sna三者中的一个，目前这了默认使用glamor
                            if(solution.name() == "glamor"){
                                r.m_resolutions.append(solution);
                            }
                        }
                    }else if (DeviceFlag::NVIDIA == m_devInfo.curDevice().m_flag){
                        if(object["name"].toString() == "nvidia"){
                            if(r.m_type == Resolutions::NoResolution){
                                r.m_type = Resolutions::INTEL_NVIDIA_USE_NVIDIA;
                                r.m_name = object["name"].toString();
                                r.m_iconName = object["icon_name"].toString();
                                r.m_statusScript = object["status"].toString();
                            }
                            if(solution.name() == "nvidia" || solution.name() == "nouveau"){
                                r.m_resolutions.append(solution);
                            }
                        }
                    }
                }else{
                    if(object["name"].toString() == "intel"){
                        //根据白名单确认使用glamor、uxa、sna三者中的一个，目前这了默认使用glamor
                        if(solution.name() == "glamor"){
                            r.m_resolutions.append(solution);
                        }
                    }
                    if(object["name"].toString() == "nvidia_intel"){
                        if(r.m_type == Resolutions::NoResolution){
                            r.m_type = Resolutions::INTEL_NVIDIA;
                            r.m_name = object["name"].toString();
                            r.m_iconName = object["icon_name"].toString();
                            r.m_statusScript = object["status"].toString();
                        }
                        if(solution.name() == "bumblebee" || solution.name() == "prime"){
                            r.m_resolutions.append(solution);
                        }
                    }
                }
                break;
            }
        }
    }
    return r;
}

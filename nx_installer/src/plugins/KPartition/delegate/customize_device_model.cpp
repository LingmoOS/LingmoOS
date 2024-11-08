#include "customize_device_model.h"
#include "../PluginService/kcommand.h"
#include "partman/device.h"
#include "base_unit/file_unit.h"

#include <QDebug>
#include <QMap>
namespace KInstaller {
using namespace Partman;
using namespace KServer;

QMap<DeviceModelType, QString> m_devModelStrs =
{
    {DeviceModelType::HUAWEI_Kirin_990, "Kirin 990"},
    {DeviceModelType::HUAWEI_Kirin990, "Kirin990"},
    {DeviceModelType::HUAWEI_Kirin_9006C, "Kirin 9006C"},
    {DeviceModelType::HUAWEI_Kirin9006C, "Kirin9006C"},
    {DeviceModelType::FT_2000, "FT-2000"},
    {DeviceModelType::FT_1500, "FT-1500"},
};
//获取硬件CPU信息
DeviceModelType getDeviceModel()
{
    QString info;
    info = KReadFile("/proc/cpuinfo");
    for(int i = 0; i < m_devModelStrs.values().length(); i++) {
        if(info.contains(m_devModelStrs.values().at(i), Qt::CaseInsensitive)) {
            return m_devModelStrs.keys().at(i);
        }
    }
    return DeviceModelType::Normal;

}

QString getDeviceStrByType(DeviceModelType type)
{
//    for (int var = 0; var < m_devModelStrs.count(); ++var) {
//        if(type == m_devModelStrs.keys().at(var)) {
            return m_devModelStrs.value(type);
//        }
//    }
}

//针对不同的硬件设备定制机器需要判断的情况
DeviceList getDeviceListByDeviceModel(DeviceList devs)
{
    DeviceModelType type = getDeviceModel();
    DeviceList devices;
    switch (type) {
    case DeviceModelType::HUAWEI_Kirin990:
    case DeviceModelType::HUAWEI_Kirin_990:
    case DeviceModelType::HUAWEI_Kirin9006C:
    case DeviceModelType::HUAWEI_Kirin_9006C:
    {
        for(Device::Ptr dev : devs) {
            if(dev->m_path == "/dev/sda" || dev->m_path == "/dev/sdb" || dev->m_path == "/dev/sdc") {
                continue;
            } else
                devices.append(dev);
        }
        return devices;
    }
    case DeviceModelType::Normal:
    default:
        return devs;
    }
}

}

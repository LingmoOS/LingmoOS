#ifndef CUSTOMIZE_DEVICE_MODEL_H
#define CUSTOMIZE_DEVICE_MODEL_H
#include "partman/device.h"
namespace KInstaller {
using namespace Partman;

enum class DeviceModelType{
    HUAWEI_Kirin_990,
    HUAWEI_Kirin990,
    Kirin990,
    HUAWEI_Kirin_9006C,
    HUAWEI_Kirin9006C,
    FT_2000,
    FT_1500,
    Normal
};
//获取硬件CPU信息
DeviceModelType getDeviceModel();
QString getDeviceStrByType(DeviceModelType type) ;
//针对不同的硬件设备定制机器需要判断的情况
DeviceList getDeviceListByDeviceModel( DeviceList devs);


}
#endif // CUSTOMIZE_DEVICE_MODEL_H

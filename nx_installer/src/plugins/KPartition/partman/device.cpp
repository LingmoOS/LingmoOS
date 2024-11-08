#include "device.h"


namespace KInstaller {

namespace Partman {
Device::Device():
m_model(""),
m_osName(""),
m_path(""),
m_sectorSize(0),
m_sectors(0),
 m_diskFreeSpace(0),
  m_readyOnly(0),
  m_partTableType(PartTableType::Empty)
{

}
QString Device::getDevicePath()
{
    if(!m_path.isEmpty()) {
        return m_path;
    }
}

qint64 Device::getByteLength()
{
    if(m_length > 0) {
        m_diskTotal = m_length * m_sectorSize;
        return m_diskTotal;
    } else {
        return -1;
    }
}

qint64 Device::getDiskUsedPercent()
{
    m_diskFreeSpace = 0;
    for(int i = 0; i < partitions.length(); i++) {
        m_diskFreeSpace += partitions.at(i)->m_freespace;
    }

    return m_diskFreeSpace;
}

int deviceIndex(const DeviceList &devices, const QString &devPath)
{
    for(int i = 0; i < devices.length(); i++) {
        if(devices.at(i)->m_path == devPath) {
            return i;
        }
    }
    return -1;
}
}

}

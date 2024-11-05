#include "ksystemsetting_unit.h"
#include <QSettings>
#include <QDebug>
#include "base_unit/file_unit.h"
#include "kdir.h"
#include <stdio.h>
using namespace std;
namespace KServer {





QString ReadSettingIni(QString group, QString key)
{
    QString path = QString("");
    path = GetLingmoInstallPath() + "/ky-installer.cfg";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup(group);
    QString value =setting.value(key).toString();
    setting.endGroup();
    return (QString)value;
}

//bool WritePwdToIni(QString group, QString key, QString value)
//{
//    FILE *file = fopen(GetLingmoInstallPath() + "/ky-installer.cfg", "wb+");
//    char str[1024] = {0};
//    if(file == NULL) {
//        return false;
//    }
//    fgets(str, 1024, file);


//}

bool WriteSettingToIni(QString group, QString key, QString value)
{
    QString path = QString("");
    path = GetLingmoInstallPath() + "/ky-installer.cfg";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup(group);
    if(key == "password" || key == "encryptyPWD" ) {
        QByteArray list = value.toLocal8Bit();
        setting.setValue(key, list);
    } else {
        setting.setValue(key, value);
    }

    setting.endGroup();
    return true;
}

QString getPWqualityConfPath()
{
    QString str = QString("");
    str = GetLingmoInstallPath() + "/pwquality.conf";
    return (QString)str;
}

bool getSettingBool(QString key)
{
    QVariant value = ReadSettingIni("setting", key);
    if(value.isValid()) {
        return value.toBool();
    }
    qCritical() << "getSettingBool() failed" << key;
    return false;
}

void writePartitionInfo(QString& rootdisk, QString& rootpart, QString& bootpath, QString& mountpoints)
{
    qDebug() << "writePartitionInfo()";
    QString path = QString("");
    path = GetLingmoInstallPath() + "/ky-installer.cfg";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("partition");
    setting.setValue("KI_ROOT_DISK", rootdisk);
    setting.setValue("KI_ROOT_PARTITION", rootpart);
    setting.setValue("KI_BOOTLOADER", bootpath);
    setting.setValue("KI_MOUNTPOINTS", mountpoints);
    setting.endGroup();
}

void writeRequiringSwapFile(bool isRequired)
{
    QString path = QString("");
    path = GetLingmoInstallPath() + "/ky-installer.cfg";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("partition");
    setting.setValue("KI_SWAP_FILE", isRequired);
    setting.endGroup();
}

void writeUEFI(bool isEfi)
{
    QString path = QString("");
    path = GetLingmoInstallPath() + "/ky-installer.cfg";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("partition");
    setting.setValue("KI_UEFI", isEfi);
    setting.endGroup();
}

void writeDiskPartitionSetting(DiskPartitionSetting& set)
{
    writePartitionInfo(set.rootDisk, set.rootPartition, set.bootPartition, set.mountPoints);
    writeRequiringSwapFile(set.swapFileRequire);
    writeUEFI(set.uefiRequired);
}

}

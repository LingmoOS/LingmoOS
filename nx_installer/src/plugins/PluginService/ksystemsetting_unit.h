#ifndef KSYSTEMSETTING_UNIT_H
#define KSYSTEMSETTING_UNIT_H
#include <QString>
class QString;
class QSettings;
namespace KServer {

struct DiskPartitionSetting
{
    DiskPartitionSetting(){}
    QString rootDisk;
    QString rootPartition;
    QString bootPartition;
    QString mountPoints;
    bool swapFileRequire = false;
    bool uefiRequired = false;
};




//从systemsetting.ini文件中，读取当前语言设置
//QString ReadSettingIniSet(QString key);

//将语言设置写入systemsetting.ini文件
//bool WriteSettingIniSet(QString key, QString value);
//将自定义分区设置写入systemsetting.ini文件
//bool WriteSettingIniCustomSet(QString key, QString value);

//获取密码复杂度的配置文件
QString getPWqualityConfPath();

//不同的硬件架构，支持不同的文件系统
//QString getSettingValue(QString key);

//通用写入配置文件设置函数
bool WriteSettingToIni(QString group, QString key, QString value);
bool WritePwdToIni(QString group, QString key, QString value);
QString ReadSettingIni(QString group, QString key);

bool getSettingBool(QString key);

void writePartitionInfo(QString& rootdisk, QString& rootpart, QString& bootpath, QString& mountpoints);
void writeRequiringSwapFile(bool isRequired);
void writeUEFI(bool isEfi);
void writeDiskPartitionSetting(DiskPartitionSetting& set);
}

#endif // KSYSTEMSETTING_UNIT_H

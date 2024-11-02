#include "partition_format.h"
#include "partition.h"
#include "../PluginService/kcommand.h"
#include "../delegate/customize_device_model.h"
#include "../PluginService/sysInfo/machine.h"


#include "partition_unit.h"
#include <memory>
#include <QMap>
#include <QDebug>
using namespace KServer;
using namespace std;
namespace KInstaller {
namespace Partman {

class  PartitionFormater
{
public:
   explicit  PartitionFormater(Partition::Ptr partition)
        :m_partition(partition) {}
    virtual ~PartitionFormater() {}

    virtual QString command()
    {
        return FSFormatCmdMap[m_partition->fs];
    }

    virtual QStringList args()
    {
        return {};
    }

    virtual bool exec()
    {
        if(!KServer::KCommand::getInstance()->RunScripCommand(command(), args())) {
            KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
            KServer::KCommand::getInstance()->RunScripCommand(command(), args());
        }
        return true;
    }

    inline QString FSlabel()
    {
        return m_partition->m_FSlabel;
    }

    inline bool choiceFSIsEmpty()
    {
        return FSlabel().isEmpty();
    }

    inline QString path()
    {
        return m_partition->m_partitionPath;
    }

private:
    Partition::Ptr m_partition;
};
class BtrFSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-f", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        qDebug() << "partprob:" << partbrobl;
        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(255);
            args << QStringList{"-L",labelFS};
            qDebug() << "PartitionFormater args:" << args;
        }
        return args;
    }
};

class Ext2FSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-F", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(16);
            args << QStringList{"-L",labelFS};
            qDebug() << "PartitionFormater args:" << args;
        }
        return args;
    }
};

class Ext3FSFormater :public Ext2FSFormater
{
public:
    using Ext2FSFormater::Ext2FSFormater;
};
class Ext4FSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-F", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
             args << QStringList{"-L",FSlabel()};
        }
        const MachineArch arch = getMachineArch();
        if(arch == MachineArch::LOONGSON || arch == MachineArch::SW) {
            args << QStringList{"-O ^64bit"};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};
class Fat12FSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-F12", "-v", "-I", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(11);
            args << QStringList{"-n",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};

class Fat16FSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-F16", "-v", "-I", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(11);
            args << QStringList{"-n",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};

class Fat32FSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-F32", "-v", "-I", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(11);
            args << QStringList{"-n",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};

class NtFSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-Q", "-v", "-F", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(128);
            args << QStringList{"-L",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};

class LinuxSwapFSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-f", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(15);
            args << QStringList{"-L",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};

class Reiser4FSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"--force", "--yes", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(16);
            args << QStringList{"--label",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};

class ReiserFSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-f", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(16);
            args << QStringList{"--label",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};
class XFSFormater :public PartitionFormater
{
public:
    using PartitionFormater::PartitionFormater;
    QStringList args() override
    {
        QStringList args{"-f", path()};
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});
//        bool partbrobl = KServer::KCommand::getInstance()->RunScripCommand("partprobe", {path()});
//        KServer::KCommand::getInstance()->RunScripCommand("sleep", {"1"});

        if(!choiceFSIsEmpty()) {
            const QString labelFS = FSlabel().left(12);
            args << QStringList{"--label",labelFS};
        }
         qDebug() << "PartitionFormater args:" << args;
        return args;
    }
};



bool MKfs(Partition::Ptr partition)
{
    qDebug() << "MKfs格式化文件系统";
    using FSFormat = std::shared_ptr<PartitionFormater>;

    QMap<FSType, std::shared_ptr<PartitionFormater>> mapFS {
        {FSType::BtrFS, FSFormat(new BtrFSFormater(partition))},
        {FSType::Ext2, FSFormat(new Ext2FSFormater(partition))},
        {FSType::Ext3, FSFormat(new Ext3FSFormater(partition))},
        {FSType::Ext4, FSFormat(new Ext4FSFormater(partition))},
        {FSType::LingmoData, FSFormat(new Ext4FSFormater(partition))},
        {FSType::Fat12, FSFormat(new Fat12FSFormater(partition))},
        {FSType::Fat16, FSFormat(new Fat16FSFormater(partition))},
        {FSType::Fat32, FSFormat(new Fat32FSFormater(partition))},
        {FSType::EFI, FSFormat(new Fat32FSFormater(partition))},
        {FSType::LinuxSwap, FSFormat(new LinuxSwapFSFormater(partition))},
        {FSType::Reiser4, FSFormat(new Reiser4FSFormater(partition))},
        {FSType::ReiserFS, FSFormat(new ReiserFSFormater(partition))},
        {FSType::XFS, FSFormat(new XFSFormater(partition))},
        {FSType::NtFS, FSFormat(new NtFSFormater(partition))}
    };
    if(!mapFS.keys().contains(partition->fs)) {
        qWarning() << "没有可支持的文件系统进行格式化";
        return false;
    }
        if(partition->fs == FSType::EFI) {
        QString tmp = getDeviceStrByType(getDeviceModel());
        tmp.remove(QRegExp("\\s"));//去除字符串中间空格
        if(tmp.contains("Kirin990") || tmp.contains("Kirin9006C")) {
            PartitionFormater* format = new Fat16FSFormater(partition);
            return format->exec();
        }
    }
    return mapFS[partition->fs]->exec();

}


}
}

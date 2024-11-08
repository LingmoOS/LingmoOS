#include "kernelfilesystemfs.h"
#include <QString>
#include <QDebug>
#include "../ksystemsetting_unit.h"


QStringList getKernelFs()
{
    QStringList list = {};
    QStringList list1 = {};
    QStringList res = {};
    const QString cmd{"/bin/bash"};
    QString arg{"cat /proc/filesystems | grep -v nodev"};
    QString output, error, dir;
    int exitcode = 0;
    if(KServer::KCommand::getInstance()->RunScripCommand(cmd, {"-c" ,arg}, dir, output, error, exitcode)){
        output.remove(0, 1);
        output.remove(output.size() -1, 1);
        list = output.replace("\n\t", " ").split(" ", QString::SkipEmptyParts );
        qDebug() << list;
    } else {
        qWarning() << QString("cat /proc/filesystems: Failed:%1").arg(error);
    }
    list1 = KServer::ReadSettingIni("setting", "FileSystem").split(";");
    for (int j = 0; j < list.count(); ++j) {
        for (int i = 0; i < list1.count(); ++i) {
            if(list.at(j) == list1.at(i)) {
                res.append(list.at(j));
                break;
            }
        }
    }
    res.append("lingmo-data");
    res.append("efi");
    res.append("linux-swap");
    res.append("unused");

    if(res.contains("ext4"))//保证第一个是ext4分区
    {
        if(res.removeOne("ext4"))
        {
            res.insert(0,"ext4");
        }
    }

    return res;
}


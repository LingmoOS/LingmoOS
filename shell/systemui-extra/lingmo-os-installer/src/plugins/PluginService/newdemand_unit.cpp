#include "newdemand_unit.h"
#include <QFile>
#include <QDebug>
#include <QDir>
const QString failstr = "SI_FAILED";
bool InstallAddFileToEFI()
{
    QString path = "/target/boot/efi/";
    QDir dir(path);
    if(!dir.exists()) {
        qDebug() << "/target/boot/efi is not exist.";
        return false;
    }
    QFile file(path + failstr);
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    qDebug() << "Create /target/boot/efi/SI_FAILED.";
    return true;

}

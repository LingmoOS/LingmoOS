#include "file_unit.h"
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>

namespace KServer {

QString KReadFile(const QString& path)
{
    QString str = "";
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "文件不存在" << path;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "打开文件失败" << path;
    } else {
        str = QString::fromUtf8(file.readAll());
        file.close();
    }
    return (QString)str;
}

}

#include <QProcess>
#include <QDebug>

#include "hostname.h"

void HostNameChanger::onHostNameChanged(const QString &newHostName) {
    // QProcess process;
    // QStringList arguments;
    // arguments << "general" << "hostname" << newHostName;
    // process.start("nmcli", arguments);
    QString chosts = QString("pkexec sed -i 's/127.0.1.1[[:space:]]*\\(.*\\)/127.0.1.1  %1/g' /etc/hosts").arg(newHostName);
    QProcess process;
    // 使用nmcli设置系统主机名
    process.start("nmcli", {"general", "hostname", newHostName});
    if (!process.waitForFinished()) {
        qDebug() << "Failed to change hostname:" << process.errorString();
    } else {
        qDebug() << "Hostname changed to:" << newHostName;
    }
    process.start("bash", QStringList() << "-c" << chosts);
    if (!process.waitForFinished()) {
        qDebug() << "Failed to modify /etc/hosts:" << process.errorString();
    } else {
        qDebug() << "/etc/hosts modified successfully";
    }

    // 检查命令是否执行成功
    // if (process.exitCode() == 0) {
    //     qDebug() << "Exit(0)" << chosts;
    // } else {
    //     qDebug() << "Error：" << chosts;
    //     qDebug() << "[Info]" << process.errorString();
    // }
}

// void HostNameChanger::onHostNameChanged(const QString &newHostName) {
//     qDebug() << "New host name:" << newHostName;
//     qDebug() << "Hello, World!";
// }
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QFileInfoList>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "welcome_manager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载翻译
    QTranslator translator;
    QLocale locale = QLocale::system();
    
    // 设置翻译文件路径
    QString qmFilePath = QString("%1/%2.qm")
            .arg("/usr/share/lingmo-welcome/translations/")
            .arg(locale.name());
            
    if (QFile::exists(qmFilePath)) {
        if (translator.load(qmFilePath)) {
            a.installTranslator(&translator);
        }
    }

    QString filePath = QDir::homePath() + "/.islin_install_lock";
    bool result = QFile::remove(filePath);
    if (result) {
        qDebug() << "文件删除成功：" << filePath;
    } else {
        qDebug() << "文件删除失败：" << filePath;
    }

    WelcomeManager w;
    w.show();

    return a.exec();
} 

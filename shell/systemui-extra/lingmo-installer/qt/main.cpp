#include "kreinstall.h"

#include <QApplication>
#include <QDesktopWidget>
#include "xatom-helper.h"
//#include "qtsingleapplication.h"
#include "adaptscreeninfo.h"
#include <qlogging.h>
#include <stdio.h>
#include <stdlib.h>

#include <QLibraryInfo>
#include <QTranslator>
//#include <lingmo-log4qt.h>
//#include <log.hpp>
#include <QSettings>
#include <lingmosdk/lingmosdk-system/libkysysinfo.h>
#include <qlogging.h>
#include <QDir>
#include <singleapplication.h>
#include <lingmostylehelper/lingmostylehelper.h>
#include <signal.h>

#include <QtCore/QCoreApplication>
#include <QSettings>
#include <iostream>
#include "widbox.h"
#include <QCryptographicHash>
QString getDebInfo(QString m_debPath, QString type)
{
    QProcess terminal;
    QStringList cmdList;
    m_debPath = QDir::toNativeSeparators(m_debPath);
    qWarning() << "m_debPath" <<m_debPath;


    QApt::DebFile debfile(m_debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }

    QRegExp rx("[^a-zA-Z0-9]");
    int pos = rx.indexIn(m_debPath);
    if (pos > -1)
        m_debPath.toHtmlEscaped();

    QFileInfo file(m_debPath);

    cmdList << "-c" << QString("dpkg -I " + m_debPath);
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();

    QString debinfoerro = terminal.readAllStandardError().data();
    if ((debInfo.contains(type) || debInfo.contains(type.toLower())) && debinfoerro=="" ){
        QString ret;
        QStringList list = debInfo.split("\n");
        foreach (QString str, list) {
            if(type == "Package"){
                if (str.left(9) == " Package:"){
                    ret = str.remove(" Package:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Version"){
                if (str.left(9) == " Version:"){
                    ret = str.remove(" Version:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Architecture"){
                if (str.left(14) == " Architecture:"){
                    ret = str.remove(" Architecture:").remove("\n").remove(" ");
                    break;
                }
            }
        }
        if ( debInfo!="" && debInfo.contains(type)){
            qInfo() << ret << "=======================";
           return ret;
        }
    }
    else if (debInfo!="" && debinfoerro.contains("压缩数据已损坏") || debinfoerro.contains("གནོན་སྐྱུང་གཞི་གྲངས་ལ་སྐྱོན་ཤོར་བ།"))
    {
        //deb包已损坏，无法解析
        return "The software package is damaged";//软件包已损坏
    }
    else if (file.exists() && debInfo=="")
    {
        //路径包含特殊字符，需要debfile解析
        //QApt::DebFile::isValid();
        QApt::DebFile debfile(m_debPath);
        if (debfile.isValid()){
            if (type == "Package"){
                return debfile.packageName();}
            else if (type == "Version"){
                return debfile.version();
            }
            else{
                return debfile.architecture();
            }
        }
        else{
            return "The software package is damaged";
        }
    }

}
int main(int argc, char *argv[])
{

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QString id = QString("lingmo-installer-" + QLatin1String(getenv("DISPLAY")));
    //QtSingleApplication a(id, argc, argv);
    kdk::QtSingleApplication a(id,argc, argv);

    QFileInfo file("/usr/share/qt5/translations/qt_zh_CN.qm");
    QTranslator * translator = new QTranslator();
    if(file.exists()!=false){
        translator->load("/usr/share/qt5/translations/qt_zh_CN.qm");
        a.installTranslator(translator);
    }
    a.setStyle("lingmo");
    if (translator->load(QLocale(), QLatin1String("kre-install"), QLatin1String("_"), QLatin1String("/usr/share/kre/kre-install/translations")))
    {
        qDebug() << "oioioioi";
        a.installTranslator(translator);
    }
    else
        qDebug() << "cannot load translator kre-install_" << QLocale::system().name() << ".qm!";
    QTranslator trans1;
    QString local1 = QLocale::system().name();
    if(trans1.load(QString(":translations/gui_" + local1 +".qm")))
    {
        a.installTranslator(&trans1);
    }
    a.setApplicationName(QObject::tr("lingmo-installer"));
    if (a.isRunning()) {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("lingmo-installer is already running!");
        return EXIT_SUCCESS;
    } else {
        setenv("QT_QPA_PLATFORMTHEME" , "lingmo" , true);
        if (argv[1]){
            std::string strTmp = argv[1];
            QString str = QString::fromStdString(strTmp);
            QFileInfo fileinfo(str);
            QString lang = getenv("LANG");
            if (!fileinfo.isFile()) {
                if (lang.contains("zh_CN"))
                    qWarning() << str << "不是一个文件";
                else
                    qWarning() << str << "is not file";
                exit(0);
            }
            /*判断文件名是否有特殊字符*/
            QString debPathname = str;
            if (debPathname.contains("`",Qt::CaseSensitive) || debPathname.contains("$(",Qt::CaseSensitive) || debPathname.contains(";",Qt::CaseSensitive)
                   || debPathname.contains("&",Qt::CaseSensitive) || debPathname.contains("|",Qt::CaseSensitive) ){
                QMessageBox msgBox;
                // msgBox.setWindowTitle(tr("open file error"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText(QObject::tr("Open file error\n"
                                  "Naming contains nonstandard characters '` $( & | '"));
                QPushButton *btn = new QPushButton();
                btn->setText(QObject::tr("OK"));
                msgBox.addButton(btn, QMessageBox::AcceptRole);
                msgBox.exec();
                QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
                kdk::WindowManager::setGeometry(msgBox.windowHandle(),QRect((availableGeometry.width() - msgBox.width())/2, (availableGeometry.height() - msgBox.height())/2,msgBox.width(),msgBox.height()));
                exit(0);
            }


            QMimeDatabase mimeDatabase;
            QMimeType mimeType = mimeDatabase.mimeTypeForFile(str);
            //安卓软件包的mimetype
            if (mimeType.name() == "application/vnd.debian.binary-package")
                qInfo() << "This is a deb package file.";
            else if (mimeType.name() == "application/vnd.android.package-archive"){
                QString debPath = fileinfo.absoluteFilePath();
                kreInstall w (debPath);
                w.showWindow();
                a.setActivationWindow(&w);
                QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
                return a.exec();
            }
            else {
                if (lang.contains("zh_CN"))
                    qWarning() << str <<"这个文件不是一个Debian类型软件包.";
                else
                    qWarning() << str <<"This is not a deb package file.";
                exit(0);
            }
            QString debPath = fileinfo.absoluteFilePath();
            a.setWindowIcon(QIcon::fromTheme("lingmo-installer"));
            QString name = getDebInfo(debPath,"Package");
            QString Architecture = getDebInfo(debPath,"Architecture");
            QProcess *process = new QProcess();
            process->start("dpkg --print-architecture");
            process->waitForFinished(-1);
            QString arch = process->readLine();
            arch.remove("\n");
            if (name == "The software package is damaged")
            {
                QString  errroinfo = QObject::tr("The software package is corrupted and cannot be installed!");

                // QWidget *widget = new widbox(errroinfo);
                QScopedPointer<QWidget> widget(new widbox(errroinfo));
                a.setActivationWindow(widget.data());
                QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
                kdk::WindowManager::setGeometry(widget->windowHandle(),QRect((availableGeometry.width() - widget->width())/2, (availableGeometry.height() - widget->height())/2,widget->width(),widget->height()));
                widget->show();
                return a.exec();
            }
            else if (arch.compare(Architecture) && Architecture.compare("all"))
            {
                if (Architecture != ""){
                    QString  errroinfo = QObject::tr("The software package architecture does not match the native architecture");
                    QScopedPointer<QWidget> widget(new widbox(errroinfo)); // 使用QScopedPointer来管理指针
                    a.setActivationWindow(widget.data());
                    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
                    kdk::WindowManager::setGeometry(widget->windowHandle(),QRect((availableGeometry.width() - widget->width())/2, (availableGeometry.height() - widget->height())/2,widget->width(),widget->height()));
                    widget->show();
                    return a.exec();
                }
                else
                {
                    QString  errroinfo = QObject::tr("The package format is not Debian format and cannot be opened.");
                    QScopedPointer<QWidget> widget(new widbox(errroinfo)); // 使用QScopedPointer来管理指针
                    a.setActivationWindow(widget.data());
                    widget->show();
                    return a.exec();
                }
            }
            else{
                kreInstall w(debPath);
                w.showWindow();
                a.setActivationWindow(&w);
                QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
                return a.exec();
            }
        }
        else{
            a.setWindowIcon(QIcon::fromTheme("lingmo-installer"));
            kreInstall w ("Batch installation");
            w.showWindow();
            a.setActivationWindow(&w);
            QObject::connect(&a, SIGNAL(messageReceived(const QString&)), &w, SLOT(secondaryInstallation(const QString&)));
            return a.exec();
        }

    }
    signal(SIGCHLD, SIG_IGN);
}




// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsdriverinterface.h"
#include <QTime>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include <QFile>
#include <QJsonArray>
#include <QRegExp>
using namespace std;

#define INSTALLER_DESKTOP_FILE_SOURCE "/usr/lib/deepin-graphics-driver-manager/deepin-gradvrmgr-installer.desktop"
#define INSTALLER_DESKTOP_FILE_DEST "/etc/xdg/autostart/deepin-gradvrmgr-installer.desktop"

#define TEST_INSTALLER_DESKTOP_FILE_SOURCE "/usr/lib/deepin-graphics-driver-manager/deepin-gradvrmgr-test-installer.desktop"
#define TEST_INSTALLER_DESKTOP_FILE_DEST "/etc/xdg/autostart/deepin-gradvrmgr-test-installer.desktop"

GraphicsDriverInterface::GraphicsDriverInterface(QObject* parent)
    : QObject(parent),
      m_resolutions(ResolutionsBuilder(m_devInfo).build())
{
    // TODO: Needs implementation
}

GraphicsDriverInterface::~GraphicsDriverInterface()
{
    // TODO: Needs implementation
}

bool GraphicsDriverInterface::command(const QString &cmd, const QStringList &args, QString &output) {
    QProcess *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    connect(proc, &QProcess::readyReadStandardOutput, this, [&]() {
        QString out = proc->readAllStandardOutput();
        //qDebug() << "comand out:" << out;
        output += out;
    });
    proc->start(cmd, args);
    proc->waitForStarted(-1);
    proc->waitForFinished();
    if ( QProcess::NormalExit != proc->exitStatus()  || proc->error() < QProcess::UnknownError){
        qWarning("Excute the comand(%s) failed!", qPrintable(cmd));
        qWarning() << "Error:" << proc->error();
        qWarning() << "Status:" << proc->exitStatus();
        return false;
    }
    return true;
}


QString GraphicsDriverInterface::GetKernelVersion() {
    QString output;
    if (!command("uname", QStringList() << "-r", output)){
        return nullptr;
    }
    return output.split("-")[0];
}

QString GraphicsDriverInterface::GetCurrPackageVersion(QString pkg_name){
    QString output;
    QStringList args;
    args << "-s" << pkg_name;
    if (!command("dpkg", args, output)){
        return nullptr;
    }
    QStringList line_list = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    int index = 0;
    for(; index < line_list.size(); index++){
        if (line_list[index].contains("Version: "))
            break;
    }
    if ( index == line_list.size()){
        return nullptr;
    }
    return line_list[index].split(" ")[1].split("-")[0];
}

QString GraphicsDriverInterface::GetDepoPackageVersion(QString pkg_name){
    QString output;
    QStringList args;
    args << "show" << pkg_name;
    if (!command("apt", args, output)){
        return nullptr;
    }
    QStringList line_list = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    int index = 0;
    for(; index < line_list.size(); index++){
        if (line_list[index].contains("Version: "))
            break;
    }
    if ( index == line_list.size()){
        return nullptr;
    }
    return line_list[index].split(" ")[1].split("-")[0];
}


QString GraphicsDriverInterface::GetDevice()
{
    QJsonObject devices;
    QJsonArray json_array;
    foreach (const Device device, m_devInfo.devices()){
        QJsonObject devObj;
        devObj.insert("name", device.m_name);
        devObj.insert("info", device.m_info);
        json_array.append(devObj);
    }
    devices.insert("devices", json_array);
    return QString(QJsonDocument(devices).toJson());
}

QString GraphicsDriverInterface::GetResolutionTitle()
{
    QJsonObject ResolutionTitle;
    ResolutionTitle.insert("name", m_resolutions.name());
    ResolutionTitle.insert("type", m_resolutions.type());
    QJsonArray json_array;
    foreach (const Resolution resl, m_resolutions.resolutions()){
        QJsonObject reslObj;
        reslObj.insert("name", resl.name());
        reslObj.insert("driver", resl.driver());
        reslObj.insert("title", resl.title());
        reslObj.insert("description", resl.description());
        QString curr_version;
        QString depo_version;
        if ("nvidia" == resl.driver()){
            curr_version = GetCurrPackageVersion("nvidia-driver");
            if (curr_version.isEmpty()){
                curr_version = GetCurrPackageVersion("nvidia-driver-bin");
            }
            depo_version = GetDepoPackageVersion("nvidia-driver");
        }else{
            curr_version = GetKernelVersion();
            depo_version = GetKernelVersion();
        }
        reslObj.insert("currVersion", curr_version);
        reslObj.insert("depoVersion", depo_version);

        if(resl.name() == GetCurrDriverName()){
            reslObj.insert("enable", true);
        }else{
            reslObj.insert("enable", false);
        }
        json_array.append(reslObj);
    } 
    ResolutionTitle.insert("resolutions", json_array);
    return QString(QJsonDocument(ResolutionTitle).toJson());
}

void GraphicsDriverInterface::PrepareInstall(QString name, QString language)
{
    Resolution new_resl;
    Resolution old_resl;
    QString old_name = GetCurrDriverName();
    if ( name.isEmpty() || old_name.isEmpty() ){
        qWarning("Resolution name is invalid!");
        return;
    }
    // if ( name == old_name ){
    //     qWarning("The new resolution and the old one cannot be the same!");
    //     return;
    // }
    new_resl = m_resolutions.getResolutionByName(name);
    if ( new_resl.isNull() ){
        qWarning("Faild to get new resolution!");
        return;
    }
    old_resl = m_resolutions.getResolutionByName(old_name);
    if ( old_resl.isNull() ){
        qWarning("Faild to get old resolution!");
        return;
    }

    const QString install = scriptAbsolutePath(new_resl.installScript());
    const QString removeOld = scriptAbsolutePath(old_resl.removeScript());
    Q_ASSERT(!install.isEmpty() && !removeOld.isEmpty());

    QProcess *proc = new QProcess(this);
    QPROCESS_DELETE_SELF(proc);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, [=](int exitCode) {
        if (exitCode){ //失败
            qWarning() << "Prepare install failed, ExitCode: " << exitCode;
            Q_EMIT ReportProgress("-1");
        }else{ //成功
            QFile installerDesktopFileSource(TEST_INSTALLER_DESKTOP_FILE_SOURCE);
            if (installerDesktopFileSource.exists()){
                installerDesktopFileSource.copy(QDir::rootPath() + TEST_INSTALLER_DESKTOP_FILE_DEST);
                qDebug() << "Prepare install scucess!"; 
                Q_EMIT ReportProgress("100");
            }else{
                qWarning() << TEST_INSTALLER_DESKTOP_FILE_SOURCE << "do not exists!";
                Q_EMIT ReportProgress("-1");
            }
        }
    });

    connect(this, &GraphicsDriverInterface::Cancel, proc, [=]{
        qDebug()<< "Cancel prepare install";
        QFile overlay_flag("/usr/lib/deepin-graphics-driver-manager/working-dir/test_in_overlay_flag");
        if (overlay_flag.exists())
	    {
	        overlay_flag.remove();
	    }
        proc->kill();
        proc->waitForFinished(-1);
    });

    connect(proc, &QProcess::started, this, [=] {
        qDebug() << "PrepareInstall start!";
        Q_EMIT ReportProgress("0");//开始
    });

    const QString &exit_gltest = new_resl.keep_gltest() ? "false" : "true";
    const QString &new_driver = new_resl.name();
    //const QString &lang = QLocale().name();
    const QString &sc = scriptAbsolutePath("dgradvrmgr-prepare.sh");
    const QString &old_driver = old_resl.name();

    QString cmd = QString("%1 %2 %3 %4 %5 %6 %7").arg(sc, removeOld, install, old_driver, new_driver, language, exit_gltest);
    qDebug() << "cmd: " << cmd;
    proc->start(cmd);
}

bool GraphicsDriverInterface::isInOverlayRoot()
{
    QString output;
    QStringList args;
    args << "-h";
    if (!command("df", args, output)){
        qWarning() << "Execute df -h failed!";
        return false;
    }
    qDebug() << "output: " << output;
    QStringList line_list = output.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    int index = 0;
    for(; index < line_list.size(); index++){
        if (line_list[index].contains("overlayroot"))
            return true;
    }
    return false;
}

void GraphicsDriverInterface::CancelInstall()
{
    Q_EMIT Cancel();

    QProcess *proc = new QProcess(this);
    QPROCESS_DELETE_SELF(proc);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, [=](int exitCode) {
        if (exitCode){ //失败
            qWarning() << "Cancel install failed, ExitCode: " << exitCode;
            Q_EMIT ReportProgress("-1");
        }else{ //成功
            qDebug() << "Cancel install success!";
            Q_EMIT ReportProgress("100");
        }
    });

    connect(proc, &QProcess::started, this, [=] {
        qDebug() << "Cancel install start!";
        Q_EMIT ReportProgress("0");//开始
    });


    connect(proc, &QProcess::readyReadStandardOutput, this, [=]() {
        QString out = proc->readAllStandardOutput();
        qDebug() << out;
    });

    const QString &cmd = scriptAbsolutePath("dgradvrmgr-cancel.sh");
    proc->start(cmd);
    proc->waitForFinished(-1);
}

void GraphicsDriverInterface::TestSuccess()
{
    QProcess *proc = new QProcess(this);
    QPROCESS_DELETE_SELF(proc);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    proc->execute("/usr/lib/deepin-graphics-driver-manager/gltest-set-success.sh");
}

bool GraphicsDriverInterface::IsTestSuccess()
{
    QFile desktop_file(INSTALLER_DESKTOP_FILE_DEST);
    if (desktop_file.exists())
	{
		desktop_file.remove();
    }
    const QString &config_file = scriptAbsolutePath("working-dir/config.conf");
    if (!QFile(config_file).exists())
        return  false;

    QSettings *settings = new QSettings(config_file, QSettings::IniFormat);
    settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

    const bool gltestSuccess = settings->value("gltest-success", false).toBool();
    qDebug() << "gltestSuccess is:" << gltestSuccess;

    return gltestSuccess;
}


void GraphicsDriverInterface::RealInstall()
{
    const QString install_script("dgradvrmgr-real-install.sh");
    Q_ASSERT(!install_script.isEmpty());
    Install(install_script);
}


void GraphicsDriverInterface::TestInstall()
{
    const QString install_script("dgradvrmgr-test-install.sh");
    Q_ASSERT(!install_script.isEmpty());
    Install(install_script);
}

void GraphicsDriverInterface::Install(QString script)
{
    QProcess *proc = new QProcess(this);
    QPROCESS_DELETE_SELF(proc);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this, [=](int exitCode) {
        if (exitCode){ //失败
            qWarning() << "Driver install failed, ExitCode: " << exitCode;
            Q_EMIT ReportProgress("-1");
        }else{ //成功
            qDebug() << "Driver install success!";
            Q_EMIT ReportProgress("100");
        }
    });

    connect(proc, &QProcess::started, this, [=] {
        qDebug() << "Driver install start!";
        Q_EMIT ReportProgress("0");//开始
    });


    connect(proc, &QProcess::readyReadStandardOutput, this, [=]() {
        QString out = proc->readAllStandardOutput();
        qDebug() << out;
        QStringList line_list = out.split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
        for(int i=0; i < line_list.size(); i++){
            QString line_str = line_list[i];
            QRegExp rx("^PROGRESS:[ /t/n]?[0-9]{1,3}");
            int pos = line_str.indexOf(rx);
            if (pos < 0) continue;
            else{
                QString ratio = rx.cap(0).split(QRegExp(":[ /t/n]?"))[1];
                qDebug() << "ratio:" <<ratio;
                if (!ratio.isEmpty()) Q_EMIT ReportProgress(ratio);
            }
        }
    });

    connect(this, &GraphicsDriverInterface::Cancel, proc, [=]{
        qDebug()<< "Cancel install";
	    //proc->kill();
	    proc->waitForFinished(-1);
    });

    const QString &cmd = scriptAbsolutePath(script);
    proc->start(cmd);
}


QString GraphicsDriverInterface::GetCurrDriverName()
{
    QString statusScript = m_resolutions.statusScript();
    QString name = m_resolutions.name();
    if (statusScript.isEmpty())
        return nullptr;

    QProcess *proc = new QProcess;
    QPROCESS_DUMP(proc);
    QPROCESS_DELETE_SELF(proc);
    EXECUTE_SCRIPT(proc, statusScript);
    proc->waitForFinished();
    const int status = proc->exitCode();
    if ( "amd" == name ){
        //return m_devInfo.curDriver();
        return m_devInfo.curDevice().m_driver;
    }else if( "intel" == name ){
        switch(status){
            case 0:
                return "glamor";
            case 1:
                return "uxa";
            case 2:
                return "sna";
            default:
                return nullptr;
        }
    }else if("nvidia" == name ){
        switch(status){
            case 0:
                return "nouveau";
            case 1:
                return "nvidia";
            default:
                return nullptr;
        }

    }else if("nvidia_intel" == name){
        switch(status){
            case 0:
                return "glamor";
            case 1:
                return "uxa";
            case 2:
                return "sna";
            case 3:
                return "bumblebee";
            case 4:
                return "prime";
            default:
                break;
        }
    }
    return nullptr;
}

QString GraphicsDriverInterface::GetDriverName(QString type)
{
    if((type !=  "old_driver") && (type !=  "new_driver")){
        qWarning("The dirver type is invalid!");
        return nullptr;
    }
    const QString &config_file = scriptAbsolutePath("working-dir/config.conf");
    if (!QFile(config_file).exists()){
        qWarning("The file %s is not exists!", "working-dir/config.conf");
        return  nullptr;
    }
    
    QSettings *settings = new QSettings(config_file, QSettings::IniFormat);
    settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

    QString driver_name = settings->value(type).toString();

    foreach (const Resolution resl, m_resolutions.resolutions()){
        QJsonObject reslObj;
        if(driver_name == resl.name()){
            reslObj.insert("name", resl.name());
            reslObj.insert("title", resl.title());
            return QString(QJsonDocument(reslObj).toJson(QJsonDocument::Compact));
        }
    }
    return nullptr;
}


QString GraphicsDriverInterface::GetOldDriverName()
{
    return GetDriverName("old_driver");
}

QString GraphicsDriverInterface::GetNewDriverName()
{
    return GetDriverName("new_driver");
}

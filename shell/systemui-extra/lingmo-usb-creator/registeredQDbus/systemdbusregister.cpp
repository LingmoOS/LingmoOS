/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "systemdbusregister.h"
#include <polkitqt1-authority.h>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDebug>
#include <QThread>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDBusConnectionInterface>

using namespace PolkitQt1;

SystemDbusRegister::SystemDbusRegister()
{

}

//excute dd command
void SystemDbusRegister::MakeStart(QString sourcePath,QString targetPath){
    //root authorization
    //   防命令注入
    //  1、形如：mkdir '`id&>id_bak_test.txt`'中的文件夹名称
    if (sourcePath.contains(QRegularExpression(".*`.*`.*"))||targetPath.contains(QRegularExpression(".*`.*`.*")))
        return ;
    // 2、形如：$()的文件夹名称
    if (sourcePath.contains(QRegularExpression(".*\\$\\(.*\\).*"))||targetPath.contains(QRegularExpression(".*\\$\\(.*\\).*")))
        return ;
    // 3、形如：${}的文件夹名称
    if (sourcePath.contains(QRegularExpression(".*\\$\\{.*\\}.*"))||targetPath.contains(QRegularExpression(".*\\$\\{.*\\}.*")))
        return ;
    // 4、包含[;、&、|]等可以包含并执行系统命令或用于连续执行系统命令的符号
    if (sourcePath.contains(QRegularExpression("[;&|]+"))||targetPath.contains(QRegularExpression("[;&|]+")))
        return ;

    sourceFile = sourcePath;
    m_isdeviceexist=false;
    m_getmountpoint=false;
        qDebug()<<__LINE__<<__func__<<m_getmountpoint;
        Authority::Result result;
        SystemBusNameSubject subject(message().service());
        result = Authority::instance()->checkAuthorizationSync("com.lingmousbcreator.systemdbus.authoritycheck",
                                                               subject , Authority::AllowUserInteraction);
        if (result == Authority::Yes){
            //TODO: send authorization failed dbus message
            emit authorityStatus("success");
        }else{
            //there's two cases NO and Challenge
            emit authorityStatus("failed");
            return ;
        }



    env = QProcessEnvironment::systemEnvironment();
    uDiskPath = targetPath;
    QFileInfo info(sourcePath);
    sourceFileSize = info.size()/1000000;
    // if(m_formate=="true"){
    if(umountPath(uDiskPath)){
        qInfo("开始进入格式化");
        bash=new QProcess(this);
        connect(bash,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(mkfs_finished(int,QProcess::ExitStatus)));
        connect(bash, &QProcess::readyReadStandardOutput, this, [=]() {
            mkfsoutput=bash->readAllStandardOutput();
            qDebug()<<__LINE__<<__func__<<mkfsoutput;
        });
        connect(bash, &QProcess::readyReadStandardError, this, [=]() {
            mkfserror=bash->readAllStandardError();
            qDebug()<<__LINE__<<__func__<<mkfserror;
        });
        qInfo("开始进入格式化1");
        QStringList mkfsargs;
        qInfo("开始进入格式化2");
        qInfo()<<__LINE__<<__func__<<uDiskPath;
        mkfsargs<<"-F32"<<"-v"<<"-I"<<uDiskPath.toLocal8Bit();
        qInfo("开始进入格式化3");
        bash->start("/usr/sbin/mkfs.fat",mkfsargs);
        qInfo("开始进入格式化4");
        if (bash->waitForStarted(-1)){
            qInfo("开始进入格式5");
        }else{
            emit makeFinish("make_error");
            qInfo()<<__LINE__<<__func__<<"格式化未启动";
        }
    }else{
        emit makeFinish("make_error");
        qInfo()<<__LINE__<<__func__<<"卸载失败";
    }
    qInfo("开始进入格式化6");
}

void SystemDbusRegister::mkfs_finished(int exitCode, QProcess::ExitStatus exitStatus){
    if(exitStatus==QProcess::NormalExit&& exitCode==0){
        qInfo("格式化结束");
        QThread::sleep(1);
        qInfo("格式化结束1");
        getmountpoint();
        qInfo("格式化结束2");
        if(m_getmountpoint==true){
            command_dd = new QProcess();
            //指定输出语言
            env.insert("LANGUAGE","en_US:en");
            command_dd->setProcessEnvironment(env);
            connect(command_dd,&QProcess::readyReadStandardOutput,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,&QProcess::readyReadStandardError,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(dd_finished(int,QProcess::ExitStatus)));
            m_isFinished = false;
            QTimer::singleShot(1*1000, this, &SystemDbusRegister::handletimeout);
            checkMd5FinishTimer = new QTimer;
            connect(checkMd5FinishTimer, &QTimer::timeout, this, &SystemDbusRegister::checkmd5finish);
            QStringList args;
            args<<"if="+sourceFile.toLocal8Bit()<<"of="+uDiskPath.toLocal8Bit()<<"status=progress"<<"bs=32M";
            command_dd->start("dd", args);
            if(command_dd->waitForStarted()){
                qInfo()<<"97command_dd->arguments()"<<command_dd->arguments();

            }
            qInfo()<<__LINE__<<__func__<<"dd开始";
            m_isStarted = true;
        }else{
            qInfo("格式化结束3");
            mountDevice(uDiskPath);
            qInfo("格式化结束4");
            QThread::sleep(1);
            command_dd = new QProcess();
            //指定输出语言
            env.insert("LANGUAGE","en_US:en");
            command_dd->setProcessEnvironment(env);
            connect(command_dd,&QProcess::readyReadStandardOutput,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,&QProcess::readyReadStandardError,this,&SystemDbusRegister::readBashStandardErrorInfo);
            connect(command_dd,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(dd_finished(int,QProcess::ExitStatus)));
            m_isFinished = false;
            QTimer::singleShot(1*1000, this, &SystemDbusRegister::handletimeout);
            checkMd5FinishTimer = new QTimer;
            connect(checkMd5FinishTimer, &QTimer::timeout, this, &SystemDbusRegister::checkmd5finish);
            QStringList args;
            args<<"if="+sourceFile.toLocal8Bit()<<"of="+uDiskPath.toLocal8Bit()<<"status=progress"<<"bs=32M";
            command_dd->start("dd", args);
            if(command_dd->waitForStarted()){
                qInfo()<<"97command_dd->arguments()"<<command_dd->arguments();

            }
            qInfo()<<__LINE__<<__func__<<"dd开始";
            m_isStarted = true;
        }
    } else{
        qInfo()<<__LINE__<<__func__<<"格式化失败"<<"退出码:"<<exitCode<<bash->errorString();
        m_isFinished = true;
        emit makeFinish("fail");
        qInfo()<<__LINE__<<__func__<<"发送退出";
    }
}
void SystemDbusRegister::dd_finished(int exitCode, QProcess::ExitStatus exitStatus){ 
    if(exitStatus!=QProcess::NormalExit&& exitCode!=0){
        m_isStarted = false;
        m_isFinished = true;
        qInfo("make_error");
        qInfo()<<__func__<<__LINE__<<"dd异常退出"<<command_dd->errorString();
        emit makeFinish("make_error");
    } else if (m_isStarted) {
        m_isStarted = false;
        qInfo()<<__func__<<__LINE__<<"dd正常退出";
        finishEvent();
    } else {
        m_isStarted = false;
        m_isFinished = true;
    }
}

/**
 * @brief 判断程序是否已开启
 * @param processName
 * @return bool
 */
bool SystemDbusRegister::isRunning(const QString &processName)
{
    bool bRet = false;

    QProcess ps;
    QProcess grep;
    ps.setStandardOutputProcess(&grep);

    ps.start("ps", QStringList() << "-ef");
    grep.start("grep", QStringList() << processName);

    if (grep.waitForFinished()) {
        QString result(grep.readAll());
        qDebug() << result;
        QStringList lines = result.split("\n");
        for (const QString& line : lines) {
            if (line.contains("grep"))
                continue;
            if (line.contains(processName)){
                bRet = true;
                break;
            }
        }
    }

    grep.close();
    ps.close();

    return bRet;
}

void SystemDbusRegister::handletimeout(){
    if (m_isFinished)
        return;

    QFile file(sourceFile);
    if (file.exists()) {
        if (m_isStarted) {
            qInfo("SystemDbusRegister::handletimeout : dd exists?");
            // 判断dd是否还在，不在就报错
            if (!isRunning("/usr/bin/dd "))
            {
                qInfo()<<__LINE__<<__func__<<"dd不在运行"<<m_isStarted;
                command_dd->kill();
                // emit makeFinish("make_error");
            }
        }
        QTimer::singleShot(1*1000, this, &SystemDbusRegister::handletimeout);
    } else {
        qInfo("源文件不存在，dd退出");
        command_dd->kill();
        emit makeFinish("make_error");
    }
}

//kill task process
void SystemDbusRegister::MakeExit(){
    //    qDebug()<<"exit dd process";
    //    if(!command_dd) return ;
    //    if(command_dd->Running){
    //        qDebug()<<"dd Runing";
    //        command_dd->kill();
    //        return ;
    //    }
    if(isRunning("/usr/bin/lingmo-usb-creator-sysdbus")){
        qDebug()<<"exit lingmo-usb-creator-sysdbus process";
        bash->kill();
        command_dd->kill();
        sync->kill();
        umount.kill();
        mount.kill();
        md5->kill();
        exit(-1);
        qDebug()<<"exit lingmo-usb-creator-sysdbus process quit";
        return ;
    }
    return ;
}

void SystemDbusRegister::readBashStandardErrorInfo()
{
    QByteArray cmdout = command_dd->readAllStandardError();
    cmdout.trimmed();
    if(!cmdout.isEmpty() && cmdout != "\r" && cmdout != "\n"){
        QString str = cmdout;
        qInfo()<<__LINE__<<__func__<<"dd写入"<<str;
        str = str.replace(" ","");
        //        if(str =="" || str.contains("[sudo]")) {return;}
        str = str.replace("\r","");
        QStringList bytes2 = str.split("bytes");
        QString size_progress = bytes2.first();
        bool ok = false;
        qulonglong progress_num = size_progress.toDouble(&ok)/1048576;
        int mission_percent = progress_num*100/sourceFileSize;
        qInfo()<<"mission_percent"<<mission_percent;

        if(!mission_percent){
            return ;
        }
        QString ii=QString::number(mission_percent);
        if(ii.isEmpty()){
            return;
        }
        //send mission percent debus message every output
        qInfo()<<__func__<<__LINE__<<mission_percent;

        emit workingProgress(mission_percent);
    }else{
        return;
    }
}

void SystemDbusRegister::finishEvent(){
    qInfo()<<__LINE__<<__func__<<"dd结束";
    QTimer *diskRefreshDelay = new QTimer;
    connect(diskRefreshDelay,&QTimer::timeout,[=]{
        diskRefreshDelay->stop();
        isMakingSucess();
        if(m_isdeviceexist==true){
            //send production success dbus message
            qInfo()<<"make success开始同步";
            sync=new QProcess();
            connect(sync,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(syncreadStandardOutput(int,QProcess::ExitStatus)));
            sync->start("sync");
            sync->waitForStarted();
            emit makeFinish("sync");
        }else{
            //send production failure dbus message
            qInfo()<<"多分区U盘"<<"make failed";
            m_isFinished = true;
            emit makeFinish("fail");
        }
    });
    diskRefreshDelay->start(2000);
}
void SystemDbusRegister::syncreadStandardOutput(int exitCode, QProcess::ExitStatus exitStatus){
    QString syncstr;
    QString syncerr;
    if(QProcess::NormalExit==exitStatus&&exitCode==0){
        qInfo()<<"同步正常结束";
        qInfo()<<"sync success!";
        emit makeFinish("unsync");

        qInfo()<<sync->arguments();
        QThread::sleep(1);
        if(umountPath(uDiskPath))
        {
            QThread::sleep(1);
            getmountpoint();
            if(m_getmountpoint==true){
                qDebug()<<__LINE__<<__func__<<"进入自动挂载点校验";
                QThread::sleep(1);
                checkmd5();
            }else{
                QThread::sleep(1);
                if(mountDevice(uDiskPath)){
                    QThread::sleep(1);
                    checkmd5();
                }
            }
        }
        m_isFinished = true;
    }else{
        qWarning()<<"An unknown error occurred! Mount process exit code:"<<sync->exitCode();
        syncstr=sync->errorString();
        syncerr=sync->readAllStandardError();
        qInfo()<<__LINE__<<__func__<<"挂载输出"<<syncerr;
        qInfo()<<__LINE__<<__func__<<"挂载err输出"<<syncstr;
        emit makeFinish("fail");
        emit makeFinish("unsync");
        qInfo()<<"同步失败";
    }

}
bool SystemDbusRegister::mountDevice(QString target){

    QString mountstr;
    QString mounterr;
    qInfo()<<"start mounting disk:"<<target;
    QDir dir;
    mountpath = "/mnt/lingmousbcreator/";
    QDir::setCurrent("/mnt/");
    dir.mkdir("lingmousbcreator");
    QDir::setCurrent(QDir::rootPath());
    QStringList stringlist;
    stringlist<<target.toLocal8Bit()<<mountpath.toLocal8Bit()<<"-v";
    mount.start("mount",stringlist);
    mount.waitForStarted();
    mount.waitForFinished(-1);
    if(QProcess::NormalExit==mount.exitStatus()&& mount.exitCode()==0){
        qInfo()<<mount.arguments();
        qInfo()<<"Mount success!";
        return true;
    }else{
        qWarning()<<"An unknown error occurred! Mount process exit code:"<<mount.exitCode();
        mountstr=mount.errorString();
        mounterr=mount.readAllStandardError();
        qInfo()<<__LINE__<<__func__<<"挂载输出"<<mountstr;
        qInfo()<<__LINE__<<__func__<<"挂载err输出"<<mounterr;
        emit makeFinish("fail");
        qInfo()<<"挂载失败";
        return false;
    }

}

void SystemDbusRegister::isMakingSucess()
{
    qInfo()<<__LINE__<<__func__<<"开始判断U盘是否正常挂载1";
    QList<QStorageInfo> storageInfo = QStorageInfo::mountedVolumes();
    qDebug()<<__LINE__<<__func__<<storageInfo.size();
    for(int i=0;i<storageInfo.size();i++){
        qDebug()<<__LINE__<<__func__<<storageInfo.at(i).device();
        if(storageInfo.at(i).device() == uDiskPath){
            qDebug()<<__LINE__<<__func__<<uDiskPath;
            m_isdeviceexist=true;
            //return true;
        }
    }
}

void  SystemDbusRegister::getmountpoint()
{
    qInfo()<<__LINE__<<__func__<<"getmountpoint";
    QProcess lsblk;
    lsblk.start("lsblk -J");
    lsblk.waitForFinished();

    QProcess lsblk2;
    lsblk2.start("lsblk -JS");
    lsblk2.waitForFinished();
    QJsonArray arr1 = QStringToJsonArray(QString::fromLocal8Bit(lsblk.readAllStandardOutput()));  //获取json类型的shell执行结果
    QJsonArray arr2 = QStringToJsonArray(QString::fromLocal8Bit(lsblk2.readAllStandardOutput()));
    QJsonArray arr3 ;
    QJsonArray mountpointarr4;
    QJsonArray mountpointarr5;
    foreach (const QJsonValue& value, arr1) {
        QJsonObject jsonObj1 = value.toObject();
        foreach (const QJsonValue& value, arr2) {
            QJsonObject jsonObj2 = value.toObject();
            if(jsonObj1["name"] == jsonObj2["name"] && jsonObj2["tran"] == "usb" && jsonObj2["type"] == "disk"){
                mountpointarr4=jsonObj1["mountpoints"].toArray();
                foreach (const QJsonValue& Value, mountpointarr4){
                    if(Value.isNull()){
                        arr3=jsonObj1["children"].toArray();
                        foreach (const QJsonValue& Value, arr3) {
                            qDebug()<<__LINE__<<__func__<<"Page1"<<jsonObj1["name"].toString();
                            QJsonObject jsonObj3 = Value.toObject();
                            qDebug()<<__LINE__<<__func__<<"分区U盘名"<<jsonObj3["name"].toString();
                            if("/dev/"+jsonObj3["name"].toString()==uDiskPath){
                                qDebug()<<__LINE__<<__func__<<"被自动挂载且是分区U盘"<<uDiskPath;
                                mountpointarr5=jsonObj3["mountpoints"].toArray();
                                foreach (const QJsonValue& Value, mountpointarr5) {
                                    if(!Value.isNull()){
                                        if(Value.toString().contains("/media")){
                                            mountpath = Value.toString();
                                        }else{
                                            mountpath=value.toString();
                                        }
                                    }
                                    qDebug()<<__LINE__<<__func__<<"SystemDbusRegister"<<jsonObj3["mountpoints"];
                                }
                                if(mountpath!=NULL){
                                    qInfo()<<__func__<<__LINE__<<mountpath;
                                    m_getmountpoint=true;
                                    qInfo()<<__LINE__<<__func__<<"SystemDbusRegister"<<jsonObj3["mountpoints"].toArray();
                                }
                                qInfo()<<__LINE__<<__func__<<"SystemDbusRegister"<<jsonObj3["mountpoints"].toArray();
                            }
                            qDebug()<<__LINE__<<__func__<<uDiskPath;
                        }
                    }
                    else if(Value.isNull()==false&&uDiskPath=="/dev/"+jsonObj1["name"].toString()){
                        if(Value.toString().contains("/media")){
                            mountpath = Value.toString();
                        }else{
                            mountpath=value.toString();
                        }
                        qDebug()<<__LINE__<<__func__<<"SystemDbusRegister"<<mountpath;
                        m_getmountpoint=true;
                        qInfo()<<__LINE__<<__func__<<"SystemDbusRegister"<<mountpath;
                    }
                    qInfo()<<__LINE__<<__func__<<"SystemDbusRegister-m_getmountpoint"<<m_getmountpoint;
                }
            }
        }
    }
}

QJsonArray SystemDbusRegister::QStringToJsonArray(const QString jsonString){
    QJsonParseError err;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data(),&err);
    if(jsonDocument.isNull())
    {
        qDebug()<< "JsonDocument is NULL.Origin data is:"<< jsonString.toLocal8Bit().data();

    }
    if(err.error != QJsonParseError::NoError){
        qDebug()<<"Parase json"<<jsonString<<" error:"<<err.error;
        //TODO：这里的错误处理后期还可以优化,目前处理错误了就会调用exit()退出程序
        qInfo()<<"U盘被移除";
        //        exit(-1);
    }
    QJsonObject obj = jsonDocument.object();
    return obj["blockdevices"].toArray();
}

void SystemDbusRegister::checkmd5(){
    qInfo("开始检测MD5");
    m_isMd5Finish=false;
    ismakesuccess=false;
    md5=new QProcess();
    connect(md5,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(readStandardOutput(int,QProcess::ExitStatus)));
    QDir::setCurrent(mountpath);
    qDebug()<<__LINE__<<__func__<<QDir::currentPath();
    checkMd5FinishTimer->start(2000);
    md5->start("md5sum",QStringList{"-c","md5sum.txt"});
    connect(md5, &QProcess::readyReadStandardOutput, this, [=]() {
        MD5str=this->md5->readAllStandardOutput();
        qDebug()<<__LINE__<<__func__<<MD5str;
    });
}

void SystemDbusRegister::readStandardOutput(int exitCode,QProcess::ExitStatus exitStatus){
    qDebug()<<"md5退出"<<exitCode;
    if(exitStatus==QProcess::NormalExit && exitCode==0){
        if(MD5str.isEmpty()){
            qDebug()<<__LINE__<<__func__<<"输出为空"<<MD5str;
            QDir::setCurrent(QDir::rootPath());
            qDebug()<<QDir::currentPath();
            ismakesuccess=false;
        }else{
            if (MD5str.contains(tr("Fail")) || MD5str.contains(tr("FAILED")))
            {
                qDebug()<<__LINE__<<__func__<<"有错误字符"<<MD5str;
                qDebug()<<"md5 fail!";
                QDir::setCurrent(QDir::rootPath());
                qDebug()<<__LINE__<<__func__<<QDir::currentPath();
                ismakesuccess=false;
            }
            else{
                qDebug()<<__LINE__<<__func__<<"完全正确"<<MD5str;
                QDir::setCurrent(QDir::rootPath());
                qDebug()<<__LINE__<<__func__<<QDir::currentPath();
                ismakesuccess=true;
            }
        }

        qDebug()<<"md5 success!";
    }else{
        QDir::setCurrent(QDir::rootPath());
        qInfo()<<__func__<<__LINE__<<md5->errorString();
        qDebug()<<__LINE__<<__func__<<QDir::currentPath();
        qDebug()<<"md5 fail!";
    }
    qDebug()<<"md5退出"<<exitCode;
    m_isMd5Finish=true;

}

void SystemDbusRegister::checkmd5finish(){

    if(m_isMd5Finish==true){
        checkMd5FinishTimer->stop();
        if(ismakesuccess==true){
            if(umountPath(mountpath)){
                emit makeFinish("success");
            }
            qDebug()<<"status"<<m_isFinishstatus;
        }else if(ismakesuccess==false){
            if(umountPath(mountpath)){
                emit makeFinish("fail");
            }
            qDebug()<<"ismakesuccess"<<ismakesuccess;
            qDebug()<<"status"<<m_isFinishstatus;
        }
    }
}

bool SystemDbusRegister::umountPath(QString Path){
    QString umountstr;
    QString umounterr;
    qInfo()<<"start umounting disk path:"<<Path;
    umount.start("umount",QStringList()<<Path<<"-v");
    umount.waitForStarted();
    umount.waitForFinished(-1);
    if(QProcess::NormalExit == umount.exitStatus() && umount.exitCode()==0){
        qInfo()<<"umount success!";
        return true;
    }else{
        qWarning()<<"An unknown error occurred! UMount process exit code:"<<umount.exitCode();
        umountstr=umount.errorString();
        umounterr=umount.readAllStandardError();
        qDebug()<<__LINE__<<__func__<<"卸"<<umountstr;
        qDebug()<<__LINE__<<__func__<<"卸1"<<umounterr;
        emit makeFinish("fail");
        qInfo()<<"卸载失败";
        return false;
    }
}

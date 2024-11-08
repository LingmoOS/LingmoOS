/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */


#include "disccommand.h"
#include <lingmo-log4qt.h>

DiscCommand::DiscCommand(QObject *parent) : QObject(parent)
{
    initMembers();
}

DiscCommand::~DiscCommand(){
    if(mProcess){
        mProcess->disconnect(); //断开mProcess所有的信号槽连接
        //delete mProcess;
        mProcess->deleteLater();
    }
}

void DiscCommand::initMembers(){
    mSuccess = false;
    mProcess = new QProcess;
}

/** 命令执行结束槽函数 */
void DiscCommand::cmdFinishSlot(int exitCode, QProcess::ExitStatus exitStatus){
    qInfo()<<__func__<<__LINE__<<mSuccess<<mFinishedInfo;
    if(QProcess::NormalExit != exitStatus)
        qInfo()<<mProcess->errorString();

    if(mSuccess)
        mFinishedInfo.clear();

    Q_EMIT cmdFinished(mFinishedInfo);  //发射信号通知其他地方命令执行完成
}

void DiscCommand::readOutputSlot(){
    QString preOutputContents = mProcess->readAllStandardOutput();
    qInfo()<<preOutputContents;
//    if(preOutputContents.contains("completed successfully")){    //刻录成功
//        mSuccess = true;
//        //mProcess->disconnect(SIGNAL(readyStandardOutput()));     //断开QProcess的信号连接
//    }else if(preOutputContents.contains("cancel its operation")){  //取消了刻录操作
//        mSuccess = false;
//        mFinishedInfo = QObject::tr("burn operation has been cancelled"); //翻译：刻录操作已取消
//        //mProcess->disconnect(SIGNAL(readyStandardOutput()));     //断开QProcess的信号连接
//    }else if(preOutputContents.contains("Device or resource busy")){
//        mSuccess = false;
//        mFinishedInfo = mProcess->arguments().at(1) + QObject::tr("is busy!");//翻译：被占用
//    }
}

void DiscCommand::readErrorSlot(){
    QString preErrorContents = mProcess->readAllStandardError();
    qInfo()<<preErrorContents;
    if(preErrorContents.contains("completed successfully")){    //刻录成功
        mSuccess = true;
        //mProcess->disconnect(SIGNAL(readyStandardOutput()));     //断开QProcess的信号连接
    }else if(preErrorContents.contains("cancel its operation")){  //取消了刻录操作
        mSuccess = false;
        mFinishedInfo = QObject::tr("burn operation has been cancelled"); //翻译：刻录操作已取消
        //mProcess->disconnect(SIGNAL(readyStandardOutput()));     //断开QProcess的信号连接
    }else if(preErrorContents.contains("Device or resource busy")){
        mSuccess = false;
        mFinishedInfo = mProcess->arguments().at(1) + QObject::tr(" is busy!");//翻译：被占用
    }
}

/** 设置命令以及命令参数 */
void DiscCommand::setCmd(const QString& inputCmd, const QStringList& inputCmdParameters){
    mCommand = inputCmd;
    mProcess->setProgram(inputCmd);
    mProcess->setArguments(inputCmdParameters);
}

/** 异步执行linux命令, 命令的输出通过槽函数进行捕获
*/
void DiscCommand::startCmd(){
    mProcess->setReadChannel(QProcess::StandardOutput);
    connect(mProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),this, &DiscCommand::cmdFinishSlot);
    connect(mProcess, &QProcess::readyReadStandardOutput, this, &DiscCommand::readOutputSlot);
    connect(mProcess, &QProcess::readyReadStandardError, this, &DiscCommand::readErrorSlot);
    mProcess->start();
}

/** 阻塞式执行linux命令，命令退出后才返回
*/
bool DiscCommand::startAndWaitCmd(QString& errorInfo){
    mProcess->start();
    /** FIXME: 使用waitForFinished()无法捕获命令在正常执行过程中的报错信息
     *  也即: 无论命令执行过程中是否会报错，该函数永远返回true; 除非存在人为kill操作
    */
    if(!mProcess->waitForFinished(-1)){
        errorInfo = mProcess->errorString();
        return false;
    }
    return true;
}

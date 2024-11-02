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


#ifndef DISCCOMMAND_H
#define DISCCOMMAND_H

#include <QObject>
#include <QProcess>

#include <explor-core_global.h>

#ifdef signals
#undef signals
#endif
class PEONYCORESHARED_EXPORT DiscCommand : public QObject
{
    Q_OBJECT
public:
    explicit DiscCommand(QObject *parent = nullptr);
    void setCmd(const QString&, const QStringList&);
    void startCmd();
    bool startAndWaitCmd(QString&);
    ~DiscCommand();
private:
    void initMembers();

Q_SIGNALS:
    void cmdFinished(QString& finishedInfo);        //命令执行完成的信号(报错信息)

private Q_SLOTS:
    void cmdFinishSlot(int, QProcess::ExitStatus);
    void readOutputSlot();
    void readErrorSlot();
private:
    QProcess *mProcess;
    QString mCommand;
    bool mSuccess;
    QString mFinishedInfo;
};

#endif // DISCCOMMAND_H

/**
  1. linux下目录不可以创建硬链接
  2. 目录创建软连接后，xorriso直接使用软连接后的目录名进行刻录，不报错，但是未刻录进光盘
  3. 硬链接没有方法通过代码实现，只能使用"ln origin dest"命令实现
*/

/**

*/

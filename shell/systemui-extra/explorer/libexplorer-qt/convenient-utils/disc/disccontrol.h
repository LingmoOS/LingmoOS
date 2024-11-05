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


#ifndef DISCREAD_H
#define DISCREAD_H

#include "discscsi.h"
#include "common.h"
#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QProcess>
#include "explorer-core_global.h"

const QString DiscBusName = "org.freedesktop.UDisks2";
const QString DiscBusBlock = "org.freedesktop.UDisks2.Block";
const QString DiscBusDrive = "org.freedesktop.UDisks2.Drive";
const QString DiscBusProperty = "org.freedesktop.DBus.Properties";
const QString DiscBusFilesystem = "org.freedesktop.UDisks2.Filesystem";
const QString DiscBusObjectPathPrefix = "/org/freedesktop/UDisks2/block_devices/";

/* 类简介:
 * First,  检测是否插入光盘
 * Second, 检测是否是空盘
 * Third,  检测是否盘片可能已损坏
 * Fourth，检测是否可追加
 * Fifth,  根据是否可追加具体化光盘类型(如不可追加的DVD-R盘的实际类型为DVD-ROM)
 */
class PEONYCORESHARED_EXPORT DiscControl:public QObject
{
    Q_OBJECT
public:
    DiscControl(QString device, QObject* parent=nullptr);
    ~DiscControl();
    /** 属性接口如下 */
    bool isGood() const;                    //光盘是否损坏
    bool isReady() const;                   //是否有光盘
    bool supportUdf() const;                //光盘是否支持udf格式
    bool discIsEmpty() const;               //是否是空光盘
    QString discLabel() const;              //光盘卷标,如果光盘未设置卷标，则该函数返回QSting("");
    bool discCanAppend() const;             //是否可追加
    QString discMediaType() const;          //光盘介质类型
    QString discFilesystemType() const;     //光盘内部文件系统类型
    QString discDevice() const;             //光驱设备号
    bool isRunningFormat();                 //光盘是否进行格式化操作
    void killFormatProcess();               //杀死不能正常结束的格式化进程
    bool isRemoved();                       //光驱是否被移除了
    void setRemoved(bool);                  //检测到光驱移除后，设置光驱被移除标识

    /** 行为接口如下 */
    void discUnmount();         //异步光盘卸载操作，下方有对应信号
    bool discUnmountSync();     //阻塞卸载光盘操作
    bool discEjectSync();       //阻塞式光盘弹出操作
    /** udf格式化操作 */
    bool formatUdfSync(QString discLabel);//阻塞式Udf格式化操作接口，格式化成功后会立即弹出光盘

    /** 不同格式化工具执行格式化操作，以阻塞方式*/
    bool xorrisoBlankFullSync();  // xorriso阻塞式执行blank操作
    bool xorrisoFormatFullSync();  // xorriso阻塞式执行format操作
    bool formatUdfByUdfclientSync(const QString &);  // 阻塞式执行udf格式化操作

    /** iso9660刻录操作 */
    bool discBurnSync(QString src, QString dest, const QString& discLabel, QString& burnError);
    void discBurn(QString src, QString dest, const QString& discLabel);
    bool work();                //读取光盘状态信息的入口
private:
    bool discBurnSync2();       //内部临时刻录操作
    void workFromBlock();       //敲定光盘卷标以及文件系统类型信息
    void workFromDrive();       //敲定光盘是否为空以及光盘介质类型
    void workForAppend();       //敲定光盘是否可追加数据、光盘是否可擦除
    void workForIsReady();      //检测光盘是否准备就绪(是否已插入光盘)
    int  readDiscProfile();     //使用scsi具体化光盘介质类型
    void workForMediaType();    //使用udisk2具体化光盘介质类型
    bool workForInitMembers();  //初始化成员变量
    void workFromFilesystem();  //检测光盘状态是否良好，是否有损坏
    bool readDiscInformation(uchar** infoData);//调用scsi接口查询光盘信息的入口
    QString prepareFileBeforeBurn(const QString&);

    /** 私有udf格式化接口 */
    bool formatUdfCdRwOrDvdPlusRw(const QString&);   //阻塞式CD-RW/DVD+RW盘udf格式化操作
    bool formatUdfDvdRw(const QString&);             //DVD-RW

Q_SIGNALS:
    void workFinished(DiscControl* pThis);
    void burnFinished(QString);                               //参数为空表明刻录成功
    void unmountFinished(QString);                            //参数为空表明卸载成功
	void formatUdfFinished(bool, QString);					  //bool为false时，QString表示失败原因
private Q_SLOTS:
    void burnSlot(QString);
    void unmountSlot(QDBusPendingCallWatcher*);
    void busPendingCallSlot(QDBusPendingCallWatcher* watcher);//槽函数:查询光盘设备的驱动
private:
    bool mIsGood;               //是否损坏
    bool mIsReady;              //是否插入了光盘
    bool mIsBlank;              //是否是空盘
    bool mIsRemove;             //是否被移除
    uint mProfile;              //scsi光盘介质类型
    bool mCanErase;             //是否可擦除(是否是RW盘)
    bool mCanAppend;            //是否可追加数据

    QString mLabel;             //设备卷标，eg "小明的光盘"
    QString mDevice;            //eg "/dev/sr0"
    QString mMediaType;         //eg DVD+RW CD-RW DVD-R
    QString mFilesystemType;    //eg udf iso9660

    QStringList     mDeleteDirAfterBurn;

    QDBusInterface* mDriveInf;
    QDBusInterface* mBlockInf;
    QDBusInterface* mPropertyInf;
    QDBusInterface* mFilesystemInf;

    QProcess formatUdf;	//UDF格式化进程,目前主要用于<newfs_udf>
};

quint16 from2Byte(const uchar *str);
QString dvdMediaType(const QString& type);

#endif // DISCREAD_H

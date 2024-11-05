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



#include "disccontrol.h"
#include "disccommand.h"
#include <lingmo-log4qt.h>

#include <QDir>
#include <QMap>
#include <QDebug>
#include <QThread>
#include <QProcess>
#include <QVariant>
#include <QFileInfo>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusObjectPath>


DiscControl::DiscControl(QString device, QObject* parent):QObject(parent){
    mDevice = device;
    mIsRemove = false;
    //work();
}

DiscControl::~DiscControl(){
    if((nullptr!=mBlockInf) && mBlockInf->isValid())
        delete mBlockInf;
    if((nullptr!=mDriveInf) && mDriveInf->isValid())
        delete mDriveInf;
    if((nullptr!=mFilesystemInf) && mFilesystemInf->isValid())
        delete mFilesystemInf;

    mDriveInf = mBlockInf = mFilesystemInf = nullptr;
}

bool DiscControl::work(){
    //1. 初始化成员变量、检测光盘是否准备就绪
    if(!workForInitMembers())
        return false;               //光盘不存在或重要成员变量new失败时返回false
    //2. 检测是否是空盘
    //workFromDrive();
    //3. 检测盘片是否可能已损坏
    //workFromFilesystem();
    //4. 检测是否可追加
    //workForAppend();
    //5. 具体化光盘类型
    //workForMediaType();
    //6. 获取光盘文件系统格式和卷标
    //workFromBlock();

    return true;
}

bool DiscControl::workForInitMembers(){
    mIsGood  = true;    //假设所有的盘均未损坏
    mIsBlank = false;   //非空盘
    mIsReady = false;   //初始时光盘未准备就绪(没有插入光盘或未检测到光盘)
    mCanAppend = false; //假设光盘不可追加
    mProfile   = MEDIA_UNKNOWN;
    mDriveInf = mBlockInf = mFilesystemInf = mPropertyInf = nullptr;

    //1. 检测光盘是否已插入
    workForIsReady();
    if(mDevice.isEmpty() || !mIsReady)      //没有设备时不做后续处理
        return false;

//    QString discBusPath;
//    QDBusObjectPath busObjectPath;
    QString busDevicePath = mDevice.mid(5);
    //2. 获取Filesystem dbus对象
    mFilesystemInf = new QDBusInterface(DiscBusName,DiscBusObjectPathPrefix+busDevicePath,
                                        DiscBusFilesystem,QDBusConnection::systemBus());
    if(!mFilesystemInf->isValid())
        qDebug()<<"["<<mDevice<<"] failed to new Filesystem object!";

    //3. 获取Block dbus对象
    mBlockInf = new QDBusInterface(DiscBusName,DiscBusObjectPathPrefix+busDevicePath,
                                   DiscBusBlock,QDBusConnection::systemBus());
    if(!mBlockInf->isValid())
        qDebug()<<"["<<mDevice<<"] failed to new Block object!";

    //4. 根据Property dbus对象
    mPropertyInf = new QDBusInterface(DiscBusName,DiscBusObjectPathPrefix+busDevicePath,
                                      DiscBusProperty,QDBusConnection::systemBus());
    if(!mPropertyInf->isValid()){
        qDebug()<<"["<<mDevice<<"] failed to new Property object!";
        return false;
    }

    //5. 异步获取设备的Drive dbus对象
    QDBusPendingCall propertyCall = mPropertyInf->asyncCall("Get",DiscBusBlock,"Drive");
    QDBusPendingCallWatcher* propertyCallWatcher = new QDBusPendingCallWatcher(propertyCall);
    connect(propertyCallWatcher,&QDBusPendingCallWatcher::finished,this,&DiscControl::busPendingCallSlot);

    return true;
}

/* 异步回调，用于从 "org.freedesktop.UDisks2.Block"中获取"Drive"属性
 * 进而下一步获取"org.freedesktop.UDisks2.Drive"，确定光盘是否为空
 */
void DiscControl::busPendingCallSlot(QDBusPendingCallWatcher* watcher){
    //qDebug()<<".............";
    QDBusPendingReply<QDBusVariant> callReplay = *watcher;
    watcher->deleteLater();
    if(callReplay.isError()){
        qDebug()<<"["<<mDevice<<"] failed query dbus object path!";
        qDebug()<<"["<<mDevice<<"]"<<callReplay.error().message();
        return;
    }

    //获取设备的Drive object path
    QDBusVariant variantReplay = callReplay.argumentAt<0>();
    QDBusObjectPath* busObjectPath =  (QDBusObjectPath*)(variantReplay.variant().data());

    QString discBusPath = busObjectPath->path();
    if(discBusPath.isEmpty()){
        qDebug()<<"["<<mDevice<<"] cannot new Drive object!";
        return;
    }

    //qDebug()<<"["<<mDevice<<"]"<<discBusPath;
    //5. 根据Drive Object Path获取Drive dbus对象
    mDriveInf = new QDBusInterface(DiscBusName,discBusPath,
                                   DiscBusDrive,QDBusConnection::systemBus());

    workFromDrive();
    workFromBlock();
    workFromFilesystem();
    workForAppend();
    Q_EMIT workFinished(this);
    //QString burnErrorMessage;
    //discBurnSync("/home/yan/桌面/qqqq.txt","/","picture",burnErrorMessage);
    //discUnmountSync();
    //discUnmount();
    //discBurn("/home/yan/桌面/qwer.txt", "/");
}

void DiscControl::workForIsReady(){
    if(mDevice.isEmpty())
        return;

    //检测是否插入了光盘
    DiscScsi scsi(mDevice);
    scsi[0] = GPCMD_TEST_UNIT_READY;//0x00
    scsi[5] = 0;
    mIsReady =  scsi.transport();

    qInfo()<<"["<<mDevice<<"]"<<(mIsReady?"is ready!":"is not ready!");
}

/* 从Drive中获取光盘的类型以及是否是空盘
 * 注释：通过scsi读取光盘信息的方式也可以读取isBlank信息，但是获取到的DVD+RW盘结果不准确
 */
void DiscControl::workFromDrive(){
    if (mDevice.isEmpty() || !mIsReady) //没有设备时不做后续处理
        return;

    //qDebug()<<__func__<<__LINE__;
    if((nullptr != mDriveInf) && mDriveInf->isValid()){
        mIsBlank = mDriveInf->property("OpticalBlank").toBool(); //获取"OpticalBlank"属性值
        mMediaType = mDriveInf->property("Media").toString();
        if(mIsBlank)
            mIsGood = true;             //假设所有的空光盘均未损坏

        workForMediaType();
    }else
        qDebug()<<"["<<mDevice<<"] cannot query media type and state!";
}

/* 从Block中获取光盘的卷标与文件系统类型
 */
void DiscControl::workFromBlock(){
    if (mDevice.isEmpty() || !mIsReady) //没有设备时不做后续处理
        return;

    if(mIsBlank)                        //空光盘没有卷标与文件系统，不需要查询
        return;

    if((nullptr==mBlockInf) || !mBlockInf->isValid())
        return;

    mLabel = mBlockInf->property("IdLabel").toString();
    mFilesystemType = mBlockInf->property("IdType").toString();
    //qDebug()<<mFilesystemType.isEmpty()<<(nullptr!=mFilesystemInf)<<(mFilesystemInf->isValid());
}

/* 检测盘盘片可能损坏的逻辑:
 * 光盘没有filesystem(文件系统)的两种状态：空盘和已损坏盘
 * 因此这里我们只检测非空盘是否可能已损坏。
 *
 * 注释：内部存有数据的光盘从udisk2的角度看有两种维度: block(块)和filesystem(文件系统)
 */
void DiscControl::workFromFilesystem(){
    if (mDevice.isEmpty() || !mIsReady)
        return;
    qInfo()<<"["<<mDevice<<"] is"<<(mIsBlank? "empty!":"not empty!");
    if(mIsBlank)
        return;

    /** Filesystem Object 有效但是无法获取到光盘内文件系统的类型(指udf或iso9660)，
     * 此时可能是以下3种情况：
     *   1)linux未正确识别(在自测过程中确实存在光盘未被系统识别的情况，重新插入即可)
     *   2)udf格式光盘加载较慢(自测过程中使用udf盘可以出现该现象；如果使用GVolumeMonitor的信号检测光盘插入，则应该不存在这种可能)
     *   3)光盘可能已损坏
     * 应对手段：建议重新插入光盘
    */
    if(mFilesystemType.isEmpty() && (nullptr !=mFilesystemInf && mFilesystemInf->isValid())){
        mIsGood = false;
        qInfo()<<"["<<mDevice<<"] is not good!";
        qDebug()<<"["<<mDevice<<"] may not be correctly identified or damaged, plaease reinsert it!";
        return;
    }
    qInfo()<<"["<<mDevice<<"] is good!";
    qInfo()<<"["<<mDevice<<"]"<<mLabel;
    qInfo()<<"["<<mDevice<<"]"<<mFilesystemType;
}

void DiscControl::workForAppend(){
    uchar* infoData = nullptr;
    disc_info_t* discInfo = nullptr;

    if (mDevice.isEmpty() || !mIsReady) //没有设备时不做后续处理
        return;

    /** DVD+RW盘从Disc Information中获取到的mCanAppend可能不准确
     *  需要使用profile修正
     *  光盘的状态好坏不影响我们读取profile, 因此把该代码提前
    */
    readDiscProfile();

    if(readDiscInformation(&infoData)){
        discInfo = (disc_info_t*)infoData;
        mCanErase = discInfo->erasable;             //
        qInfo()<<"["<<mDevice<<"]"<<(mCanErase?"can erase!":"can not erase!");
        if(1 == discInfo->status)
            mCanAppend = true;

        free(infoData);
        infoData = nullptr;
    }

    if(mIsBlank || !mIsGood){                       //空盘一般说"首次刻录"或"首次会话"这个概念
        mCanAppend = false;                         //空盘一般不说"是否可追加"这个概念
        qInfo()<<"["<<mDevice<<"]"<<"can not append!";
        return;
    }

    if( (mProfile & MEDIA_DVD_ALL) && (mProfile & (MEDIA_DVD_PLUS_RW | MEDIA_DVD_RW_OVWR)) ){
    //if( (mProfile & MEDIA_DVD_ALL) && (mProfile & MEDIA_DVD_PLUS_RW) ){
        qInfo()<<"["<<mDevice<<"] fix appendable property with profile for DVD+RW/DVD-RW.";
        mCanAppend |= true;
    }

    /* 目前无法处理udf盘的追加刻录功能，所以设定udf不可追加刻录 */
    if(mFilesystemType=="udf")
        mCanAppend = false;
    qInfo()<<"["<<mDevice<<"]"<<(mCanAppend? "can append!":"can not append!");
}

/* 从udisk2获取到光盘介质类型信息后，将其转变为用户可读的表达方式
*/
void DiscControl::workForMediaType(){
    if(mDevice.isEmpty() || !mIsReady || mMediaType.isEmpty())
        return;
    //optical_cd_
    //optical_dvd_r
    //optical_dvd_plus_
    //qDebug()<<__func__<<__LINE__<<mMediaType;
    char ch = mMediaType.at(8).toLatin1();
    //char ch2 = mMediaType.at(12).toLatin1();
    switch(ch){
    case 'c':{   //CD盘
        if(mMediaType == "optical_cd_r"){
            mMediaType = "CD-R";
//            if(!mCanAppend)
//                mMediaType = "CD-ROM";
        }else{
            mMediaType = "CD-RW";
        }
    }
        break;
    case 'd':   //DVD盘
        mMediaType = dvdMediaType(mMediaType);
        break;
    default:
        break;
    }
    qInfo()<<"["<<mDevice<<"]"<<mMediaType;
}

bool DiscControl::readDiscInformation(uchar** infoData){
    uchar* data;
    uint dataLen;
    uchar header[2];
    bool transferRet;

    if(mDevice.isEmpty())
        return false;

    ::memset(header, 0, 2);
    DiscScsi scsi(mDevice);
    scsi[0] = GPCMD_READ_DISC_INFO;//0x51
    scsi[8] = 2;
    scsi[9] = 0;

    dataLen = 0;
    transferRet = scsi.transport(SCSI_READ, header, 2);
    if(transferRet)
        dataLen = from2Byte(header) + 2u;
    else
        qDebug()<<"["<<mDevice<<"] READ DISC INFORMATION length det failed!";

    if(dataLen < 32){
        //qDebug()<<"Device reports bogus disc information length of " <<dataLen<<" for "<<mDevice<<" ...";
        dataLen = 32;
    }

    data          = (uchar*)malloc(dataLen);
    *infoData     = data;
    scsi[7] = dataLen >> 8;
    scsi[8] = dataLen;

    transferRet = scsi.transport(SCSI_READ, data, dataLen);
    if(transferRet){
        quint16 tmp = from2Byte((const uchar*)data) + 2;
        uchar* newData = (uchar*)realloc(data,qMin<uint>(dataLen,tmp));
        if(newData)         //防止失败时内存泄露
            data = newData;
        return true;
    }else{
        qDebug()<<"["<<mDevice<<"] READ DISC INFORMATION with real length "<<dataLen<<" failed!";
        return false;
    }
}

quint16 from2Byte(const uchar *str){
    if(nullptr == str)
        return 0;
    return (str[0]<<8 & 0xFF00) | (str[1] & 0xFF);
}

/* 具体化DVD盘的类型
 */
QString dvdMediaType(const QString& type){
    QString dvdType;

    if(type.isEmpty() || ('d' != type.at(8).toLatin1()))
        return type;

    //DVD-ROM盘不可再追加文件，从udisks2内部读出来该类盘的标识为"optical_dvd"
    if(11==type.length() && (type=="optical_dvd"))
        return QString("DVD-ROM");

    //optical_dvd_r optical_dvd_rw
    //optical_dvd_plus_r optical_dvd_plus_rw
    //qDebug()<<__func__<<__LINE__<<type;
    char ch = type.at(12).toLatin1();

    switch(ch){
    case 'p':{  //DVD+R DVD+RW
        if(type == "optical_dvd_plus_r")
            dvdType = "DVD+R";
        else
            dvdType = "DVD+RW";
    }
        break;
    case 'r':{  //DVD-R DVD-RW
        if(type == "optical_dvd_r")
            dvdType = "DVD-R";
        else
            dvdType = "DVD-RW";
    }
        break;
    default:
        dvdType = type;
        break;
    }
    return dvdType;
}

/* profile可分类CD与DVD, 并严格区分 R RW +-R +-RW */
int DiscControl::readDiscProfile(){
    bool transferRet;
    uchar profileBuffer[8];
    DiscScsi scsi(mDevice);

    ::memset(profileBuffer,0,8);
    scsi[0] = GPCMD_GET_CONFIGURATION;//0x46
    scsi[1] = 1;
    scsi[8] = 8;
    scsi[9] = 0;

    transferRet = scsi.transport(SCSI_READ, profileBuffer, 8);
    if(transferRet){
        short profile = from2Byte((const uchar*)&profileBuffer[6]);
        switch(profile){
        case 0x00: mProfile = MEDIA_NONE;          break;
        case 0x08: mProfile = MEDIA_CD_ROM;        break;
        case 0x09: mProfile = MEDIA_CD_R;          break;
        case 0x0A: mProfile = MEDIA_CD_RW;         break;
        case 0x10: mProfile = MEDIA_DVD_ROM;       break;
        case 0x11: mProfile = MEDIA_DVD_R_SEQ;     break;
        case 0x12: mProfile = MEDIA_DVD_RAM;       break;
        case 0x13: mProfile = MEDIA_DVD_RW_OVWR;   break;
        case 0x14: mProfile = MEDIA_DVD_RW_SEQ;    break;
        case 0x15: mProfile = MEDIA_DVD_R_DL_SEQ;  break;
        case 0x16: mProfile = MEDIA_DVD_R_DL_JUMP; break;
        case 0x1A: mProfile = MEDIA_DVD_PLUS_RW;   break;
        case 0x1B: mProfile = MEDIA_DVD_PLUS_R;    break;
        case 0x2B: mProfile = MEDIA_DVD_PLUS_R_DL; break;
        default: break;
        }
    }else{
        qDebug()<<"[<<"<<mDevice<<"] GET CONFIGURATION failed.";
    }
    return 0;
}

/** 光盘异步卸载操作接口
*/
void DiscControl::discUnmount(){
    QMap<QString,QVariant> tmpArgs;

    if(mDevice.isEmpty())
        return;
    if(!mIsReady || !mIsGood || mIsBlank){
        qInfo()<<"["<<mDevice<<"] does not need to unmount!";
        return;
    }

    tmpArgs.insert(mDevice,"unmount");
    QDBusPendingCall unmountCall = mFilesystemInf->asyncCall("Unmount",tmpArgs);
    QDBusPendingCallWatcher* unmountWatcher = new QDBusPendingCallWatcher(unmountCall);
    connect(unmountWatcher, &QDBusPendingCallWatcher::finished, this, &DiscControl::unmountSlot);
}

/** 异步处理光盘卸载操作的槽函数, 执行完后发射unmountFinished(QString)信号
 *  卸载成功时信号参数is empty
 *  卸载失败时信号参数为报错信息
*/
void DiscControl::unmountSlot(QDBusPendingCallWatcher* watcher){
    QString unmountError;
    QDBusPendingReply<QDBusVariant> unmountReply = *watcher;
    watcher->disconnect();      //断开该对象所有信号与其他对象的槽函数连接
    watcher->deleteLater();

    if(unmountReply.isError())
        unmountError = unmountReply.error().message();

    if(unmountError.contains("Unexpected reply signature"))//报此错时实际上卸载成功
        unmountError.clear();
    else if(unmountError.contains("is not mounted"))       //光盘无需卸载
        unmountError.clear();
    else if(unmountError.contains("target is busy"))
        unmountError = mDevice + tr(" is busy!");           //翻译："被其他程序占用!"
    else if(!unmountError.isEmpty())
        qInfo()<<"["<<mDevice<<"]"<<unmountError;

    Q_EMIT unmountFinished(unmountError);

    /*test: 测试异步光盘刻录流程*/
//    if(!unmountError.isEmpty())                             //卸载成功才可以刻录
//        discBurn("/home/yan/桌面/2.txt", "/", "");
}

/** 阻塞式卸载光盘
 *  卸载成功或无需卸载返回true 卸载失败返回false
 */
bool DiscControl::discUnmountSync(){
    QString unmountSyncError;
    QDBusMessage unmountRetMsg;
    QDBusMessage::MessageType unmountRetMsgType;
    QMap<QString,QVariant> unmountArgs;

    if(mDevice.isEmpty())
        return false;
    if(!mIsReady || !mIsGood || mIsBlank){
        qInfo()<<"["<<mDevice<<"] does not need to unmount!";
        return true;
    }

    unmountArgs.insert(mDevice,"unmount");
    unmountRetMsg = mFilesystemInf->call("Unmount", unmountArgs);
    unmountRetMsgType = unmountRetMsg.type();
    if(QDBusMessage::ErrorMessage == unmountRetMsgType
            || QDBusMessage::InvalidMessage == unmountRetMsgType)
        unmountSyncError = unmountRetMsg.errorMessage();

    if(unmountSyncError.contains("Unexpected reply signature"))//报此错时实际上卸载成功
        unmountSyncError.clear();
    else if(unmountSyncError.contains("is not mounted"))       //光盘无需卸载
        unmountSyncError.clear();
    else if(unmountSyncError.contains("target is busy"))
        unmountSyncError = mDevice + tr("is busy!");           //翻译："被其他程序占用!"
    else if(!unmountSyncError.isEmpty())
        qInfo()<<"["<<mDevice<<"]"<<unmountSyncError;

    return unmountSyncError.isEmpty()? true:false;
}

/** 阻塞式光盘弹出功能
*/
bool DiscControl::discEjectSync(){
    QDBusMessage ejectMsg;
    QString ejectSyncError;
    QMap<QString, QVariant> ejectArgs;
    QDBusMessage::MessageType ejectMsgType;

    if(mDevice.isEmpty())
        return false;
    if(nullptr == mDriveInf || !mDriveInf->isValid())
        return false;

    ejectArgs.insert(mDevice, "eject");
    ejectMsg = mDriveInf->call("Eject", ejectArgs);
    ejectMsgType = ejectMsg.type();
    if(QDBusMessage::ErrorMessage == ejectMsgType ||
            QDBusMessage::InvalidMessage == ejectMsgType)
        ejectSyncError = ejectMsg.errorMessage();

    if(ejectSyncError.contains("is mounted")){
        ejectSyncError.clear();
        ejectSyncError = mDevice + "cannot unmount due to permission.";
    }

    if(!ejectSyncError.isEmpty())
        qDebug()<<"["<<mDevice<<"]"<<ejectSyncError;

    return ejectSyncError.isEmpty()? true:false;
}

/** udf格式化对外统一接口
*/
bool DiscControl::formatUdfSync(QString discLabel){
    bool formatRet;
	QString errInfo;

    //1.目前仅仅支持CD-RW DVD-RW DVD+RW盘的udf格式化操作
    if(!supportUdf()){
        qDebug()<<"["<<mDevice<<"] does not support udf for"<<mMediaType;
		errInfo = mMediaType + tr(" not support udf at present.");
		formatUdfFinished(false, errInfo);
        return false;
    }
    //2. 保证光盘处于卸载状态，卸载失败返回false
    if(!discUnmountSync()){
        qDebug()<<"["<<mDevice<<"] newfs_udf failed because unmount error.";
		errInfo = tr("unmount disc failed before udf format.");
		formatUdfFinished(false, errInfo);
        return false;
    }

    //3. 区分光盘类型，三种光盘有不同的实现细节
    formatRet = false;
    if(mProfile & (MEDIA_CD_RW | MEDIA_DVD_PLUS_RW)){   //CD-RW DVD+RW
        formatRet = formatUdfCdRwOrDvdPlusRw(discLabel);
    }else if(mProfile & MEDIA_DVD_RW_ALL){              //DVD-RW
        formatRet = formatUdfDvdRw(discLabel);
    }

    qDebug()<<"["<<mDevice<<"] udf format"<<(formatRet?"successed.":"failed.");

    //4. 弹出光盘
    if(formatRet){                               //成功则立即弹出，失败后的行为由调用者自己决定
        discEjectSync();                        //如：失败后可重新尝试？或者弹出？
        formatUdfFinished(true, errInfo);		//udf格式化成功
    }

    return formatRet;
}

/** 阻塞式使用xorriso清空光盘
 * @brief DiscControl::xorrisoBlankFullSync
 * @return 清空光盘成功返回true,失败返回false
 */
bool DiscControl::xorrisoBlankFullSync()
{
    QString output;
    QStringList arg;
    QProcess formatUdf;

    arg << "-dev" << mDevice << "-blank" << "full";
    formatUdf.setProcessChannelMode(QProcess::MergedChannels);
    formatUdf.start("/usr/bin/xorriso", arg);
    formatUdf.waitForFinished(-1);
    output = formatUdf.readAll();
    formatUdf.close();

    if(output.contains("xorriso : aborting")){     //xorriso命令失败
        qInfo() << "["<<mDevice<<"] xorriso -blank full failed";
        return false;
    }
    return true;
}

/** 阻塞式使用xorriso命令格式化光盘
 * @brief DiscControl::xorrisoFormatFullSync
 * @return 格式化成功返回true,失败返回false
 */
bool DiscControl::xorrisoFormatFullSync()
{
    QString output;
    QStringList arg;
    QProcess formatUdf; //xorriso -format full在拔出光驱时会自动退出进程，不需主动杀死

    arg << "-dev" << mDevice << "-format" << "full";
    formatUdf.setProcessChannelMode(QProcess::MergedChannels);
    formatUdf.start("/usr/bin/xorriso", arg);
    formatUdf.waitForFinished(-1);
    output = formatUdf.readAll();
    formatUdf.close();
    qDebug() << __LINE__ << output;
    if(output.contains("Failure to open device or file")) {
            qInfo() << "["<<mDevice<<"] xorriso -format full failed: can't open device or file";
            return false;
    } else if(output.contains("xorriso : aborting")){     //xorriso命令失败
        qInfo() << "["<<mDevice<<"] xorriso -format full failed";
        return false;
    }
    return true;
}

/** 阻塞式使用udfclient中的newfs_udf命令对光盘进行udf格式化
 * @brief DiscControl::formatUdfByUdfclientSync
 * @param dvdRwLabel 光盘卷标名
 * @return 格式化成功返回true; 失败返回false
 */
bool DiscControl::formatUdfByUdfclientSync(const QString &dvdRwLabel)
{
    QString output;
    QStringList arg;
    QString formatErr;
    //QProcess formatUdf;

    arg << "-P" << dvdRwLabel << "-L" << dvdRwLabel << mDevice;
    formatUdf.setProcessChannelMode(QProcess::MergedChannels);
    formatUdf.start("/usr/bin/newfs_udf", arg);
    formatUdf.waitForFinished(-1);
    output = formatUdf.readAll();
    formatErr = formatUdf.error();
    formatUdf.close();
    qDebug() << __LINE__ << output;

    if(output.contains("Disc is not properly formatted")){     //newfs_udf命令无法对空盘进行格式化
        qInfo() << "["<<mDevice<<"] preparation failed before udf format.";
        return false;
    } else if(output.contains("No support yet for creating filingsystem on sequential recordables")) {
        qInfo() << "[" <<mDevice << "] No support yet for creating filingsystem on sequential recordables";
        return false;
    } else if(output.contains("Disc is marked being not serial, full, but the last session is not marked closed")) {
        // Disc is marked being not serial, full, but the last session is not marked closed
        // Most likely formatting problem, try formatting it again
        qInfo() << "[" <<mDevice << "] Disc is marked being not serial, full, but the last session is not marked closed";
        return false;
    } else if(output.contains("Can't create filingsystem on a non recordable disc")) {
        qInfo() << "[" <<mDevice << "] Can't create filingsystem on a non recordable disc";
        return false;
    } else if(output.contains("No support yet for non-sequential WORM devices")) {
        qInfo() << "[" <<mDevice << "] No support yet for non-sequential WORM devices";
        return false;
    } else if(output.contains("Disc is empty; please packet-format it before use")) {
        qInfo() << "[" <<mDevice << "] Disc is empty; please packet-format it before use";
        return false;
    } else if(output.contains("Can't handle multiple session rewritable discs yet")) {
        qInfo() << "[" <<mDevice << "] Can't handle multiple session rewritable discs yet";
        return false;
    } else if(output.contains("Input/output error")) {
        qInfo() << "[" <<mDevice << "] Input/output error";
        return false;
    } else if(output.contains(("Failure to open device or file"))) {
        qInfo() << "[" <<mDevice << "] Failure to open device or file";
        return false;
    }

    if(QProcess::FailedToStart == formatErr){
        qInfo() << "[" <<mDevice << "] Can not found newfs_udf tool.";
        return false;
    }

    return true;
}


/** FIXME: 失败时，根据命令输出结果@output1匹配错误信息
 * 阻塞式对CD-RW/DVD+RW盘进行udf格式化
*/
bool DiscControl::formatUdfCdRwOrDvdPlusRw(const QString& udfLabel1)
{
    // 空光盘
    QString formatErrInfo;
    if(mIsBlank) {
        if(!xorrisoFormatFullSync()) {
            qInfo() << __LINE__<< "xorriso -format full: format DVD+RW fail.";
        } else {
            qInfo() << __LINE__<< "xorriso -format full: format DVD+RW success.";
        }
    }
    qInfo() << __LINE__ <<"newfs_udf start exec....";
    if(!formatUdfByUdfclientSync(udfLabel1)) {
        qInfo() << __LINE__ << "newfs_udf: udf format DVD+RW fail.";
        QString errInfo = tr("DVD+RW udf format fail.");
        formatUdfFinished(false, errInfo);
        return false;
    } else {
        qInfo() << "newfs_udf: udf format DVD+RW success.";
        return true;
    }
}

/** DVD-RW空盘的格式化操作比较繁琐，分为两步
 *  1. 先将空盘变为非空盘
 *  2. 对非空盘进行udf格式化
*/
bool DiscControl::formatUdfDvdRw(const QString& dvdRwLabel){
    QString output2;
    QStringList arg2;
    QProcess formatUdf2;

    //1. 准备非空盘
    if(mIsBlank)
        discBurnSync2();

    //2. 进行udf格式化
    arg2<<"-P"<<dvdRwLabel<<"-L"<<dvdRwLabel<<mDevice;
    formatUdf2.setProcessChannelMode(QProcess::MergedChannels);
    formatUdf2.start("/usr/bin/newfs_udf", arg2);
    formatUdf2.waitForFinished(-1);
    output2 = formatUdf2.readAll();
    formatUdf2.close();

    if(output2.contains("Disc is not properly formatted")){     //newfs_udf命令无法对空盘进行格式化
        qDebug()<<"["<<mDevice<<"] preparation failed before DVD-RW udf format.";
        QString errInfo = tr("preparation failed before DVD-RW udf format.");
        formatUdfFinished(false, errInfo);
        return false;
    }

    //3. FIXME: 如果第一步成功，第二步失败了，而且此时光盘未弹出，用户想要重新尝试udf格式化,
    //          那么问题是：需要重新read disc吗？mIsBlank或其他变量需要更新吗？
    return true;
}

/** 阻塞式光盘刻录操作
 *  @srcFile   需要刻录的文件或者文件夹
 *  @destDir   "/"表示刻录到光盘跟目录，"/newburn"表示将@srcFile刻录到光盘的/newburn目录下
 *  @discLabel 本次刻录需要更新的光盘名，""表示采用原来的光盘名
 *  @burnErorr 引用传递，用于传递刻录报错信息
 *  @return    刻录成功返回true
 *  FIXME: QProcess的waitForFinished()无法捕获刻录操作是否成功
*/
bool DiscControl::discBurnSync(QString srcFile, QString destDir, const QString& discLabel,QString& burnError){
    QString burnCmd;                //刻录命令
    bool burnSuccess;               //刻录是否成功
    QString willBurnDir;            //需要刻录的文件夹
    QString burnErrorInfo;          //刻录报错信息
    DiscCommand * burnOperation;
    QStringList burnCmdParameters;  //刻录命令的参数

    //1. 确定需要刻录的目录名
    willBurnDir = prepareFileBeforeBurn(srcFile);
    if(willBurnDir.isEmpty())
        return false;

    //2. 进行光盘的卸载操作
    if(!discUnmountSync())  //光盘卸载失败时无法进行刻录操作
        return false;

    //3. 硬链接创建完成后，对目录进行阻塞式刻录操作
    burnCmd = "xorriso";
    burnCmdParameters << "-dev" << mDevice << "-map" << willBurnDir << destDir;
    if(!discLabel.isEmpty())
        burnCmdParameters << "-volid" << discLabel;
    burnCmdParameters << "-close" << "off" << "-commit" << "-eject";

    burnOperation = new DiscCommand();
    burnOperation->setCmd(burnCmd, burnCmdParameters);
    burnSuccess = burnOperation->startAndWaitCmd(burnErrorInfo);
    burnError = burnErrorInfo;      //保留刻录报错信息
    delete burnOperation;
    if(!burnError.isEmpty())
        qInfo()<<burnError;

    //4. 刻录完成后，删除硬链接文件、目录保留
    if(srcFile!=willBurnDir){       //删除硬链接所在的目录
        QDir deleteDir(willBurnDir);
        bool deleteSuccess = deleteDir.removeRecursively();
    }

    return burnSuccess;
}

/** DVD-RW盘做UDF格式化前的准备操作：将空盘变为非空盘
 *  这里不使用growisofs，因为它刻录完后会弹出光盘，这不是我们所期望的操作
*/
bool DiscControl::discBurnSync2(){
    QDir tmpDir;
    QFile tmpFile;
    QProcess tmpBurn;
    QString tmpFilePath;
    QString tmpDirPath;
    QString tmpBurnOutput;
    QStringList tmpBurnArg;

    //1. 创建临时目录
    tmpDirPath = QDir::homePath() + "/udf-tmp";
    tmpDir.setPath(tmpDirPath);
    if(!tmpDir.mkpath(tmpDirPath))
        return false;

    //2. xorriso不会刻录空的目录，所以创建一个临时文件
    tmpFilePath = tmpDirPath + "/udf-tmp.txt";
    tmpFile.setFileName(tmpFilePath);
    if(!tmpFile.open(QIODevice::ReadWrite))             //如果文件不存在则会创建文件
        return false;
    tmpFile.close();                                    //关闭文件

    //3. 刻录操作：将空盘变为非空盘
    tmpBurnArg<<"-dev"<<mDevice<<"-map"<<tmpDirPath<<"/"<<"-close"<<"off"<<"-commit";
    tmpBurn.start("/usr/bin/xorriso", tmpBurnArg);
    tmpBurn.waitForFinished(-1);
    //tmpBurnOutput = tmpBurn.readAll();

    //4. 刻录结束后删除临时目录
    tmpDir.removeRecursively();                        //删除临时目录以及内部的临时文件
    return true;
}

/** 异步光盘刻录操作
 *  note: 刻录操作执行前需要先进行光盘卸载操作，卸载成功后再进行刻录操作
*/
void DiscControl::discBurn(QString srcFile, QString destDir, const QString& discLabel){
    QString willBurnDir;
    QStringList asyncBurnParameters;
    DiscCommand *asyncBurnOperation;                //TODO: 考虑是否作为成员变量

    //1. 确定需要刻录的目录名
    willBurnDir = prepareFileBeforeBurn(srcFile);
    if(willBurnDir.isEmpty())
        return;

    //2. 异步执行刻录操作，通过burnFinished信号参数判断是否刻录成功
    asyncBurnOperation = new DiscCommand;
    asyncBurnParameters << "-dev" << mDevice << "-map" << willBurnDir << destDir;
    if(!discLabel.isEmpty())
        asyncBurnParameters << "-volid" << discLabel;
    asyncBurnParameters << "-close" << "off" << "-commit" << "-eject";

    mDeleteDirAfterBurn << srcFile << willBurnDir;
    asyncBurnOperation->setCmd("/usr/bin/xorriso", asyncBurnParameters);
    connect(asyncBurnOperation, &DiscCommand::cmdFinished, this, &DiscControl::burnSlot);
    asyncBurnOperation->startCmd();

    //TODO: asyncBurnOperation->deleteLater();
}

/** 光盘刻录结束的槽函数
*/
void DiscControl::burnSlot(QString burnRetInfo){
    //删除刻录前创建的硬连接
    if(mDeleteDirAfterBurn.count() ==2 ){
        QString needDeleteDir = mDeleteDirAfterBurn.at(1);
        if(mDeleteDirAfterBurn.at(0) != needDeleteDir){
            QDir deleteDir(needDeleteDir);
            deleteDir.removeRecursively();
        }
        mDeleteDirAfterBurn.clear();
    }
    qInfo()<<__func__<<__LINE__<<burnRetInfo;
    //通知其他地方刻录操作已经结束
    Q_EMIT burnFinished(burnRetInfo);
}

/** 刻录文件前的准备操作：
 *  @srcFile是文件或者软连接，则在指定目录下创建硬链接并返回
 *  @srcFile是目录则直接返回目录
 *  note: @srcFile必须要是绝对路径
 *  @return 返回硬链接的父目录 或者直接返回 传入的目录
*/
QString DiscControl::prepareFileBeforeBurn(const QString& srcFile){
    QFileInfo srcFileAttr;
    QString realSrcFile;
    QString needBurnDir;
    if(srcFile.isEmpty())
        return QString();

    srcFileAttr.setFile(srcFile);
    if(!srcFileAttr.exists())        //文件路径错误或文件不存在，不予刻录
        return QString();

    if(srcFileAttr.isSymLink()){     //如果是软连接，先获取到原文件/原目录
        realSrcFile = srcFileAttr.symLinkTarget();
        srcFileAttr.setFile(realSrcFile);
    }

    //普通文件需要创建硬链接后再对目录进行刻录操作
    if(srcFileAttr.isFile()){
        QStringList hardLinkArgs;
        QString hardLinkFileName;
        QString hardLinkParentDir;
        QDir dirObject;
        //创建同名硬链接到指定目录下
        QProcess *createHardlink = new QProcess();
        hardLinkFileName = srcFileAttr.fileName();          //同名硬链接
        hardLinkParentDir = QDir::homePath()+"/.cache/LingmoBurner/";
    //hardLinkParentDir = srcFileAttr.canonicalPath(() + ".cache/LingmoBurner";//jxyh项目修改硬链接路径
        //先确保指定目录存在,不存在则递归创建
        dirObject.setPath(hardLinkParentDir);
        if(!dirObject.exists()){
            if(!dirObject.mkpath(hardLinkParentDir)){
                qInfo()<<"failed for mkdir:" + hardLinkParentDir;
                return QString();
            }
        }
        //阻塞式创建文件的硬链接
        hardLinkFileName = hardLinkParentDir+hardLinkFileName;
        createHardlink->setProgram("/usr/bin/ln");
        hardLinkArgs.append(srcFileAttr.absoluteFilePath());//ln第一个参数
        hardLinkArgs.append(hardLinkFileName);              //ln第二个参数
        createHardlink->setArguments(hardLinkArgs);
        createHardlink->start();
        if(!createHardlink->waitForFinished(-1)){           //阻塞等待ln命令执行
            qInfo()<<createHardlink->errorString();
            createHardlink->deleteLater();
            return QString();
        }
        createHardlink->deleteLater();
        srcFileAttr.setFile(hardLinkFileName);
        if(!srcFileAttr.exists()){                          //硬链接文件未找到
            qInfo()<<"create hardlink failed for"<<srcFile;
            return QString();
        }

        return hardLinkFileName.mid(0,hardLinkFileName.lastIndexOf("/"));
    }else{//目录可以直接刻录
        needBurnDir = srcFileAttr.absoluteFilePath();
        return needBurnDir;
    }

    return QString();
}

bool DiscControl::discIsEmpty() const{
    return mIsBlank;
}

bool DiscControl::isReady() const{
    return mIsReady;
}

bool DiscControl::isGood() const{
    return mIsGood;
}

bool DiscControl::discCanAppend() const{
    return mCanAppend;
}

QString DiscControl::discLabel() const{
    return mLabel;
}

QString DiscControl::discMediaType() const{
    return mMediaType;
}

QString DiscControl::discFilesystemType() const{
    return mFilesystemType;
}

QString DiscControl::discDevice() const
{
    return mDevice;
}

bool DiscControl::isRunningFormat()
{
    qDebug() << "formatUdf.state() = " << formatUdf.state();
    if ((formatUdf.state() != QProcess::NotRunning) ){
        qDebug() << __LINE__ << "format process is still running";
        return true;
    } else
        return false;
}

void DiscControl::killFormatProcess()
{
    qDebug() << __LINE__ << "isRunningFormat() ==" << isRunningFormat();
    if (isRunningFormat()){
        if(mProfile & (MEDIA_CD_RW | MEDIA_DVD_PLUS_RW | MEDIA_DVD_RW_ALL) ){   //CD-RW DVD+RW
            qDebug() << __FILE__ << "cd-rw or dvd+/-rw process of formatting UDFs will be killed";
            formatUdf.kill();
        }
    }

}

bool DiscControl::isRemoved()
{
    return mIsRemove;
}

void DiscControl::setRemoved(bool value)
{
    qDebug() << "mIsRemove = " << mIsRemove;
    mIsRemove = value;
}


/** 目前仅CD-RW DVD-RW DVD+RW支持udf
*/
bool DiscControl::supportUdf() const{
    //if(mProfile & (MEDIA_CD_RW|MEDIA_DVD_RW_ALL|MEDIA_DVD_PLUS_RW))

    // 判断当前系统中是否集成有udfclient包，如果没有则不支持DVD+RW格式化为udf格式
    bool isExistUDFClient = false;
    QFileInfo binfile;
    binfile.setFile("/bin/newfs_udf");
    if(!isExistUDFClient && binfile.exists() && binfile.isExecutable()) {
        isExistUDFClient = true;
    }
    binfile.setFile("/usr/bin/newfs_udf");
    if(!isExistUDFClient && binfile.exists() && binfile.isExecutable()) {
        isExistUDFClient = true;
    }
    if (!isExistUDFClient) {
        return false;
    }

    if(mProfile & MEDIA_DVD_PLUS_RW) {					//2209仅提供DVD+RW的udf格式化
        return true;
    }

    return false;
}

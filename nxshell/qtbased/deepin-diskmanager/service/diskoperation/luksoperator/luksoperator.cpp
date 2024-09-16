// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "luksoperator.h"
#include "../fsinfo.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTextStream>
#include <QDebug>

DeviceInfoMap *LUKSOperator::m_dev = nullptr;
LVMInfo *LUKSOperator::m_lvmInfo = nullptr;
CRYPTError LUKSOperator::m_cryErr = CRYPTError::CRYPT_ERR_NORMAL;
static const QString saveKeyPath = "/root/.deepin-diskmanager-service";     //key文件保存位置
/***********************************************public****************************************************************/

LUKSOperator::LUKSOperator()
{

}

bool LUKSOperator::updateLUKSInfo(DeviceInfoMap &dev, LVMInfo &lvmInfo, LUKSMap &luks)
{
    m_dev = &dev;
    m_lvmInfo = &lvmInfo;
    resetLuksMap(luks);
    //初始化映射 算法
    if (!initMapper(luks) || !getCIPHERSupport(luks.m_cryptSuuport)) {
        return setLUKSErr(luks, CRYPTError::CRYPT_ERR_INIT_FAILED);
    }

    //磁盘luks信息
    for (DeviceInfoMap::iterator devIt = dev.begin(); devIt != dev.end(); ++devIt) {
        devIt.value().m_crySupport = luks.m_cryptSuuport;
        if (!lvmInfo.lvInfoExists(devIt.key()) && isLUKS(devIt.key())) { //判断是否为lv加密的映射盘
            devIt->m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
            LUKS_INFO info;
            if (!getLUKSInfo(luks, devIt.key(), info)) {
                return setLUKSErr(luks, CRYPTError::CRYPT_ERR_GET_LUKSINFO_FAILED);
            }
            luks.m_luksMap.insert(info.m_devicePath, info);
            continue;
        }
        //分区luks信息
        for (PartitionVec::iterator partIt = devIt.value().m_partition.begin(); partIt != devIt.value().m_partition.end(); ++partIt) {
            if (partIt->m_fileSystemType != FSType::FS_LUKS) { //分区通过文件系统判断是否为luks
                continue;
            }
            partIt->m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;//获取分区数据
            devIt->m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
            LUKS_INFO info;
            if (!getLUKSInfo(luks, partIt->m_path, info)) {
                return setLUKSErr(luks, CRYPTError::CRYPT_ERR_GET_LUKSINFO_FAILED);
            }
            luks.m_luksMap.insert(info.m_devicePath, info);
        }
    }
    //lv luks信息
    for (QMap<QString, VGInfo>::iterator vgIt = lvmInfo.m_vgInfo.begin(); vgIt != lvmInfo.m_vgInfo.end(); ++vgIt) {
        for (QVector<LVInfo>::iterator lvIt = vgIt.value().m_lvlist.begin(); lvIt != vgIt.value().m_lvlist.end(); ++lvIt) {
            if (!isLUKS(lvIt->m_lvPath)) {
                continue;
            }
            lvIt->m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
            lvIt->m_lvFsType = FSType::FS_LUKS;
            lvIt->m_fsUsed = lvIt->m_LESize * lvIt->m_lvLECount;
            vgIt.value().m_luksFlag = LUKSFlag::IS_CRYPT_LUKS;
            LUKS_INFO info;
            if (!getLUKSInfo(luks, lvIt->toMapperPath(), info)) {
                return setLUKSErr(luks, CRYPTError::CRYPT_ERR_GET_LUKSINFO_FAILED);
            }

            info.m_devicePath = lvIt->m_lvPath;
            info.m_mapper.m_devicePath = lvIt->m_lvPath;
            luks.m_luksMap.insert(info.m_devicePath, info);
        }
    }

    return setLUKSErr(luks, CRYPTError::CRYPT_ERR_NORMAL);
}

bool LUKSOperator::encrypt(LUKSMap &luks, LUKS_INFO &luksInfo)
{
    //判断参数是否正确
    if (!(luksInfo.m_crypt == CRYPT_CIPHER::AES_XTS_PLAIN64 || luksInfo.m_crypt == CRYPT_CIPHER::SM4_XTS_PLAIN64)
            || luksInfo.m_devicePath.isEmpty()
            || luksInfo.m_decryptStr.isEmpty()) {
        return setLUKSErr(luks, CRYPTError::CRYPT_ERR_ENCRYPT_ARGUMENT);
    }
    //加密
    if (!format(luksInfo)) {
        qCritical() << Q_FUNC_INFO << "format failed";
        return setLUKSErr(luks, CRYPTError::CRYPT_ERR_ENCRYPT_FAILED);
    }
    //添加错误记录
    updateDecryptToken(luksInfo, true);
    //添加密钥提示
    addToken(luksInfo, luksInfo.m_tokenList);
    bool success = getLUKSInfo(luks, luksInfo.m_devicePath, luksInfo);
    luksInfo.isDecrypt = false;
    return setLUKSErr(luks, luksInfo, success ? CRYPTError::CRYPT_ERR_NORMAL : CRYPTError::CRYPT_ERR_GET_LUKSINFO_FAILED);
}

bool LUKSOperator::decrypt(LUKSMap &luks, LUKS_INFO &luksInfo)
{
    //判断参数是否正确
    if (luksInfo.m_mapper.m_dmName.isEmpty()
            || luksInfo.m_devicePath.isEmpty()
            || luksInfo.m_decryptStr.isEmpty()) {
        return setLUKSErr(luks, CRYPTError::CRYPT_ERR_ENCRYPT_ARGUMENT);
    }

    if (!luksInfo.isDecrypt) {
        luksInfo.m_decryptErrCount = open(luksInfo) ? 0 : luksInfo.m_decryptErrCount + 1;
    } else {
        luksInfo.m_decryptErrCount = testKey(luksInfo) ? 0 : luksInfo.m_decryptErrCount + 1;
    }

    //更新解密记录
    updateDecryptToken(luksInfo, false);
    getLUKSInfo(luks, luksInfo.m_devicePath, luksInfo);
    luksInfo.isDecrypt = (0 == luksInfo.m_decryptErrCount);
    return setLUKSErr(luks, luksInfo, luksInfo.isDecrypt ? CRYPTError::CRYPT_ERR_NORMAL : CRYPTError::CRYPT_ERR_DECRYPT_FAILED);
}

bool LUKSOperator::closeMapper(LUKSMap &luks, LUKS_INFO &luksInfo)
{
    //如果没有打开 返回正确
    if (!luksInfo.isDecrypt) {
        return setLUKSErr(luks, luksInfo, CRYPTError::CRYPT_ERR_NORMAL);
    }
    //判断参数是否正确
    if (luksInfo.m_mapper.m_dmPath.isEmpty()) {
        return setLUKSErr(luks, luksInfo, CRYPTError::CRYPT_ERR_ENCRYPT_ARGUMENT);
    }

    return setLUKSErr(luks, luksInfo, close(luksInfo) ? CRYPTError::CRYPT_ERR_NORMAL : CRYPTError::CRYPT_ERR_DECRYPT_FAILED);
}

bool LUKSOperator::addKeyAndCrypttab(LUKSMap &luks, LUKS_INFO &luksInfo)
{
    if (!addKeyFile(luksInfo)) {
        return setLUKSErr(luks, luksInfo, CRYPTError::CRYPT_ERR_ADD_KEY_FAILED);
    }

    bool success = wirteCrypttab(luksInfo, true);
    return setLUKSErr(luks, luksInfo, success ? CRYPTError::CRYPT_ERR_NORMAL : CRYPTError::CRYPT_ERR_CRYPTTAB_FAILED);
}

bool LUKSOperator::deleteKeyAndCrypttab(LUKSMap &luks, LUKS_INFO &luksInfo)
{
    if (!deleteKeyFile(luksInfo)) {
        return setLUKSErr(luks, luksInfo, CRYPTError::CRYPT_ERR_DEL_KEY_FAILED);
    }

    bool success = wirteCrypttab(luksInfo, false);
    return setLUKSErr(luks, luksInfo, success ? CRYPTError::CRYPT_ERR_NORMAL : CRYPTError::CRYPT_ERR_CRYPTTAB_FAILED);
}

bool LUKSOperator::removeMapperAndKey(LUKSMap &luks, LUKS_INFO &luksInfo)
{
    return deleteKeyAndCrypttab(luks, luksInfo) && closeMapper(luks, luksInfo);
}

void LUKSOperator::resetLuksMap(LUKSMap &luks)
{
    luks.m_mapper.clear();
    luks.m_luksMap.clear();
    luks.m_cryErr = CRYPTError::CRYPT_ERR_NORMAL;
    luks.m_cryptSuuport.aes_xts_plain64 = CRYPT_CIPHER_Support::NOT_SUPPORT;
    luks.m_cryptSuuport.sm4_xts_plain64 = CRYPT_CIPHER_Support::NOT_SUPPORT;
}

/***********************************************private****************************************************************/



bool LUKSOperator::initMapper(LUKSMap &luks)
{
    luks.m_mapper.clear();
    QString cmd, strout, strerr;
    cmd = QString("dmsetup ls --target crypt"); //获取所有解密的设备
    if (Utils::executCmd(cmd, strout, strerr) != 0) {
        return false;
    }

    foreach (QString str, strout.split("\n")) {
        QStringList list = str.split("\t");
        if (list.count() == 2) {
            LUKS_MapperInfo mapper;
            mapper.m_dmName = list[0];
            mapper.m_dmPath = "/dev/mapper/" + mapper.m_dmName;

            QString cmd2, strout2, strerr2;
            cmd2 = QString(" cryptsetup status %1").arg(mapper.m_dmPath);
            if (Utils::executCmd(cmd2, strout2, strerr2) != 0) {
                return false;
            }

            foreach (QString str2, strout2.split("\n")) {
                QStringList list2 = str2.split(":");
                if (list2.count() == 2) {
                    if (list2[0].contains("type")) {
                        mapper.m_luskType = list2[1].trimmed();
                    }

                    if (list2[0].contains("cipher")) {
                        mapper.m_crypt = Utils::getCipher(list2[1].trimmed());
                    }

                    if (list2[0].contains("device")) {
                        mapper.m_devicePath = list2[1].trimmed();
                    }

                    if (list2[0].contains("mode")) {
                        mapper.m_mode = list2[1].trimmed();
                    }
                }
            }

            auto it = m_dev->find(mapper.m_dmPath);
            if (it != m_dev->end()) {
                auto partIt =  it.value().m_partition.begin();
                if (partIt != it.value().m_partition.end()) {
                    PartitionInfo part = *partIt;
                    mapper.m_busy = part.m_busy;
                    mapper.m_mountPoints = part.m_mountPoints;
                    mapper.m_luksFs = static_cast<FSType>(part.m_fileSystemType);
                    mapper.m_fsUsed = part.m_sectorsUsed * it.value().m_sectorSize;
                    mapper.m_fsUnused = part.m_sectorsUnused * it.value().m_sectorSize;
                    mapper.m_Size = Utils::LVMFormatSize(mapper.m_fsUsed + mapper.m_fsUnused);
                    mapper.m_uuid = part.m_uuid;
                    mapper.m_fsLimits = part.m_fsLimits;
                    if (FS_FAT32 == mapper.m_luksFs || FS_FAT16 == mapper.m_luksFs) {
                        mapper.m_fsLimits = FS_Limits(-1, -1); //fat格式不支持逻辑卷的扩展缩小
                    } else if (FS_UNALLOCATED ==   mapper.m_luksFs) { //empty fs , no limits
                        mapper.m_fsLimits = FS_Limits(0, 0);
                    }
                }
            }

            bool labelFound = false;
            QString label = DiskManager::FsInfo::getLabel(mapper.m_dmPath, labelFound);
            if (labelFound) {
                mapper.m_fileSystemLabel = label;
            }

            if (m_lvmInfo->pvExists(mapper.m_dmPath)) {
                PVInfo pv = m_lvmInfo->getPV(mapper.m_dmPath);
                mapper.m_vgflag = pv.joinVG() ? LVMFlag::LVM_FLAG_JOIN_VG : LVMFlag::LVM_FLAG_NOT_JOIN_VG;
            }

            luks.m_mapper.insert(mapper.m_devicePath, mapper);
        }
    }

    return true;
}

bool LUKSOperator::getCIPHERSupport(CRYPT_CIPHER_Support &support)
{
    QString cmd, strout, strerr;
    QStringList cipherList = {"aes", "sm4"}; //后续有算法添加 在此处增加算法名称

    cmd = QString("modprobe -c");
    if (Utils::executCmd(cmd, strout, strerr) != 0) {
        return false;
    }

    for (int i = 0; i < cipherList.count(); ++i) {
        int a = 0;
        if (strout.contains(QString("crypto_%1_decrypt").arg(cipherList[i]))) {
            a = CRYPT_CIPHER_Support::Support::DECRYPT;
        }

        if (strout.contains(QString("crypto_%2_encrypt").arg(cipherList[i]))) {
            a |= CRYPT_CIPHER_Support::Support::ENCRYPT;
        }
        if (i == 0) {
            support.aes_xts_plain64 = static_cast<CRYPT_CIPHER_Support::Support>(a);
        }
        if (i == 1) {
            support.sm4_xts_plain64 = static_cast<CRYPT_CIPHER_Support::Support>(a);
        }
    }

    //通过/boot/config-$(uname -r)获取是否支持加密
    cmd = QString("uname -r");
    if (Utils::executCmd(cmd, strout, strerr) != 0) {
        return false;
    }

    QString filePath = QString(QString("/boot/config-%1").arg(strout)).replace("\n", "");
    bool dm = false, aes = false, sm4 = false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();//获取数据
        QString str = line.replace("\n", "");
        if (str == "CONFIG_DM_CRYPT=m" || str == "CONFIG_DM_CRYPT=y") {
            dm = true;
        }

        if (str == "CONFIG_CRYPTO_AES=m" || str == "CONFIG_CRYPTO_AES=y") {
            aes = true;
        }

        if (str == "CONFIG_CRYPTO_SM4=m" || str == "CONFIG_CRYPTO_SM4=y") {
            sm4 = true;
        }
    }
    file.close();
    if (!dm) {
        support.aes_xts_plain64 = CRYPT_CIPHER_Support::NOT_SUPPORT;
        support.sm4_xts_plain64 = CRYPT_CIPHER_Support::NOT_SUPPORT;
    } else {
        if (aes) {
            support.aes_xts_plain64 = CRYPT_CIPHER_Support::CRYPT_ALL;
        }
        if (sm4) {
            support.sm4_xts_plain64 = CRYPT_CIPHER_Support::CRYPT_ALL;
        }

    }
    return true;
}

bool LUKSOperator::getLUKSInfo(const LUKSMap &luks, const QString &devPath, LUKS_INFO &info)
{
    QString cmd, strout, strerr;
    cmd = QString("cryptsetup luksDump %1 --debug-json").arg(devPath);
    if (Utils::executCmd(cmd, strout, strerr) != 0) {
        return false;
    }
    info.m_tokenList.clear();
    info.m_devicePath = devPath;
    //查找mapper数据 如果映射存在
    info.isDecrypt = luks.mapperExists(devPath);
    if (info.isDecrypt) {
        info.m_mapper = luks.getMapper(devPath);
    }
    //解析字符串 获取版本与uuid
    foreach (QString str, strout.split("\n")) {
        if (str.contains("Version:")) {
            info.m_luksVersion = str.replace("Version:", "").toInt();
        }

        if (str.contains("UUID:")) {
            info.m_dmUUID = str.replace("UUID:", "").trimmed();
        }
    }

    bool labelFound = false;
    QString label = DiskManager::FsInfo::getLabel(info.m_devicePath, labelFound);
    if (labelFound) {
        info.m_fileSystemLabel = label;
    }

    //解析json 获取luks数据
    int jsonBegin = strout.indexOf("# {") + 2;
    int jsonEnd = strout.indexOf("}\nLUKS header information") + 1; //截取json字符串
    // cryptsetup 2.4.1 返回："}LUKS header information"
    if (jsonEnd <= 1) {
        jsonEnd = strout.indexOf("}LUKS header information") + 1;
    }
    QString jsonStr = strout.mid(jsonBegin, jsonEnd - jsonBegin);

    return jsonToLUKSInfo(jsonStr, info);
}

bool LUKSOperator::jsonToLUKSInfo(QString jsonStr, LUKS_INFO &info)
{
    //获取对象填入value中  同时判断该value是否为object
    auto getObjectValue = [ = ](QJsonObject obj, QString key, QJsonValue & value)->bool{
        if (!obj.contains(key))
        {
            return false;
        }
        value = obj.take(key);
        return value.isObject();
    };

    //判断json字符串是否可以解析
    QJsonParseError jsonErr;
    QJsonDocument json = QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError || !json.isObject()) {
        return false;
    }
    //判断对象是否存在
    QJsonObject obj = json.object();
    QJsonValue tokenValue;
    QJsonValue keyslotsValue;
    QJsonValue segValue;
    if (!(getObjectValue(obj, "tokens", tokenValue)
            && getObjectValue(obj, "keyslots", keyslotsValue)
            && getObjectValue(obj, "segments", segValue))) {
        return false;
    }

    //获取数据 tokens keyslots segments 目前token-id 0 作为记录解密失败次数用 token-id 1之后为密码提示信息
    //获取密码提示可以为空
    if (!tokenValue.isNull()) {
        QJsonObject tobj = tokenValue.toObject();
        for (auto iter = tobj.begin(); iter != tobj.end(); ++iter) {
            if (!iter.value().isObject()) {
                return false;
            }
            QJsonObject tobj2 = iter.value().toObject();
            if (!tobj2.isEmpty()) {
                QJsonValue val = tobj2.take("key_description");
                if (!val.isString()) {
                    return false;
                }
                QString tokenStr  = val.toString();
                tokenStr = tokenStr.mid(1, tokenStr.size() - 2);//去掉首尾字符串的''
                //decryptCount:####decryptErrTimer:   解密错误记录
                if (tokenStr.contains("decryptCount:") && tokenStr.contains("decryptErrTimer:")) {
                    QStringList list1 = tokenStr.split("####");
                    if (list1.size() == 2) {
                        info.m_decryptErrCount = list1[0].mid(QString("decryptCount:").size()).toUInt();
                        info.m_decryptErrorLastTime = list1[1].mid(QString("decryptErrTimer:").size()).replace("$$", " ");
                    }
                    continue;
                }
                info.m_tokenList.append(tokenStr.replace("$####$", " ")); //将空格替换回来
            }
        }
    }

    //获取密码使用槽个数  密钥槽不能为空 为空或者不是对象 说明有问题
    if (keyslotsValue.isNull() || !keyslotsValue.isObject()) {
        return false;
    }
    info.m_keySlots = keyslotsValue.toObject().count();


    //获取加密格式
    if (segValue.isNull() || !segValue.isObject()) {
        return false;
    }
    QJsonObject sObj = segValue.toObject();
    if (!sObj.contains("0")) { //目前假设没有数据段0 为异常状态
        return false;
    }
    QJsonValue value2 = sObj.take("0");
    if (!value2.isObject()) {
        return false;
    }
    QJsonObject sObj2 = value2.toObject();
    if (sObj2.contains("encryption")) {
        QJsonValue value3 = sObj2.take("encryption");
        if (!value3.isString()) {
            return false;
        }
        info.m_crypt = Utils::getCipher(value3.toString().trimmed());
    }
    return true;
}

bool LUKSOperator::isLUKS(QString devPath)
{
    QString cmd, strout, strerr;
    cmd = QString("cryptsetup isLuks %1").arg(devPath);
    return  Utils::executCmd(cmd, strout, strerr) == 0;
}

bool LUKSOperator::format(const LUKS_INFO &luks)
{
    QString outPut, error;
    QString cmd = QString("cryptsetup --cipher %1 --key-size 256 --hash sha256 luksFormat --label=%2 %3 -q")
                        .arg(Utils::getCipherStr(luks.m_crypt))
                        .arg(luks.m_fileSystemLabel)
                        .arg(luks.m_devicePath);
    bool success = (Utils::executWithInputOutputCmd(cmd, &luks.m_decryptStr, outPut, error) == 0);
    if (!success) {
        qCritical() << Q_FUNC_INFO << "output:" << outPut << "\terror:" << error;
    }
    return success;
}

bool LUKSOperator::open(const LUKS_INFO &luks)
{
    QString outPut, error;
    QString cmd = QString("cryptsetup open %1 %2 -q")
                        .arg(luks.m_devicePath)
                        .arg(luks.m_mapper.m_dmName);
    return (Utils::executWithInputOutputCmd(cmd, &luks.m_decryptStr, outPut, error) == 0);
}

bool LUKSOperator::testKey(const LUKS_INFO &luks)
{
    QString outPut, error;
    QString cmd = QString("cryptsetup open --test-passphrase  %1 %2 -q")
                        .arg(luks.m_devicePath)
                        .arg(luks.m_mapper.m_dmName);
    return (Utils::executWithInputOutputCmd(cmd, &luks.m_decryptStr, outPut, error) == 0);
}

bool LUKSOperator::close(const LUKS_INFO &luks)
{
    QString cmd, strout, strerr;
    cmd = QString("cryptsetup close %1").arg(luks.m_mapper.m_dmPath);
    return Utils::executCmd(cmd, strout, strerr) == 0;
}

bool LUKSOperator::addToken(const LUKS_INFO &luks, QStringList list, int number)
{
    QString cmd, strout, strerr;
    foreach (QString str, list) {
        if (number == -1) {
            str = str.replace(" ", "$####$");
            cmd = QString("cryptsetup token add --key-description='%1' %2 ")
                  .arg(str)
                  .arg(luks.m_devicePath);
        } else {
            cmd = QString("cryptsetup token add --key-description='%1' --token-id=%2 %3")
                  .arg(str)
                  .arg(number)
                  .arg(luks.m_devicePath);
            number++;
        }

        if (Utils::executCmd(cmd, strout, strerr) != 0) {
            return false;
        }
    }
    return true;
}

bool LUKSOperator::removeToken(const LUKS_INFO &luks, int number)
{
    QString cmd, strout, strerr;
    cmd = QString("cryptsetup token remove --token-id %1 %2 ")
          .arg(number)
          .arg(luks.m_devicePath);
    return Utils::executCmd(cmd, strout, strerr) == 0;
}

bool LUKSOperator::updateDecryptToken(LUKS_INFO &info, bool isFirst)
{
    if (!isFirst) {
        if (!removeToken(info, 0)) {
            return false;
        }
    }
    info.m_decryptErrorLastTime = QDateTime::currentDateTime().toString("yyyy-MM-dd$$hh:mm:ss");
    return addToken(info, QStringList() << QString("decryptCount:%1####decryptErrTimer:%2").arg(info.m_decryptErrCount).arg(info.m_decryptErrorLastTime), 0);

}

bool LUKSOperator::addKeyFile(const LUKS_INFO &luks)
{
    //创建文件
    QDir dir;
    if(!dir.exists(saveKeyPath)){
        if(!dir.mkdir(saveKeyPath)){
            return false;
        }
    }


    QString filePath = QString("%1/%2.key").arg(saveKeyPath).arg(luks.m_dmUUID);
    QString cmd, strout, strerr;
    cmd = QString("dd if=/dev/urandom of=%1 bs=664k count=1").arg(filePath);
    if (Utils::executCmd(cmd, strout, strerr) != 0) {
        return false;
    }

    //添加key
    QString outPut, error;
    cmd = QString("cryptsetup luksAddKey %1 %2 -q")
                .arg(luks.m_devicePath)
                .arg(filePath);
    return (Utils::executWithInputOutputCmd(cmd, &luks.m_decryptStr, outPut, error) == 0);
}

bool LUKSOperator::deleteKeyFile(const LUKS_INFO &luks)
{
    QString filePath = QString("%1/%2.key").arg(saveKeyPath).arg(luks.m_dmUUID);
    if (!QFile::exists(filePath)) {
        return true;
    }

    QString cmd, strout, strerr;
    cmd = QString("cryptsetup luksRemoveKey %1 %2  ").arg(luks.m_devicePath).arg(filePath);
    return  Utils::executCmd(cmd, strout, strerr) == 0 && QFile::remove(filePath);
}

bool LUKSOperator::wirteCrypttab(LUKS_INFO &luksInfo, bool isMount)
{
    QString filePath = QString("%1/%2.key").arg(saveKeyPath).arg(luksInfo.m_dmUUID);
    // open crypttab
    QFile file("/etc/crypttab");
    QStringList list;
    if (!file.open(QIODevice::ReadOnly)) { //打开指定文件
        return false;
    }

    // read crypttab
    bool findflag = false; //目前默认只改第一个发现的uuid findflag 标志位：是否已经查找到uuid
    while (!file.atEnd()) {
        QByteArray line = file.readLine();//获取数据
        QString str = line;
        if (isMount) {
            QString mountStr = QString("%1 UUID=\"%2\" %3\n").arg(luksInfo.m_mapper.m_dmName).arg(luksInfo.m_dmUUID).arg(filePath);
            if (str.contains(luksInfo.m_dmUUID) && !findflag) { //首次查找到uuid
                findflag = true;
                list << mountStr;
                continue;
            } else if (file.atEnd() && !findflag) { //查找到结尾且没有查找到uuid
                list << str << mountStr;
                break;
            }
            list << str;
        } else {
            if (!str.contains(luksInfo.m_dmUUID)) {
                list << str;
            }
        }
    }
    file.close();

    //write crypttab
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        return false;
    }

    QTextStream out(&file);
    for (int i = 0; i < list.count(); i++) {
        out << list.at(i);
    }
    out.flush();
    file.close();
    return true;
}

bool LUKSOperator::setLUKSErr(LUKSMap &luksInfo, const CRYPTError &err)
{
    m_cryErr = err;
    luksInfo.m_cryErr = err;
    return luksInfo.m_cryErr == CRYPTError::CRYPT_ERR_NORMAL;
}

bool LUKSOperator::setLUKSErr(LUKSMap &luksInfo, LUKS_INFO &info, const CRYPTError &err)
{
    info.m_cryptErr = err;
    return setLUKSErr(luksInfo, err);
}

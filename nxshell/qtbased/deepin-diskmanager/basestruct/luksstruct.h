// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef LUKSSTRUCT_H
#define LUKSSTRUCT_H
#include "utils.h"
#include "luksenum.h"
#include "lvmenum.h"

//new by liuwh 2022/4/27
/**
 * @struct CRYPT_CIPHER_Support
 * @brief CRYPT_CIPHER_Support 加密算法支持
 */
struct CRYPT_CIPHER_Support {
public:
    enum Support {
        NOT_SUPPORT = 0x0000,           //not support
        ENCRYPT = 0x0001,               //支持加密
        DECRYPT = 0x0002,               //支持解密
        CRYPT_ALL = ENCRYPT | DECRYPT   //支持加解密
    };

    Support aes_xts_plain64 = NOT_SUPPORT;
    Support sm4_xts_plain64 = NOT_SUPPORT;

    static  bool supportAllcrypt(CRYPT_CIPHER_Support::Support x);
    static  bool notSupportCrypt(CRYPT_CIPHER_Support::Support x);
    static  bool supportEncrypt(CRYPT_CIPHER_Support::Support x);
    static  bool supportDecrypt(CRYPT_CIPHER_Support::Support x);
};
DBUSStructEnd(CRYPT_CIPHER_Support)



//new by liuwh 2022/4/27
/**
 * @class LUKS_MapperInfo
 * @brief LUKS_MapperInfo luks映射盘属性
 */
class LUKS_MapperInfo
{
public:
    FSType m_luksFs{FSType::FS_UNKNOWN};                    //文件系统类型 dm解密后获取
    QVector<QString> m_mountPoints;                         //挂载点
    QString m_uuid;                                         //fstab
    QString m_dmName;                                       //映射名称    sdb1_aesE
    QString m_dmPath;                                       //映射设备路径 /dev/mapper/sdb1_aesE
    bool m_busy{false};                                     //挂载标志
    long long  m_fsUsed{0};                                 //文件已经使用大小 单位byte
    long long  m_fsUnused{0};                               //文件未使用大小 单位byte
    QString m_Size;                                         //字符串类型 展示用
    QString m_devicePath;                                   //原始设备 例如/dev/sdb1 加密映射
    CRYPT_CIPHER m_crypt{CRYPT_CIPHER::NOT_CRYPT};          //加密算法
    QString m_luskType;                                     //LUKS1 LUKS2
    QString m_mode;                                         //read/write
    LVMFlag m_vgflag{LVMFlag::LVM_FLAG_NOT_PV};             //pv设备标识  当该标识为LVM_FLAG_JOIN_VG时，数据从lvminfo中获取
    FS_Limits m_fsLimits;                                   //文件系统限制 该属性在没有文件系统存在时无效
    QString m_fileSystemLabel;                              //卷标名
};
DBUSStructEnd(LUKS_MapperInfo)



//new by liuwh 2022/4/27
/**
 * @class LUKS_INFO
 * @brief LUKS_INFO luks原始盘属性
 */
class LUKS_INFO
{
public:
    LUKS_MapperInfo m_mapper;                               //映射盘属性
    QString m_devicePath;                                   //原始设备 例如/dev/sdb1 加密映射
    CRYPT_CIPHER m_crypt{CRYPT_CIPHER::NOT_CRYPT};          //加密算法
    int m_luksVersion{0};                                   // 1 2

    CRYPTError m_cryptErr{CRYPTError::CRYPT_ERR_NORMAL};    //crypt错误
    QString m_dmUUID;                                       //crypttab  device mapper

    QStringList m_tokenList;                                //密钥提示   luks####提示信息####
    unsigned int m_decryptErrCount{0};                      //解密错误尝试次数  8
    QString m_decryptErrorLastTime;                         //解密错误尝试时间  date
    QString m_decryptStr;                                   //用户解密密码字符串

    bool isDecrypt{false};                                  //是否解密
    int m_keySlots = 0;                                     //当前密钥槽个数

    bool m_Suspend = false;                                 //是否挂起  该属性获取待定
    QString m_fileSystemLabel;                              //卷标名
};
DBUSStructEnd(LUKS_INFO)

typedef QMap<QString, LUKS_INFO> LUKSInfoMap;
Q_DECLARE_METATYPE(LUKSInfoMap)



//new by liuwh 2022/5/17
/**
 * @class OperatorLUKSMap
 * @brief 专为操作luksmap结构体用
 */
class LUKSMap
{
public:
    LUKSMap();

    //判断是否存在  path ：/dev/sda1 or /dev/mapper/sda1_aesE
    bool mapperExists(const QString &path)const;

    //判断dev是否存在  dev:/dev/sda1
    bool deviceExists(const QString &dev)const;

    //判断luks是否存在 dev:/dev/sda1
    bool luksExists(const QString &devPath)const;

    //获取mapper
    LUKS_MapperInfo getMapper(const QString &path)const; //映射盘原始设备 /dev/sda1
    LUKS_MapperInfo getMapper(const LUKS_MapperInfo &mapper)const; //映射盘原始设备 /dev/sda1
    //获取Luks
    LUKS_INFO getLUKS(const QString &path)const; //映射盘原始设备 /dev/sda1

    //通过dev获取mapper路径
    QString getMapperPath(const QString &devPath)const;
    //通过mapper获取dev路径
    QString getDevPath(const QString &mapper)const;
    //判断映射是否属于dev
    bool mapperOfDevice(const QString &mapper, const QString &dev)const;
private:
    template<class T>
    T getItem(const QString &str, const QMap<QString, T> &containers)const;

    template<class T>
    bool itemExists(const QString &str, const QMap<QString, T> &containers)const;
public:
    LUKSInfoMap m_luksMap;                          // key:/dev/sda1  value: LUKS_INFO  Or key:/dev/vg01/lv01  value: LUKS_INFO
    CRYPTError m_cryErr;
    QMap<QString, LUKS_MapperInfo>m_mapper;         // key:/dev/sda1   value: LUKS_MapperInfo  Or  key:/dev/vg01/lv01  value:  LUKS_MapperInfo
    CRYPT_CIPHER_Support m_cryptSuuport;
};
DBUSStructEnd(LUKSMap)


#endif // LUKSSTRUCT_H

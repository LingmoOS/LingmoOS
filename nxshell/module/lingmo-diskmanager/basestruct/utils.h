// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef UTILS_H
#define UTILS_H
#include "commondef.h"
#include "luksenum.h"

#include <QString>
#include <QStringList>
#include <set>

class LVMInfo;
using std::set;


/**
 * @class Utils
 * @brief 文件系统类
 */

class Utils
{
public:
    Utils();

    /**
     * @brief 获取命令路径
     * @param proname：命令名称
     * @return 路径信息
     */
    static QString findProgramInPath(const QString &proName);

    /**
     * @brief 定时执行命令列表
     * @param strCmd：命令
     * @param strArg：命令列表
     * @param outPut：命令输出
     * @param error：错误信息
     * @return 非0失败
     */
    static int executeCmdWithArtList(const QString &strCmd, const QStringList &strArg, QString &outPut, QString &error);

    /**
     * @brief 执行外部命令
     * @param strCmd：命令
     * @param outPut：命令输出
     * @param error：错误信息
     * @return 非0失败
     */
    static int executCmd(const QString &strCmd, QString &outPut, QString &error);

    static int executCmd(const QString &strCmd);

    /**
     * @brief 执行外部命令，并提供输入输出
     * @param strCmd：命令
     * @param strArg：参数
     * @param inPut：命令输入
     * @param outPut：命令输出
     * @param error：错误信息
     * @return 非0失败
     */
    static int executWithInputOutputCmd(const QString &strCmdArg, const QString *inPut, QString &outPut, QString &error);

    /**
     * @brief 执行外部命令
     * @param strCmd：命令
     * @param strArg：参数
     * @param outPut：命令输出
     * @param outPutError: 命令错误输出
     * @param error：错误信息
     * @return 非0失败
     */
    static int executWithErrorCmd(const QString &strCmd, const QStringList &strArg, QString &outPut, QString &outPutError , QString &error);

    /**
     * @brief 正则表达式截取字符串
     * @param strText：原字符串
     * @param strPatter：截取格式
     * @return 需要的字符串
     */
    static QString regexpLabel(const QString &strText, const QString &strPatter);

    /**
     * @brief 获取分区格式字符串
     * @param type：分区格式
     * @return 分区格式字符串
     */
    static const QString getPartitionTypeString(PartitionType type);

    /**
     * @brief 文件系统格式转字符串
     * @param type：分区格式
     * @return 分区格式字符串
     */
    static const QString fileSystemTypeToString(FSType type);

    /**
     * @brief 字符串转文件系统格式
     * @param fileSystemName:文件系统字符串
     * @return 文件系统格式
     */
    static FSType stringToFileSystemType(const QString &fileSystemName);

    /**
     * @brief 获取挂载文件系统已用空间
     * @param mountpoint：挂载点
     * @param fileSystemSize:文件系统大小
     * @param fileSystemFree：空间大小
     * @return 已用空间大小
     */
    static int getMountedFileSystemUsage(const QString &mountpoint, Byte_Value &fileSystemSize, Byte_Value &fileSystemFree);

    /**
     * @brief 获取文件系统软件
     * @param fstype：文件系统格式
     * @return 文件系统软件
     */
    static QString getFileSystemSoftWare(FSType fileSystemType);

    /**
     * @brief 格式化大小
     * @param sectors:扇区
     * @param sectorSize:扇区大小
     * @return 大小字符串
     */
    static QString formatSize(Sector sectors, Byte_Value sectorSize);

    /**
     * @brief 扇区转换函数
     * @param sectors:扇区
     * @param sectorSize:扇区大小
     * @param sizeUnit:转换大小
     * @return 大小字符串
     */
    static double sectorToUnit(Sector sectors, Byte_Value sectorSize, SIZE_UNIT sizeUnit);

    /**
     * @brief 获取最长分区名字长度
     * @param tableType:表类型
     * @return 长度
     */
    static int getMaxPartitionNameLength(QString &tableType);

    /**
     * @brief 计算下偏移量
     * @param value:大小
     * @param roundingSize:舍入值
     * @return 下偏移量
     */
    static Byte_Value floorSize(Byte_Value value, Byte_Value roundingSize);

    /**
     * @brief 计算上偏移量
     * @param value:大小
     * @param roundingSize:舍入值
     * @return 上偏移量
     */
    static Byte_Value ceilSize(Byte_Value value, Byte_Value roundingSize);

    /**
     * @brief 创建UUID
     * @return UUID
     */
    static QString createUuid();


    /**
     * @brief 格式化大小
     * @param lvmSize:lvm 属性大小
     * @return 大小字符串
     */
    static QString LVMFormatSize(long long lvmSize);



    /**
     * @brief lvm转换函数
     * @param lvmSize:lvm 属性大小
     * @param sizeUnit:转换大小
     * @return 大小字符串
     */
    static double LVMSizeToUnit(long long lvmSize, SIZE_UNIT sizeUnit);


    /**
     * @brief 判断pv是否可以删除
     * @param lvmInfo:lvm 属性
     * @param pvStrList:待删除pv列表
     * @param bigDataMove:传入参数 大文件移动标志位
     * @param realMovePvList:传入参数 真实需要移动的pv列表
     * @return true 允许删除 false 不允许删除
     */
    static bool adjudicationPVDelete(LVMInfo lvmInfo, const set<QString> &pvStrList, bool &bigDataMove, QStringList &realMovePvList);

    /**
     * @brief 删除指针
     * @param point:待删除指针
     */
    template<typename T>
    static inline void deletePoint( T *& point){
        if(point !=nullptr){
            delete point;
            point = nullptr;
        }
    }

    /**
     * @brief 根据枚举获取算法字符串
     * @param cipher:算法枚举
     * @return 算法字符串
     */
    static QString getCipherStr(CRYPT_CIPHER cipher);

    /**
     * @brief 根据字符串获取枚举值
     * @param cipher:算法字符串
     * @return 算法枚举
     */
    static CRYPT_CIPHER getCipher(QString cipher);

    /**
    * @brief 判断内核是否支持指定的文件系统
    * @param fsType:文件系统类型名，如ext4
    * @return 支持：true，否则false
    */
    static bool kernelSupportFS(const QString &fsType);

    static QString mkTempDir(const QString &infix);

    static void  rmTempDir(QString &dirName);

    static QString readContent(const QString &filename);
};

#endif // UTILS_H

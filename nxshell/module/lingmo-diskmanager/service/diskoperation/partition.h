// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef PARTITION_H
#define PARTITION_H
#include "commondef.h"
#include "partitioninfo.h"

namespace DiskManager {

/**
 * @class Partition
 * @brief 分区信息类
 */

class Partition
{
public:
    Partition();
    virtual ~Partition() {}

    /**
     * @brief 创建一个新对象
     */
    virtual Partition *clone() const;

    /**
     * @brief 扇区使用率已知
     * @return true成功false失败
     */
    virtual bool sectorUsageKnown() const;

    /**
     * @brief 获得未分配的扇区
     * @return 未分配的扇区
     */
    virtual Sector getSectorsUnallocated() const;

    /**
     * @brief 初始化类
     */
    void reset();

    /**
     * @brief 使用已有的类对象来初始化
     * @param info：已知类对象
     */
    void reset(const PartitionInfo &info);

    //simple Set-functions.  only for convenience, since most members are public
    /**
     * @brief 设置数据
     * @param devicePath：设备路径
     * @param partition：分区信息
     * @param partitionNumber：分区个数
     * @param type：分区类型
     * @param fstype：文件类型格式
     * @param sectorStart：分区扇区开始
     * @param sectorEnd：分区扇区结束
     * @param sectorSize：扇区大小
     * @param insideExtended：扩展分区标志
     * @param busy：挂载标志
     */
    void set(const QString &devicePath, const QString &partition, int partitionNumber, PartitionType type, FSType fstype, Sector sectorStart, Sector sectorEnd, Byte_Value sectorSize, bool insideExtended, bool busy);

    /**
     * @brief 设置未分区
     * @param devicePath：设备路径
     * @param partitionPath：分区路径
     * @param fstype：文件类型格式
     * @param length：扇区长度
     * @param sectorSize：扇区大小
     * @param busy：挂载标志
     */
    void setUnpartitioned(const QString &devicePath, const QString &partitionPath, FSType fstype, Sector length, Byte_Value sectorSize, bool busy);

    /**
     * @brief 设置未分配集合
     * @param devicePath：设备路径
     * @param sectorStart：分区扇区开始
     * @param sectorEnd：分区扇区结束
     * @param sectorSize：扇区大小
     * @param insideExtended：扩展分区标志
     */
    void setUnallocated(const QString &devicePath, Sector sectorStart, Sector sectorEnd, Byte_Value sectorSize, bool insideExtended);

    /**
     * @brief 文件系统标签已知
     * @return true成功false失败
     */
    bool filesystemLabelKnown() const;

    /**
     * @brief 设置文件系统标签
     * @param filesystemLabel：分区标签
     */
    void setFilesystemLabel(const QString &filesystemLabel);

//    /**
//     * @brief 添加挂载点
//     * @param mountpoint：挂载点
//     */
//    void addMountPoint(const QString &mountpoint);

    /**
     * @brief 添加挂载点（复数）
     * @param mountpoints：挂载点（复数）
     */
    void addMountPoints(const QVector<QString> &mountpoints);

    /**
     * @brief 获取挂载点（复数）
     * @return 挂载点（复数）
     */
    QVector<QString> getMountPoints() const;

    /**
     * @brief 获取挂载点
     * @return 挂载点
     */
    QString getMountPoint() const;

    /**
     * @brief 设置扇区使用率
     * @param sectorsFsSize：扇区文件系统大小
     * @param sectorsFsUnused：扇区文件系统未使用
     */
    void setSectorUsage(Sector sectorsFsSize, Sector sectorsFsUnused);

    /**
     * @brief 获取路径
     * @return 路径
     */
    inline QString getPath() const { return this->m_path; }

    /**
     * @brief 设置路径
     * @param path：路径
     */
    inline void setPath(const QString &path) { this->m_path = path; }

    /**
     * @brief 获取扇区大小
     * @return 扇区大小
     */
    inline Sector getSector() const { return (m_sectorStart + m_sectorEnd) / 2; }

    /**
     * @brief 获取字节长度
     * @return 字节长度
     */
    Byte_Value getByteLength() const;

    /**
     * @brief 获取扇区长度
     * @return 扇区长度
     */
    Sector getSectorLength() const;

    /**
     * @brief 获取文件系统标签
     * @return 文件系统标签
     */
    QString getFileSystemLabel() const;

    /**
     * @brief 获取分区信息
     * @return 分区信息
     */
    PartitionInfo getPartitionInfo() const;

    QString m_devicePath;         //设备路径
    int m_partitionNumber;        //分区编号
    PartitionType m_type; // UNALLOCATED, PRIMARY, LOGICAL, etc...
    PartitionStatus m_status; //STAT_REAL, STAT_NEW, etc..
    PartitionAlignment m_alignment; //ALIGN_CYLINDER, ALIGN_STRICT, etc
    FSType m_fstype;              //分区文件格式类型
    QString m_uuid;               //分区uuid
    QString m_name;               //分区别名
    Sector m_sectorStart;        //分区扇区开始
    Sector m_sectorEnd;          //分区扇区结束
    Sector m_sectorsUsed;        //分区已用空间
    Sector m_sectorsUnused;      //分区未用空间
    Sector m_sectorsUnallocated; //Difference between the size of the partition and the file system
    Sector m_significantThreshold; //Threshold from intrinsic to significant unallocated sectors
    bool m_insideExtended;       //扩展标志
    bool m_busy;                  //挂载标志
    bool m_fsReadonly; // Is the file system mounted read-only?
    std::vector<QString> m_flags; //分区属性标志

    //PartitionVector logicals;
    QVector<Partition *> m_logicals; //分区集合
    bool m_strictStart; //Indicator if start sector must stay unchanged
    Sector m_freeSpaceBefore; //Free space preceding partition value

    Byte_Value m_sectorSize; //Sector size of the disk device needed for converting to/from sectors and bytes.
    Byte_Value m_fsBlockSize; // Block size of of the file system, or -1 when unknown.

    FS_Limits m_fsLimits; //分区上文件系统限制 该属性在没有文件系统存在时无效

    LUKSFlag m_luksFlag{LUKSFlag::NOT_CRYPT_LUKS};          //创建分区时 该属性为确认是否创建加密  其余情况为确认该分区是否为加密分区
    CRYPT_CIPHER m_cryptCipher{CRYPT_CIPHER::NOT_CRYPT};          //加密算法(创建分区时该属性有效)
    QStringList m_tokenList;                                //密钥提示   luks####提示信息####(创建分区时该属性有效)
    QString m_decryptStr;                                   //用户解密密码字符串(创建分区时该属性有效)
    QString m_dmName;
private:
    /**
     * @brief 计算重要未分配扇区
     * @return 重要未分配扇区
     */
    Sector calcSignificantUnallocatedSectors() const;
    QString m_path;       //路径
    QVector<QString> m_mountpoints; //挂载点
    bool m_haveFileSystemLabel; //是否有文件系统标签
    QString m_filesystemLabel;   //文件系统标签
};
} // namespace DiskManager
#endif // PARTITION_H

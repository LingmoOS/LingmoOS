// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef LVMSTRUCT_H
#define LVMSTRUCT_H

#include "utils.h"
#include "lvmenum.h"
#include "luksstruct.h"

//new by liuwh 2022/1/17
/**
 * @struct PVDATA
 * @brief pv设备信息数据结构体  创建vg 调整vg使用
 */
typedef struct PVData {
    bool operator<(const PVData &tmp)const;
    bool operator==(const PVData &tmp)const;
    QString m_diskPath;       //磁盘路径
    Sector m_startSector{0};  //开始扇区
    Sector m_endSector{0};    //结束扇区
    int m_sectorSize{0};      //扇区大小
    LVMAction m_pvAct{LVMAction::LVM_ACT_UNkNOW};//执行动作
    DevType m_type{DevType::DEV_UNKNOW_DEVICES};//设备类型
    QString m_devicePath;     //pv路径
} PVData;
DBUSStructEnd(PVData)



//new by liuwh 2022/2/15
/**
 * @struct CreateLVInfo
 * @brief 创建lv结构体 前后端通信用
 */
typedef struct LVAction {
    QString m_vgName;       //vg名称
    QString m_lvName;       //lv名称
    QString m_lvSize;       //lv大小
    long long m_lvByteSize{0}; //lv大小 byte单位
    FSType m_lvFs{FSType::FS_UNKNOWN};//文件系统类型
    QString m_user;         //当前用户名称
    LVMAction m_lvAct{LVMAction::LVM_ACT_UNkNOW};//执行动作
    QString m_mountPoint;   //挂载点
    QString m_mountUuid;    //挂载Uuid

    LUKSFlag m_luksFlag{LUKSFlag::NOT_CRYPT_LUKS};          //创建lv时 该属性为确认是否创建加密  其余情况为确认该分区是否为加密lv
    CRYPT_CIPHER m_crypt{CRYPT_CIPHER::NOT_CRYPT};          //加密算法(创建分区时该属性有效)
    QStringList m_tokenList;                                //密钥提示   luks####提示信息####(创建分区时该属性有效)
    QString m_decryptStr;                                   //用户解密密码字符串(创建分区时该属性有效)
    QString m_dmName;

} LVAction;
DBUSStructEnd(LVAction)



//new by liuwh 2022/1/17
/**
 * @struct LVDATA
 * @brief 逻辑卷信息数据结构体
 */
typedef struct LVDATA {
    QString m_lvName;           //lv名称
    QString m_lvPath;           //lv设备路径
    QString m_lvSize;           //lv大小
    long long m_lvByteSize{0};  //lv大小 byte单位
} LVData;
DBUSStructEnd(LVData)



//new by liuwh 2022/1/17
/**
 * @struct VGDATA
 * @brief 逻辑卷组信息数据结构体
 */
typedef struct VGDATA {
    QString m_vgName;           //vg名称
    QString m_vgSize;           //vg大小 字符串显示
    QString m_vgUuid;           //vg uuid 唯一标识
    long long m_vgByteSize{0};  //vg大小 byte单位
    QVector <LVData>m_lvList;   //lv数据集合
} VGData;
DBUSStructEnd(VGData)



//new by liuwh 2022/1/17
/**
 * @struct PV_RANGES
 * @brief pv设备 使用分布范围
 */
typedef struct PVRANGES {
    QString m_lvName;       //lv名称
    QString m_devPath;      //lv设备路径
    QString m_vgName;       //vg名称
    QString m_vgUuid;       //vg uuid 唯一标识
    long long m_start{0};   //单位 pe
    long long m_end{0};     //单位 pe
    bool m_used{false};     //是否使用  针对vg lv不使用该属性
} PVRanges;
DBUSStructEnd(PVRanges)

typedef PVRanges LV_PV_Ranges;
typedef PVRanges VG_PV_Ranges;



//new by liuwh 2022/1/20
/**
 * @class PVInfo
 * @brief 物理卷信息 属性结构体
 */

/*pvs -o pv_attr
 * Attr
 * a--
1 (d)uplicate复制, (a)llocatable 可分配, (u)sed 使用
2 e(x)ported 导出
3 (m)issing 丢失
*/
class PVInfo
{
public:
    bool isDuplicate()const {return m_pvStatus[0] == 'd';}
    bool isAllocatable()const {return m_pvStatus[0] == 'a';}//已经使用并且分配
    bool isUsed()const {return m_pvStatus[0] == 'u';} //已经加入vg 但是未分配
    bool isExported()const {return m_pvStatus[1] == 'x';}
    bool isMissing()const {return m_pvStatus[2] == 'm';}
    bool noJoinVG()const {return m_pvStatus[0] == "-";}
    bool joinVG()const {return m_pvStatus[0] == "a";}
public:
    QString m_pvFmt;  //pv格式 lvm1 lvm2
    QString m_vgName; //vgName
    QString m_pvPath; //pv路径 /dev/sdb1 ...
    QString m_pvUuid; //pv uuid 唯一名称
    QString m_vgUuid; //vg uuid 唯一名称
    long long m_pvMdaSize{0};//pv metadata size 单位byte
    long long m_pvMdaCount{0};//pv metadata 个数
    QString m_pvSize; //字符串类型 展示用
    QString m_pvFree; //字符串类型 展示用
    long long m_pvUsedPE{0}; //已经使用pe个数
    long long m_pvUnusedPE{0};//未使用pe个数
    int m_PESize{0};  //单个pe大小  单位byte
    QString m_pvStatus{"---"}; //状态
    LVMError m_pvError{LVMError::LVM_ERR_NORMAL};//物理卷错误码
    QMap<QString, QVector<LV_PV_Ranges>> m_lvRangesList; //lv pv分布情况 key：lvPath  value：lv集合
    QVector<VG_PV_Ranges> m_vgRangesList; //vg pv分布情况
    DevType m_lvmDevType{DevType::DEV_UNKNOW_DEVICES};    //lvm 设备类型
    long long m_pvByteTotalSize{0};//pv总大小  单位byte
    long long m_pvByteFreeSize{0};//pv未使用大小 单位byte
};
DBUSStructEnd(PVInfo)



//new by liuwh 2022/1/17
/**
 * @class LVInfo
 * @brief 逻辑卷信息 属性结构体
 */
/*
lvs -o lv_attr
  Attr
  -wi-a-----
  -wi-a-----

 The lv_attr bits are:
       1  Volume type: (C)ache, (m)irrored, (M)irrored without initial sync, (o)rigin, (O)rigin with merging snapshot, (r)aid, (R)aid without initial sync, (s)napshot, merging (S)napshot, (p)vmove, (v)irtual, mirror or  raid  (i)mage,  mirror  or  raid  (I)mage  out-
          of-sync, mirror (l)og device, under (c)onversion, thin (V)olume, (t)hin pool, (T)hin pool data, raid or pool m(e)tadata or pool metadata spare.
       2  Permissions: (w)riteable, (r)ead-only, (R)ead-only activation of non-read-only volume
       3  Allocation policy:  (a)nywhere, (c)ontiguous, (i)nherited, c(l)ing, (n)ormal This is capitalised if the volume is currently locked against allocation changes, for example during pvmove(8).
       4  fixed (m)inor
       5  State:  (a)ctive, (h)istorical, (s)uspended, (I)nvalid snapshot, invalid (S)uspended snapshot, snapshot (m)erge failed, suspended snapshot (M)erge failed, mapped (d)evice present without tables, mapped device present with (i)nactive table, thin-pool (c)heck
          needed, suspended thin-pool (C)heck needed, (X) unknown
       6  device (o)pen, (X) unknown
       7  Target type: (C)ache, (m)irror, (r)aid, (s)napshot, (t)hin, (u)nknown, (v)irtual.  This groups logical volumes related to the same kernel target together.  So, for example, mirror images, mirror logs as well as mirrors themselves appear as (m) if  they  use
          the  original  device-mapper  mirror kernel driver; whereas the raid equivalents using the md raid kernel driver all appear as (r).  Snapshots using the original device-mapper driver appear as (s); whereas snapshots of thin volumes using the new thin provi‐
          sioning driver appear as (t).
       8  Newly-allocated data blocks are overwritten with blocks of (z)eroes before use.
       9  Volume Health, where there are currently three groups of attributes identified:
          Common ones for all Logical Volumes: (p)artial, (X) unknown.
          (p)artial signifies that one or more of the Physical Volumes this Logical Volume uses is missing from the system. (X) unknown signifies the status is unknown.

          Related to RAID Logical Volumes: (r)efresh needed, (m)ismatches exist, (w)ritemostly.
          (r)efresh signifies that one or more of the Physical Volumes this RAID Logical Volume uses had suffered a write error. The write error could be due to a temporary failure of that Physical Volume or an indication that it is failing.  The device should be re‐
          freshed  or  replaced. (m)ismatches signifies that the RAID logical volume has portions of the array that are not coherent.  Inconsistencies are detected by initiating a "check" on a RAID logical volume.  (The scrubbing operations, "check" and "repair", can
          be performed on a RAID logical volume via the 'lvchange' command.)  (w)ritemostly signifies the devices in a RAID 1 logical volume that have been marked write-mostly.  Re(s)haping signifies a RAID Logical Volume is either undergoing  a  stripe  addition/re‐
          moval, a stripe size or RAID algorithm change.  (R)emove after reshape signifies freed striped raid images to be removed.

          Related to Thin pool Logical Volumes: (F)ailed, out of (D)ata space, (M)etadata read only.
          (F)ailed  is set if thin pool encounters serious failures and hence no further I/O is permitted at all. The out of (D)ata space is set if thin pool has run out of data space. (M)etadata read only signifies that thin pool encounters certain types of failures
          but it's still possible to do reads at least, but no metadata changes are allowed.

          Related to Thin Logical Volumes: (F)ailed.
          (F)ailed is set when related thin pool enters Failed state and no further I/O is permitted at all.
       10 s(k)ip activation: this volume is flagged to be skipped during activation.

逻辑卷状态。逻辑卷的属性字节如下：
      1：卷类型： (m)镜像卷，(M) 没有初始同步的镜像卷，(o)原始卷，(O)附带合并快照的原始卷，(r)阵列，(R)没有初始同步的阵列，(s)快照，(S)合并快照，(p)pvmove，(v)虚拟，(i)镜像或阵列映象，(I)未同步的镜像或阵列映象，
        (l)映象日志设备，(c)底层转换卷，(V)精简卷，(t)精简池，(T)精简池数据，(e)阵列或精简池元数据或池元数据备件，
      2：授权：(w)写入，(r)只读，(R)非只读卷的只读激活
      3：分配策略：(a)任意位置，(c)相邻，(i)继承，(l)紧邻，(n)常规。如果该卷目前锁定无法进行分配更改，则该字母会呈大写状态。例如执行 pvmove 命令时。
      4：(m)固定镜像
      5：状态：(a)激活， (s)挂起, (I) 无效快照， (S) 无效挂起快照， (m) 快照合并失败，(M) 挂起快照合并失败，(d) 显示的映射设备不包含表格，(i) 显示的映射设备中包含停用表格。
      6：设备 (o) 开启
      7：目标类型： (m)镜像，(r) RAID，(s) 快照，(t) 精简，(u)未知，(v) 虚拟。这样可将有类似内核目标的逻辑卷分在一组。比如镜像映象、镜像日志以及镜像本身分为组（m），它们使用原始设备映射器内核驱动程序，使用 md raid 内核驱动程序的类似的 raid 设备则分组为（r）。
        使用原始设备映射器驱动程序的快照则分组为（s），使用精简配置驱动程序的精简卷快照则分组为（t）。
      8：使用前，以设置为 0 的块覆盖新分配了数据的块。
      9：卷正常情况：(p) 部分正常，(r) 需要刷新，(m) 存在映射错误，(w) 大部分写入。(p) 部分正常表示该系统中缺少这个逻辑卷使用的一个或多个物理卷。
                   (p) 部分正常表示这个 RAID 逻辑卷使用的一个或多个物理卷有写入错误。该写入错误可能是由于该物理卷故障造成，也可能表示该物理卷正在出现问题。应刷新或替换该设备。
                   (m) 存在映射错误表示 RAID 逻辑卷中有阵列不一致的部分。在 RAID 逻辑卷中启动 check 操作就会发现这些不一致之处。（取消该操作，可使用 lvchange 命令在 RAID 逻辑卷中执行 check 和 repair ）。
                   (w) 大部分写入表示已将 RAID 1 逻辑卷中的设备标记为大部分写入。
      10：(k) 跳过激活：将该卷标记为在激活过程中跳过。
*/
class LVInfo
{
public:
    //由于属性过多 之后补上 目前只需要激活及暂停状态。
    bool isActivve()const {return m_lvStatus[4] == 'a';}
    bool isSuspended()const {return m_lvStatus[4] == 's';}
    bool isPartial()const {return m_lvStatus[8] == 'p';}
    QString toMapperPath(){return QString("/dev/mapper/%1-%2").arg(m_vgName).arg(m_lvName);}
public:
    QString m_vgName; //vg名称
    QString m_lvPath; //lv路径
    QString m_lvUuid; //lv uuid 唯一名称
    QString m_lvName; //lv名称 lv0 lv1 ....
    FSType  m_lvFsType{FSType::FS_UNKNOWN}; //文件系统类型
    QString m_lvSize; //字符串类型 展示用
    long long m_lvLECount{0}; //le个数
    long long  m_fsUsed{0}; //文件已经使用大小 单位byte
    long long  m_fsUnused{0};//文件未使用大小 单位byte
    int m_LESize{0};  //单个pe大小 与所在vg的pe大小相同 单位byte
    bool m_busy{false}; //挂载标志
    QVector<QString> m_mountPoints;//挂载点 可多次挂载
    QString m_lvStatus{"----------"};//lv状态
    LVMError m_lvError{LVMError::LVM_ERR_NORMAL};//逻辑卷错误码
    QString m_mountUuid;//逻辑卷挂载uuid
    FS_Limits m_fsLimits;//文件系统限制 该属性在没有文件系统存在时无效

    LUKSFlag m_luksFlag{LUKSFlag::NOT_CRYPT_LUKS};
    QString m_fileSystemLabel;
    bool m_dataFlag{false};
};
DBUSStructEnd(LVInfo)



//new by liuwh 2022/1/17
/**
 * @class VGInfo
 * @brief 逻辑卷组属性 结构体
 */
/*
vgs -o vg_attr
  Attr
  wz--n-
The vg_attr bits are:
      1  Permissions: (w)riteable, (r)ead-only 权限： 允许写入  只读
      2  Resi(z)eable                          允许调整大小
      3  E(x)ported                            已导出
      4  (p)artial: one or more physical volumes belonging to the volume group are missing from the system  部分：系统中缺少属于卷组的一个或多个物理卷
      5  Allocation policy: (c)ontiguous, c(l)ing, (n)ormal, (a)nywhere 分配政策：连续，紧贴，正常，任何地方
      6  (c)lustered, (s)hared  集群，共享
*/
class VGInfo
{
public:
    bool isWriteable() const {return m_vgStatus[0] == 'w';}
    bool isReadOnly() const {return m_vgStatus[0] == 'r';}
    bool isResizeable() const {return m_vgStatus[1] == 'z';}
    bool isExported() const {return m_vgStatus[2] == 'x';}
    bool isPartial() const {return m_vgStatus[3] == 'p';}
    bool isContiguous() const {return m_vgStatus[4] == 'c';}
    bool isCling() const {return m_vgStatus[4] == 'l';}
    bool isNormal() const {return m_vgStatus[4] == 'n';}
    bool isAnywhere() const {return m_vgStatus[4] == 'a';}
    bool isClustered() const {return m_vgStatus[5] == 'c';}
    bool isShared() const {return m_vgStatus[5] == 's';}

    LVInfo getLVinfo(const QString &lvName);
    bool lvInfoExists(const QString &lvName);
    bool isAllPV(QVector<QString> pvList)const; //判断是否包含了全部pv
public:
    QString m_vgName; //vg名称 vg0, vg1 ....
    QString m_vgUuid; //vg唯一名称 uuid
    QString m_vgSize; //字符串类型 展示用  vg总大小
    QString m_vgUsed; //字符串类型 展示用  vg使用
    QString m_vgUnused; //字符串类型 展示用 vg已用
    long long m_pvCount{0}; //物理卷个数
    long long m_peCount{0}; //pe个数
    long long m_peUsed{0};  //pe使用个数
    long long m_peUnused{0};//pe未使用个数
    int m_PESize{0};  //单个pe大小  单位byte
    int m_curLV{0};//当前lv个数
    QString m_vgStatus{"------"}; //状态
    LVMError m_vgError{LVMError::LVM_ERR_NORMAL};//逻辑卷组错误码
    QVector<LVInfo>m_lvlist; //vg 下lv列表
    QMap<QString, PVInfo> m_pvInfo;
    LUKSFlag m_luksFlag; //luks 标志位
};
DBUSStructEnd(VGInfo)



//new by liuwh 2022/1/17
/**
 * @class LVMInfo
 * @brief lvm属性结构体
 */
class LVMInfo
{
public:
    LVInfo getLVInfo(const QString &lvPath); //  /dev/vg01/lv01   /dev/mapper/vg01-lv01
    LVInfo getLVInfo(const QString &vgName, const QString &lvName);

    VGInfo getVG(const QString &vgName);
    VGInfo getVG(const PVData &pv);
    VGInfo getVG(const PVInfo &pv);

    PVInfo getPV(const QString &pvPath);
    PVInfo getPV(const PVData &pv);

    QVector<PVInfo> getVGAllPV(const QString &vgName);
    QVector<PVInfo> getVGAllUsedPV(const QString &vgName);
    QVector<PVInfo> getVGAllUnUsedPV(const QString &vgName);
    QList<QString> getVGOfDisk(const QString &vgName, const QString &disk);

    bool lvInfoExists(const QString &vgName, const QString &lvName);
    bool lvInfoExists(const QString &lvPath);//  /dev/vg01/lv01   /dev/mapper/vg01-lv01

    bool vgExists(const QString &vgName);
    bool vgExists(const PVData &pv);
    bool vgExists(const PVInfo &pv);

    bool pvExists(const QString &pvPath);
    bool pvExists(const PVData &pv);

    bool pvOfVg(const QString &vgName, const QString &pvPath);
    bool pvOfVg(const QString &vgName, const PVData &pv);
    bool pvOfVg(const PVInfo &pv);
    bool pvOfVg(const VGInfo &vg, const PVInfo &pv);
    bool pvOfVg(const VGInfo &vg, const PVData &pv);
    bool pvOfVg(const QString &vgName, const PVInfo &pv);

private:
    template<class T>
    T getItem(const QString &str, const QMap<QString, T> &containers);

    template<class T>
    bool itemExists(const QString &str, const QMap<QString, T> &containers);

    QVector<PVInfo> getVGPVList(const QString &vgName, bool isUsed = true);
public:
    QMap<QString, VGInfo> m_vgInfo;        //lvm设备信息 key:vgName value vginfo
    QMap<QString, PVInfo> m_pvInfo;        //lvm pv信息 key:/dev/sdb1 value:pvinfo
    LVMError m_lvmErr{LVMError::LVM_ERR_NORMAL};
};
DBUSStructEnd(LVMInfo)


#endif // LVMSTRUCT_H

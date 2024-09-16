// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGICALCPU_H
#define LOGICALCPU_H
#include <QString>

class LogicalCpuPrivate;

class LogicalCpu
{
    Q_DECLARE_PRIVATE(LogicalCpu)
public:
    LogicalCpu();
    /**
     * @brief setPhysicalID : 设置物理id
     * @param value : id
     */
    void setPhysicalID(int value);

    /**
     * @brief setCoreID : 设置核心id
     * @param value : id
     */
    void setCoreID(int value);

    /**
     * @brief setLogicalID : 设置逻辑id
     * @param value : id
     */
    void setLogicalID(int value);

    /**
     * @brief setL1dCache :set l1d cache
     * @param value : id
     */
    void setL1dCache(const QString &value);

    /**
     * @brief setL1iCache : set l1i cache
     * @param value : cache
     */
    void setL1iCache(const QString &value);

    /**
     * @brief setL2Cache : set l2 cache
     * @param value : cache
     */
    void setL2Cache(const QString &value);

    /**
     * @brief setL3Cache : set l3 cache
     * @param value cache
     */
    void setL3Cache(const QString &value);

    /**
     * @brief setL4Cache : set l4 cache
     * @param value cache
     */
    void setL4Cache(const QString &value);

    /**
     * @brief setMinFreq : set min freq
     * @param value : freq
     */
    void setMinFreq(const QString &value);

    /**
     * @brief setCurFreq : set cur freq
     * @param value : freq
     */
    void setCurFreq(const QString &value);

    /**
     * @brief setMaxFreq : set max freq
     * @param value : id
     */
    void setMaxFreq(const QString &value);

    /**
     * @brief setModel : set model
     * @param value : id
     */
    void setModel(const QString &value);

    /**
     * @brief setModelName : set model name
     * @param value : name
     */
    void setModelName(const QString &value);

    /**
     * @brief setStepping : set stepping
     * @param value : stepping value
     */
    void setStepping(const QString &value);

    /**
     * @brief setVendor : set vendor name
     * @param value : vendor name
     */
    void setVendor(const QString &value);

    /**
     * @brief setcpuFamily : set cpu family
     * @param value : cpu family
     */
    void setcpuFamily(const QString &value);

    /**
     * @brief setFlags : set flags
     * @param value : flags
     */
    void setFlags(const QString &value);

    /**
     * @brief setBogomips
     * @param value
     */
    void setBogomips(const QString &value);

    /**
     * @brief diagPrintInfo
     */
    void diagPrintInfo();

    /**
     * @brief setArch
     * @param value
     */
    void setArch(const QString &value);

    /**
     * @brief physicalID
     * @return : value
     */
    int physicalID();

    /**
     * @brief coreID
     * @return : value
     */
    int coreID();

    /**
     * @brief logicalID
     * @return : value
     */
    int logicalID();

    /**
     * @brief l1dCache
     * @return : value
     */
    const QString &l1dCache();

    /**
     * @brief l1iCache
     * @return : value
     */
    const QString &l1iCache();

    /**
     * @brief l2Cache
     * @return : value
     */
    const QString &l2Cache();

    /**
     * @brief l3Cache
     * @return : value
     */
    const QString &l3Cache();

    /**
     * @brief l4Cache
     * @return : value
     */
    const QString &l4Cache();

    /**
     * @brief minFreq
     * @return : value
     */
    const QString &minFreq();

    /**
     * @brief curFreq
     * @return : value
     */
    const QString &curFreq();

    /**
     * @brief maxFreq
     * @return : value
     */
    const QString &maxFreq();

    /**
     * @brief model
     * @return : value
     */
    const QString &model();

    /**
     * @brief modelName
     * @return : value
     */
    const QString &modelName();

    /**
     * @brief stepping
     * @return : value
     */
    const QString &stepping();

    /**
     * @brief vendor
     * @return : value
     */
    const QString &vendor();

    /**
     * @brief cpuFamliy
     * @return : value
     */
    const QString &cpuFamliy();

    /**
     * @brief flags
     * @return : value
     */
    const QString &flags();

    /**
     * @brief bogomips
     * @return : value
     */
    const QString &bogomips();

    /**
     * @brief arch
     * @return
     */
    const QString &arch();


private:
    LogicalCpuPrivate *d_ptr;

};

class LogicalCpuPrivate
{
    Q_DECLARE_PUBLIC(LogicalCpu)
public:
    explicit LogicalCpuPrivate(LogicalCpu *q)
        : q_ptr(q)
        , logicalID(-1)
        , coreID(-1)
        , physicalID(-1)
        , l1d_cache("")
        , l1i_cache("")
        , l2_cache("")
        , l3_cache("")
        , l4_cache("")
        , max_freq("")
        , min_freq("")
        , cur_freq("")
        , model("")
        , modelName("")
        , setpping("")
        , vendor("")
        , cpuFamily("")
        , flags("")
        , bogoMips("")
        , arch("")
    {
    }
    LogicalCpu *q_ptr;                 //LogicalCpu 中的q指针

    int         logicalID;             // logical id
    int         coreID;                // core id
    int         physicalID;            // physical id
    QString     l1d_cache;             // l1d cache
    QString     l1i_cache;             // l1i cache
    QString     l2_cache;              // l2 cache
    QString     l3_cache;              // l3 cache
    QString     l4_cache;              // l4 cache
    QString     max_freq;              // max freq
    QString     min_freq;              // min freq
    QString     cur_freq;              // cur freq
    QString     model;                 // model
    QString     modelName;             // model name
    QString     setpping;              // stepping
    QString     vendor;                // vendor
    QString     cpuFamily;             // cpu family
    QString     flags;                 // flags
    QString     bogoMips;              // bogomips
    QString     arch;
};

#endif // LOGICALCPU_H

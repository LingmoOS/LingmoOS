// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef LUKSOPERATOR_H
#define LUKSOPERATOR_H

#include "lvmstruct.h"
#include "deviceinfo.h"

class LUKSOperator
{
public:
    LUKSOperator();

    /**
     * @brief 更新设备上的luks数据
     * @param dev: 设备集合
     * @param lvmInfo:lvm数据结构体
     * @param luks:luksMap数据结构体
     * @return true 成功 false 失败
     */
    static bool updateLUKSInfo(DeviceInfoMap &dev, LVMInfo &lvmInfo, LUKSMap &luks);

    /**
     * @brief 加密 加密设备并且添加token
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool encrypt(LUKSMap &luks, LUKS_INFO &luksInfo);

    /**
     * @brief 解密
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool decrypt(LUKSMap &luks, LUKS_INFO &luksInfo);

    /**
     * @brief 关闭加密映射
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool closeMapper(LUKSMap &luks, LUKS_INFO &luksInfo);

    /**
     * @brief 添加keyfile 并修改/etc/crypttab文件
     * @param luks: luks属性集合
     * @param luksInfo: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool addKeyAndCrypttab(LUKSMap &luks, LUKS_INFO &luksInfo);

    /**
     * @brief 删除keyfile 并修改/etc/crypttab文件
     * @param luks: luks属性集合
     * @param luksInfo: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool deleteKeyAndCrypttab(LUKSMap &luks, LUKS_INFO &luksInfo);

    /**
     * @brief 关闭加密映射 删除keyfile 并修改/etc/crypttab文件
     * @param luks: luks属性集合
     * @param luksInfo: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool removeMapperAndKey(LUKSMap &luks, LUKS_INFO &luksInfo);

private:
    /**
     * @brief 初始化luksMap
     * @param luks:luksMap数据结构体
     * @return true 成功 false 失败
     */
    static void resetLuksMap(LUKSMap &luks);

    /**
     * @brief 初始化映射设备
     */
    static bool initMapper(LUKSMap &luks);

    /**
     * @brief 获取支持的算法
     * @param support: 算法结构体
     * @return true 获取成功 false 获取失败
     */
    static bool getCIPHERSupport(CRYPT_CIPHER_Support &support);

    /**
     * @brief 获取luks属性
     * @param devPath: 设备路径
     * @param info: luks属性结构体
     * @return true 获取成功 false 获取失败
     */
    static bool getLUKSInfo(const LUKSMap &luks, const QString &devPath, LUKS_INFO &info);

    /**
     * @brief 通过json解析luks属性
     * @param json: luks属性json字符串
     * @param info: luks属性结构体
     * @return true 获取成功 false 获取失败
     */
    static bool jsonToLUKSInfo(QString json, LUKS_INFO &info);

    /**
     * @brief 判断是否luks设备
     * @param devPath: 设备路径
     * @return true luks设备 false 非luks设备
     */
    static bool isLUKS(QString devPath);

    /**
     * @brief 加密
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool format(const LUKS_INFO &luks);

    /**
     * @brief 打开映射
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool open(const LUKS_INFO &luks);


    /**
     * @brief 打开映射
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool testKey(const LUKS_INFO &luks);

    /**
     * @brief 关闭加密映射
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool close(const LUKS_INFO &luks);

    /**
     * @brief 添加token
     * @param luks: luks属性结构体
     * @param list: tokens列表
     * @return true 成功 false 失败
     */
    static bool addToken(const LUKS_INFO &luks, QStringList list, int number = -1);

    /**
     * @brief 添加token
     * @param luks: luks属性结构体
     * @param list: tokens列表
     * @return true 成功 false 失败
     */
    static bool removeToken(const LUKS_INFO &luks, int number);

    /**
     * @brief 更新token  0号位置
     * @param luks: luks属性结构体
     * @param isFirst: 是否首次创建
     * @return true 成功 false 失败
     */
    static bool updateDecryptToken(LUKS_INFO &info, bool isFirst);

    /**
     * @brief 添加key
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool addKeyFile(const LUKS_INFO &luks);

    /**
     * @brief 添加key
     * @param luks: luks属性结构体
     * @return true 成功 false 失败
     */
    static bool deleteKeyFile(const LUKS_INFO &luks);

    /**
     * @brief 修改/etc/crypttab文件
     * @param luks: luks属性结构体
     * @param isMount: 是否挂载
     * @return true 成功 false 失败
     */
    static bool wirteCrypttab(LUKS_INFO &luksInfo, bool isMount);

    /**
     * @brief 设置luks错误
     * @param info: luks属性结构体
     * @param err: 错误
     * @return true 正常 false 错误
     */
    static bool setLUKSErr(LUKSMap &luksInfo, const CRYPTError &err);

    /**
     * @brief 设置luks错误
     * @param info: luks属性结构体
     * @param err: 错误
     * @return true 正常 false 错误
     */
    static bool setLUKSErr(LUKSMap &luksInfo, LUKS_INFO &info, const CRYPTError &err);
private:
    static DeviceInfoMap *m_dev;
    static LVMInfo *m_lvmInfo;
    static CRYPTError m_cryErr;
};

#endif // LUKSOPERATOR_H

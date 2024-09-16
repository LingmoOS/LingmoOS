// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEEPINDEBINSTALLERLIB_H
#define DEEPINDEBINSTALLERLIB_H

#include "deepin-deb-installer-lib_global.h"

#include <QtCore>
#include <QObject>

class PackagesManager;
class DEEPINDEBINSTALLERLIBSHARED_EXPORT DeepinDebInstallerLib : public QObject
{
    Q_OBJECT
public:
    DeepinDebInstallerLib();

    virtual ~DeepinDebInstallerLib();

public:
    /**
     * @brief addPackages 添加deb包到程序中
     * @param debFilePath 包的路径
     */
    void addPackages(const QStringList &debFilePath);

    /**
     * @brief deletePackage 在程序中删除指定包
     * @param index 要删除的包的下表
     */
    void deletePackage(int index = 0);

    /**
     * @brief checkPackageFile 检查指定包的下标
     * @param index 指定包的下标
     * @return true:包有效  false:包无效
     */
    bool checkPackageFile(int index = 0);

    /**
     * @brief checkPkgDependsStatus 检查包的依赖是否满足要求
     * @param index 指定包的下标
     * @return true:包可安装 false:依赖错误，不可安装
     */
    bool checkPkgDependsStatus(int index = 0);

    /**
     * @brief checkDigitalSignature 检查指定包的数字签名
     * @param index 指定包的下标
     * @return true:包数字签名验证成功  false：包数字签名验证失败
     */
    bool checkDigitalSignature(int index = 0);

    /**
     * @brief checkInstallStatus 检查包的安装状态
     * @param index 指定包的下标
     * @return
     *          0：安装状态未知
     *          1：包未安装
     *          2：当前已经安装相同版本
     *          3：当前安装了较早的版本
     *          4：当前已经安装了更新的版本
     */
    int checkInstallStatus(int index = 0);

    /**
     * @brief install 开始安装已经添加的包
     */
    void install();

    /**
     * @brief uninstall 卸载某个包
     * @param index 要卸载的包的下标
     */
    void uninstall(int index = 0);

signals:

    void signal_startInstall();

    /**
     * @brief installProcess 当前操作的进度
     * @param 进度信息
     */
    void signal_installProcess(int);

    /**
     * @brief installDetails 当前安装的过程信息
     */
    void signal_installDetails(QString);

    /**
     * @brief installFinished 安装成功
     */
    void signal_installFinished();

    /**
     * @brief signal_uninstallFinished 卸载成功
     */
    void signal_uninstallFinished();

    /**
     * @brief installFailedReason 安装失败
     *  int:        安装失败的错误代码
     *  QString:    安装失败的错误信息
     */
    void signal_installFailedReason(int, QString);

    void signal_removePackageSuccess(int);

signals:
    /**
     * @brief signal_invalidPackage 当前添加的包无效
     *
     * @param 包的下标
     */
    void signal_invalidPackage(int);

    /**
     * @brief signal_invalidIndex 当前处理的包的下标无效
     * @param 包的下标
     */
    void signal_invalidIndex(int);

    /**
     * @brief signal_dependStatusError 当前添加的包依赖错误
     *
     * @param 包的下标
     * @param 依赖的状态
     *
     *      0: 依赖状态未知
     *      1: 依赖满足
     *      2: 依赖可用但是需要下载
     *      3: 依赖不满足
     *      4: 依赖下载授权失败
     *      5: 架构错误
     */
    void signal_dependStatusError(int, int);

    /**
     * @brief signal_signtureError 当前添加的包签名错误
     *
     * @param 包的下标
     * @param 签名状态
     *
     *      0: 签名状态未知
     *      1: 签名验证成功
     *      2: 解压deb文件用的临时目录不存在
     *      3: 提取deb包内容时出错
     *      4: 签名数据校验失败
     */
    void signal_signtureError(int, int);

    /**
     * @brief signal_packageAlreadyExits 当前添加的包已经添加过
     * @param 包的下标
     */
    void signal_packageAlreadyExits(int);

    /**
     * @brief signal_appendPackageSuccess 添加包成功
     * @param 包的下标
     */
    void signal_appendPackageSuccess(int);

    /**
     * @brief signal_packageNotInstalled 卸载时未安装
     */
    void signal_packageNotInstalled(int);

private:
    /**
     * @brief m_pPackageManager 后端包管理对象
     */
    PackagesManager *m_pPackageManager;

private:
    /**
     * @brief initConnections 连接信号与槽
     */
    void initConnections();
};

#endif  // DEEPINDEBINSTALLERLIB_H

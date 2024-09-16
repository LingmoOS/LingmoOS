// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PackageSigntureStatus_H
#define PackageSigntureStatus_H

#include <QObject>
#include <QProcess>

#define deb_verify_tool "/usr/bin/deepin-deb-verify"

enum SigntureStatus {
    SigntureUnknown = -1,   // 其他错误
    SigntureVerifySuccess,  // 验证成功
    SigntureInexistence,    // 解压deb文件用的临时目录不存在
    SigntureExtractFail,    // 提取deb包内容时出错
    SigntureVerifyFail      // deb包验证失败
};

class PackageSigntureStatus
{
public:
    PackageSigntureStatus();

    ~PackageSigntureStatus();

    /**
     * @brief checkPackageSignture 检查包的签名状态
     * @param packagePath 包的路径
     * @return
     */
    SigntureStatus checkPackageSignture(QString packagePath);

private:
    /**
     * @brief checkDigitalVerifyTools 检查当前系统中是否存在验证工具
     * @return true: 存在签名验证工具   false:签名验证工具不存在
     */
    bool checkDigitalVerifyTools();

    /**
     * @brief checkDeviceLock 检查当前是否开启开发者模式
     * @return true: 已开启开发者模式   false: 未开启开发者模式
     */
    bool checkDeviceLock();

private:
    QProcess *m_pCheckSignProc = nullptr;

    PackageSigntureStatus(const PackageSigntureStatus &rhs) = delete;
    PackageSigntureStatus &operator=(const PackageSigntureStatus &rhs) = delete;
};

#endif  // PackageSigntureStatus_H

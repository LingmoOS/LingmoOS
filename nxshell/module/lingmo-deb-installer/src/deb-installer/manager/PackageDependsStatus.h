// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGEDEPENDSSTATUS_H
#define PACKAGEDEPENDSSTATUS_H
#include <QObject>

class PackageDependsStatus
{
public:
    /**
     * @brief ok 组装依赖的状态，将传入的包的依赖状态设置为ok
     * @return 某个包的依赖状态为ok
     */
    static PackageDependsStatus ok();
    /**
     * @brief available 组装依赖的状态，将传入的包名的依赖状态设置为Available
     * @param package 当前依赖包的包名。
     * @return 某个包的依赖状态为Available
     */
    static PackageDependsStatus available(const QString &package);

    /**
     * @brief _break 组装break依赖状态。将传入的包的依赖状态设置为break
     * @param package 包的名字
     * @return 某个包的依赖状态为break
     */
    static PackageDependsStatus _break(const QString &package);

    /**
     * @brief _prohibit 将传入的报名的依赖状态设置为_prohibit
     * @param package 包的名字
     * @return 包依赖状态为_prohibit
     */
    static PackageDependsStatus _prohibit(const QString &package);

    PackageDependsStatus();
    PackageDependsStatus(const int status, const QString &package);

    /**
     * @brief operator = 重写=操作符
     * @param other 要赋值的依赖的状态
     * @return 赋值后的依赖的状态
     */
    PackageDependsStatus &operator=(const PackageDependsStatus &other);

    /**
     * @brief max   比较当前状态并返回状态值更大的那一个
     * @param other 与当前状态比较的另一个依赖状态
     * @return 两个状态中更大的一个
     */
    PackageDependsStatus max(const PackageDependsStatus &other);

    /**
     * @brief maxEq   比较当前状态并返回状态值更大的那一个
     * @param other 与当前状态比较的另一个依赖状态
     * @return 两个状态中更大的一个
     */
    PackageDependsStatus maxEq(const PackageDependsStatus &other);

    /**
     * @brief min   比较当前状态并返回状态值更小的那一个
     * @param other 与当前状态比较的另一个依赖状态
     * @return 两个状态中更小的一个
     */
    PackageDependsStatus min(const PackageDependsStatus &other);

    /**
     * @brief minEq   比较当前状态并返回状态值更小的那一个
     * @param other 与当前状态比较的另一个依赖状态
     * @return 两个状态中更小的一个
     */
    PackageDependsStatus minEq(const PackageDependsStatus &other);

    /**
     * @brief 当前依赖状态是否为break
     *
     * @return true 当前依赖状态为break
     * @return false 当前依赖状态不是break
     */
    bool isBreak() const;

    /**
     * @brief 当前依赖状态是否为AuthCancel
     *
     * @return true 当前依赖状态是AuthCancle
     * @return false 当前依赖状态不是AuthCancle
     */
    bool isAuthCancel() const;

    /**
     * @brief 当前依赖状态是否为Available
     *
     * @return true 当前依赖状态是Available
     * @return false 当前依赖状态不是Available
     */
    bool isAvailable() const;

    /**
     * @brief isProhibit 当前依赖状态是否为prohibit
     * @return true 当前依赖状态是prohibit
     * @return fasle 当前依赖状态不是prohibit
     */
    bool isProhibit() const;

    bool canInstall() const;
    bool canInstallCompatible() const;

public:
    int status;
    QString package;
};
#endif  // PACKAGEDEPENDSSTATUS_H

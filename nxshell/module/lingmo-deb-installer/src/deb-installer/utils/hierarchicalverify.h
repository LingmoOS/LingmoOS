// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIERARCHICALVERIFY_H
#define HIERARCHICALVERIFY_H

#include <QObject>
#include <QSet>

class HierarchicalVerify : public QObject
{
    Q_OBJECT
    explicit HierarchicalVerify();
    ~HierarchicalVerify() override;

public:
    static HierarchicalVerify *instance();

    bool isValid();
    bool checkTransactionError(const QString &pkgName, const QString &errorString);
    bool pkgVerifyPassed(const QString &pkgName);
    void clearVerifyResult();

    Q_SLOT void proceedDefenderSafetyPage();
    Q_SIGNAL void validChanged(bool valid);

private:
    bool checkHierarchicalInterface();
    bool checkValidImpl();

private:
    bool valid = false;             ///< 分级管控是否开启
    bool interfaceInvalid = false;  ///< DBus接口是否有效
    QSet<QString> invalidPackages;  ///< 验签失败的包集合

    Q_DISABLE_COPY(HierarchicalVerify)
};

#endif  // HIERARCHICALVERIFY_H

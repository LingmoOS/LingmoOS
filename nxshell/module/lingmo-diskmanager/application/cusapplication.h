// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef CUSAPPLICATION_H
#define CUSAPPLICATION_H

#include <DApplication>

DWIDGET_USE_NAMESPACE

/**
 * @class CusApplication
 * @brief 自定义Application类
*/

class CusApplication : public DApplication
{
    Q_OBJECT
public:
    CusApplication(int &argc, char **argv);

protected:
    void handleQuitAction() override;

signals:
    void handleQuitActionChanged();
};

#endif // CUSAPPLICATION_H

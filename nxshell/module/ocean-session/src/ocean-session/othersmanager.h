// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OTHERS_MANAGER_H
#define OTHERS_MANAGER_H

/**
 * @brief The OthersManager class
 * @brief 处理一些比较琐碎不好归类的事情，后面功能做的差不多了再梳理
 */
class OthersManager
{
public:
    OthersManager();

    void init();

private:
    void launchWmChooser();

    bool inVM();
};

#endif // OTHERS_MANAGER_H

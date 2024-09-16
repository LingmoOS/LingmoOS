// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONHELPER_H
#define COOPERATIONHELPER_H

#include <QWidget>
#include <QPair>

namespace dfmplugin_cooperation {

class CooperationHelper
{
public:
    [[nodiscard]] static QPair<QWidget *, QWidget *> createSettingButton(QObject *opt);

    static void showSettingDialog();
};

}   // namespace dfmplugin_cooperation

#endif   // COOPERATIONHELPER_H

/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KSECURITYLEVELBAR_H
#define KSECURITYLEVELBAR_H

#include "gui_g.h"
#include <QWidget>

namespace kdk
{

/** @defgroup MessageAlertModule
  * @{
  */

/**
 * @brief 分为三个安全等级
 */
enum SecurityLevel
{
    Low,
    Medium,
    High
};

class KSecurityLevelBarPrivate;

/**
 * @brief 密码安全等级提示条，支持三个安全等级
 */
class GUI_EXPORT KSecurityLevelBar : public QWidget
{
    Q_OBJECT
public:
    explicit KSecurityLevelBar(QWidget *parent = nullptr);

    /**
     * @brief 设置安全等级
     * @param level
     */
    void setSecurityLevel(SecurityLevel level);

    /**
     * @brief 获取安全等级
     * @return
     */
    SecurityLevel securityLevel();

protected:
    void paintEvent(QPaintEvent* event);

private:
    Q_DECLARE_PRIVATE(KSecurityLevelBar)
    KSecurityLevelBarPrivate*const d_ptr;
};
}
/**
  * @example testsecuritylevelbar/widget.h
  * @example testsecuritylevelbar/widget.cpp
  * @}
  */

#endif // KSECURITYLEVELBAR_H

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

#ifndef KPASSWORDEDIT_H
#define KPASSWORDEDIT_H

#include "gui_g.h"
#include <QLineEdit>

namespace kdk
{

/** @defgroup InputBoxModule
  * @{
  */

/**
 * @brief 支持三种状态
 */
enum LoginState
{
    Ordinary,
    LoginSuccess,
    LoginFailed
};
class KPasswordEditPrivate;

/**
 * @brief 密码输入框，继承于QLineEdit,支持三种不同状态显示
 */
class GUI_EXPORT KPasswordEdit:public QLineEdit
{
    Q_OBJECT

public:
    KPasswordEdit(QWidget*parent);

    /**
     * @brief 设置状态
     * @param state
     */
    void setState(LoginState state);

    /**
     * @brief 返回状态
     * @return
     */
    LoginState state();

    /**
     * @brief 设置加载状态
     * @param flag
     */
    void setLoading(bool flag);

    /**
     * @brief 判断是否处于加载状态
     * @return
     */
    bool isLoading();

    /**
     * @brief 返回placeholderText
     * @return
     */
    QString placeholderText();

    /**
     * @brief 设置PlaceholderText
     * @param text
     */
    void setPlaceholderText(QString&text);

    /**
     * @brief 设置是否启用ClearButton
     * @param enable
     */
    void setClearButtonEnabled(bool enable);

    /**
     * @brief 返回是否启用了ClearButton
     * @return
     */
    bool isClearButtonEnabled() const;

    /**
     * @brief 设置EchoModeBtn是否可见
     * @return
     * @since 1.2
    */
    void setEchoModeBtnVisible(bool enable);

    /**
     * @brief 返回EchoModeBtn是否可见
     * @return
     * @since 1.2
    */
    bool echoModeBtnVisible();

    /**
     * @brief 设置ClearBtn是否可见
     * @return
     * @since 1.2
    */
    void setClearBtnVisible(bool enable);

    /**
     * @brief 返回ClearBtn是否可见
     * @return
     * @since 1.2
    */
    bool clearBtnVisible();

    /**
     * @brief 设置KLineEdit是否可用
     * @return
    */
    void setEnabled(bool);

    /**
     * @brief setEchoMode
     * @since 1.2
     */
    void setEchoMode(EchoMode mode);

    /**
     * @brief 设置是否走默认palette
     * @param flag
     * @since 1.2
     */
    void setUseCustomPalette(bool flag);


protected:
    void resizeEvent(QResizeEvent*event);
    bool eventFilter(QObject *watched, QEvent *event);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KPasswordEdit)
    KPasswordEditPrivate* const d_ptr;
};
}
/**
  * @example testpasswordedit/widget.h
  * @example testpasswordedit/widget.cpp
  * @}
  */

#endif // KPASSWORDEDIT_H

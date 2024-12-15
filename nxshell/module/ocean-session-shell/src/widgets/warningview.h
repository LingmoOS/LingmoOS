// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WARNINGVIEW_H
#define WARNINGVIEW_H

#include <QFrame>

/*!
 * \brief The ButtonType enum 按钮枚举
 */
enum ButtonType {
    Cancel, // 取消
    Accept  // 接受
};

class WarningView : public QFrame
{
public:
    explicit WarningView(QWidget *parent = nullptr) : QFrame(parent) {}
    virtual void toggleButtonState() = 0;
    virtual void buttonClickHandle() = 0;
    virtual void setAcceptReason(const QString &reason) = 0;

protected:
    /*!
     * \brief setCurrentButton 设置当前的按钮
     * \param btntype 按钮枚举
     */
    virtual void setCurrentButton(const ButtonType btntype) = 0;
};

#endif // WARNINGVIEW_H

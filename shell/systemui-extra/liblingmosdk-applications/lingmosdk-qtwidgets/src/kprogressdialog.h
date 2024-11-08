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

#ifndef KPROGRESSDIALOG_H
#define KPROGRESSDIALOG_H

#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QDialog>
#include "kwidget.h"
#include "gui_g.h"
#include "themeController.h"
#include "kwindowbuttonbar.h"
#include "kiconbar.h"
#include "kdialog.h"


namespace kdk
{

/** @defgroup DialogBoxModule
  * @{
  */

class KProgressDialogPrivate;

/**
 * @brief 进度对话框，会实时显示当前进度
 */
class GUI_EXPORT KProgressDialog:public KDialog
{
    Q_OBJECT

public:
    explicit KProgressDialog(QWidget *parent = nullptr);
    KProgressDialog(const QString &labelText, const QString &cancelButtonText="cancel",
                    int minimum=0, int maximum=100, QWidget *parent = nullptr);
    ~KProgressDialog();

    /**
     * @brief 设置提示文字
     * @param label
     */
    void setLabel(QLabel *label);

    /**
     * @brief 设置取消按钮
     * @param button
     */
    void setCancelButton(QPushButton *button);

    /**
     * @brief 设置进度条
     * @param bar
     */
    void setBar(QProgressBar *bar);

    /**
     * @brief 设置detail的后缀
     * @param suffix
     */
    void setSuffix(const QString& suffix);

    /**
     * @brief 设置是否显示详细信息
     * @param flag
     */
    void setShowDetail(bool flag);

    /**
     * @brief 返回最小值
     * @return
     */
    int minimum() const;

    /**
     * @brief 返回最大值
     * @return
     */
    int maximum() const;

    /**
     * @brief 返回当前值
     * @return
     */
    int value() const;

    /**
     * @brief 返回提示内容
     * @return
     */
    QString labelText() const;

    /**
     * @brief 设置自动重置
     * @param reset
     */
    void setAutoReset(bool reset);

    /**
     * @brief 返回是否自动重置
     * @return
     */
    bool autoReset() const;

    /**
     * @brief 设置是否自动关闭对话框
     * @param close
     */
    void setAutoClose(bool close);

    /**
     * @brief 返回是否自动关闭对话框
     * @return
     */
    bool autoClose() const;

    /**
     * @brief 获取进度条
     * @return
     */
    QProgressBar* progressBar();

Q_SIGNALS:
    void canceled();

public Q_SLOTS:
    /**
     * @brief 取消进度条
     */
    void cancel();

    /**
     * @brief 重置进度条
     */
    void reset();

    /**
     * @brief 设置最大值
     * @param maximum
     */
    void setMaximum(int maximum);

    /**
     * @brief 设置最小值
     * @param minimum
     */
    void setMinimum(int minimum);

    /**
     * @brief 设置范围
     * @param minimum
     * @param maximum
     */
    void setRange(int minimum, int maximum);

    /**
     * @brief 设置当前值
     * @param progress
     */
    void setValue(int progress);

    /**
     * @brief 设置提示内容
     * @param text
     */
    void setLabelText(const QString &text);

    /**
     * @brief 设置取消按钮内容
     * @param text
     */
    void setCancelButtonText(const QString &text);

    /**
     * @brief 设置次级内容
     * @param text
     */
    void setSubContent(const QString&text);

protected:
    void changeTheme();

private:
    Q_DECLARE_PRIVATE(KProgressDialog)
    KProgressDialogPrivate *const d_ptr;
};
}
/**
  * @example testprogressdialog/widget.h
  * @example testprogressdialog/widget.cpp
  * @}
  */
#endif // KPROGRESSDIALOG_H

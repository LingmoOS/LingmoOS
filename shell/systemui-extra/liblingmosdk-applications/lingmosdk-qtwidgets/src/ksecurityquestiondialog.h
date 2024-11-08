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
 * Authors: SiSheng He <hesisheng@kylinos.cn>
 *
 */

#ifndef KSECURITYQUESTIONDIALOG_H
#define KSECURITYQUESTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QBoxLayout>
#include "gui_g.h"
#include "kdialog.h"
#include "kpushbutton.h"

namespace kdk
{

class KSecurityQuestionDialogPrivate;

/**
 * @brief 继承自 KDialog，
 * @since 2.4
 */
class GUI_EXPORT KSecurityQuestionDialog: public KDialog
{
    Q_OBJECT

public:
    explicit KSecurityQuestionDialog(QWidget* parent = nullptr);
    ~KSecurityQuestionDialog();

    /**
     * @brief 设置对话框标题
     *
     * @param text 标题
     */
    void setTitleText(const QString &text);

    /**
     * @brief 添加安全问题控件
     *Label combox
     *Label lineedit
     *      tipsLabel
     * @param count 控件个数
     * @param mutex 是否开启下拉框互斥校验，若开启后，已被选择项，在其他下拉框中被置灰
     */
    void addSecurityQuestionItem(const int count, bool mutex);

    /**
     * @brief 添加安全问题控件
     *Label combox/lineedit
     *Label lineedit
     *      tipsLabel
     * @param count 控件个数
     * @param mutex 是否开启下拉框互斥校验，若开启后，已被选择项，在其他下拉框中被置灰
     * @param isVerify 是否为校验窗口
     */
    void addSecurityQuestionItem(const int count, bool mutex, bool isVerify);

    /**
     * @brief 添加下拉框选项列表
     *
     * @param questionlist
     */
    void initQustionCombox(const QStringList &questionlist);

    /**
     * @brief 获取下拉框左侧Label
     *
     * @param questionIndex item索引 [0,count)
     * @return QLabel 下拉框左侧Label，若未获取到，返回nullptr
     */
    QLabel *questionLabel(const int questionIndex);

    /**
     * @brief 获取combox
     *
     * @param questionIndex item索引 [0,count)
     * @return QComboBox 下拉框，若未获取到，返回nullptr
     */
    QComboBox *questionCombox(const int questionIndex);

    /**
     * @brief 获取lineedit左侧label
     *
     * @param answerIndex item索引 [0,count)
     * @return QLabel lineedit左侧label，若未获取到，返回nullptr
     */
    QLabel *answerLabel(const int answerIndex);

    /**
     * @brief 获取 lineedit
     *
     * @param answerIndex item索引 [0,count)
     * @return QLineEdit lineedit，若未获取到，返回nullptr
     */
    QLineEdit *answerLineedit(const int answerIndex);

    /**
     * @brief 获取提示 label
     *
     * @param index item 索引 [0,count)
     * @return QLabel 提示label，若未获取到，返回nullptr
     */
    QLabel *tipsLabel(const int index);

    /**
     * @brief 获取取消按钮
     *
     * @return KPushButton
     */
    KPushButton *cancelButton();

    /**
     * @brief 获取确认按钮
     *
     * @return KPushButton
     */
    KPushButton *confirmButton();

protected:
    void keyPressEvent(QKeyEvent *);

private:
    Q_DECLARE_PRIVATE(KSecurityQuestionDialog)
    KSecurityQuestionDialogPrivate* const d_ptr;
};
}
/**
  * @example testsecurityquestiondialog/main.cpp
  * @}
  */
#endif // KSECURITYQUESTIONDIALOG_H

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <DDialog>
#include <DProgressBar>

DWIDGET_USE_NAMESPACE

/**
 * @class MessageBox
 * @brief 弹窗消息
 */

class MessageBox : public DDialog
{
    Q_OBJECT
public:
    explicit MessageBox(QWidget *parent = nullptr);

    /**
     * @brief 设置警告窗口
     * @param title 标题，没有标题写空字符串
     * @param warningMsg 文本内容
     * @param sureBtnText 确定按钮
     * @param sureAccessibleName 确定按钮属性名称，可不填
     * @param cancalBtnText="" 取消按钮，可不填
     * @param cancelAccessibleName 取消按钮属性名称，可不填
    */
    void setWarings(const QString &title, const QString &warningMsg, const QString &sureBtnText, const QString &sureAccessibleName = "", const QString &cancalBtnText = "", const QString &cancelAccessibleName = "");

    /**
     * @brief 设置警告窗口
     * @param title 标题，没有标题写空字符串
     * @param warningMsg 文本内容
     * @param sureBtnText 确定按钮
     * @param sureBtnType=ButtonNormal 确定按钮类型
     * @param sureAccessibleName 确定按钮属性名称，可不填
     * @param cancalBtnText="" 取消按钮，可不填
     * @param cancelAccessibleName 取消按钮属性名称，可不填
    */
    void setWarings(const QString &title, const QString &warningMsg, const QString &sureBtnText, ButtonType sureBtnType, const QString &sureAccessibleName = "", const QString &cancalBtnText = "", const QString &cancelAccessibleName = "");

    /**
     * @brief 设置警告窗口
     * @param title 标题
     * @param sureBtnText 确定按钮
     * @param sureBtnType=ButtonNormal 确定按钮类型
     * @param sureAccessibleName 确定按钮属性名称，可不填
     * @param cancalBtnText="" 取消按钮，可不填
     * @param cancelAccessibleName 取消按钮属性名称，可不填
    */
    void setWarings(const QString &title, const QString &sureBtnText, ButtonType sureBtnType, const QString &sureAccessibleName = "",
                    const QString &cancalBtnText = "", const QString &cancelAccessibleName = "");

    /**
     * @brief 设置进度窗口
     * @param title 标题，没有标题写空字符串
     * @param cancalBtnText 取消按钮
    */
    void setProgressBar(const QString &title, const QString &cancalBtnText);

    /**
     * @brief 设置label
     * @param text label内容
    */
    void addLabel(QString text);

    /**
     * @brief 设置进度条
    */
    void addProgressBar();

signals:

public slots:

protected:
    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    DProgressBar *m_progressBar; // 进度条
};

#endif // MESSAGEBOX_H

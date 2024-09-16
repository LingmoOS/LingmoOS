// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**

 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file messagebox.h
 *
 * @brief 提示窗口类
 *
 * @date 2020-06-09 10:52
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H
#include "global.h"
#include <DDialog>
#include <DLabel>
#include <DTextEdit>
#include <DCheckBox>
#include <DLineEdit>
#include <DRadioButton>

DWIDGET_USE_NAMESPACE

class QPushButton;

/**
 * @class MessageBox
 * @brief 弹窗消息
 */
class MessageBox : public DDialog
{
    Q_OBJECT
public:
    explicit MessageBox(QWidget *parent = nullptr);

public:
    /**
     * @brief 设置警告窗口
     * @param warningMsg 文本内容
     * @param surebtntext 确定按钮
     * @param cancalbtntext="" 取消按钮，可不填
     * @param sameUrlCount 相同url数量
     * @param sameUrlList 相同url列表
    */
    void setWarings(QString warningMsg, QString surebtntext, QString cancalbtntext = "", int sameUrlCount = 0, QList<QString> sameUrlList = {});

    /**
     * @brief 设置重新下载窗口
     * @param ret 默认false为新建任务时，重复下载。 为true时，为回收站和已完成列表重新下载
    */
    void setRedownload(const QString sameUrl, bool ret = false, bool isShowRedownload = false);

    /**
     * @brief 设置下载异常窗口（正在下载中，文件被删除）
    */
    void setUnusual(const QString &taskId, QString taskList);

    /**
     * @brief 设置删除窗口
     * @param permanentl bool类型，真为选择删除文件同时，删除本地文件。 假为可选择是否删除本地文件。
    */
    void setDelete(bool permanentl, bool checked = false);

    /**
     * @brief 设置清空窗口。可选择是否清空回收站及删除本地文件
    */
    void setClear();

    /**
     * @brief 设置退出窗口
    */
    void setExit();

    /**
     * @brief 设置文件选择权限警告窗口
    */
    void setFolderDenied();

    /**
     * @brief 设置文件不存在窗口
    */
    void setFolderNotExists();

    /**
     * @brief 设置网络链接错误
    */
    void setNetWorkError(QString warningMsg);


private:
    /**
     * @brief 设置label
     * @param text label内容
    */
    void addLabel(QString text);
    /**
     * @brief 设置radioGroup
     * @param text label内容
    */
    void addRadioGroup(QString quitText, QString minText);

    /**
     * @brief Checkbox
     * @param text Checkbox内容
    */
    void addCheckbox(QString checkboxText, bool checked = false);
signals:
    /**
     * @brief 重命名信号
     * @param newname 新名字
    */
    void Rename(QString &newname);

    /**
     * @brief 重新下载信号
     * @param newname 新名字
    */
    void reDownload(const QString sameUrl, QString fileName, QString type);

    /**
     * @brief 清空信号
     * @param ret 是否清空全部
    */
    void Clearrecycle(bool ret);

    /**
     * @brief 删除信号
     * @param ret
     * @param ret1
    */
    void Deletedownload(bool ret, bool ret1);

    /**
     * @brief 关闭确认信号
    */
    void closeConfirm(int index);

    /**
     * @brief 异常确认信号
    */
    void unusualConfirm(int index, const QString &taskId);

public slots:
    /**
     * @brief 重命名编辑框更新槽函数
     * @param text 更改内容
    */
    void onRenamelineeditChanged(const QString &text);
    /**
     * @brief 确定重命名
    */
    void onRenameSureBtnClicked();
    /**
     * @brief 确定清空
    */
    void onClearBtnClicked(int index);
    /**
     * @brief 确定删除
    */
    void onDeleteBtnClicked(int index);

    /** 右上角退出按钮按下弹出框的操作
     * @brief
    */
    void onExitBtnClicked(int index);

private:
    bool m_DeleteFlag;
    DCheckBox *m_CheckBox;
    DLineEdit *m_NewnameLineedit;
    QPushButton *m_RenameSureButton;

    DRadioButton *m_ButtonQuit;
    DRadioButton *m_ButtonMin;
};

#endif // MESSAGEBOX_H

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btinfodialog.h
 *
 * @brief BT窗口类
 *
 * @date 2020-06-09 10:50
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

#ifndef BTINFODIALOG_H
#define BTINFODIALOG_H

#include "settings.h"
#include "aria2cbtinfo.h"
#include "dfilechooseredit.h"
#include <DDialog>
#include <DTableView>
#include <DPushButton>
#include <DLineEdit>
#include <DLabel>
#include <DTitlebar>
#include <DCheckBox>
#include <DHeaderView>
#include <DFileDialog>
#include <DMessageBox>
#include <DStyleOption>
#include <DSuggestButton>

class MessageBox;
class BtInfoDelegate;
class headerView;
class BtInfoTableView;
class QStandardItemModel;

DWIDGET_USE_NAMESPACE

class BtInfoDialog : public DDialog
{
    Q_OBJECT

public:
    explicit BtInfoDialog(QString torrentFile, QString btLastSavePath,QWidget *parent = nullptr);
    ~BtInfoDialog() override;

    /**
     * @brief getSelected 获取选中下载的文件索引集合(用于aria2的select-file的参数)
     * @return 以逗号分割的索引字符串，如"1,2,3,6"
     */
    QString getSelected();

    /**
     * @brief getSaveto 获取选择的下载到路径
     * @return 选择的下载到路径
     */
    QString getSaveto();

    /**
     * @brief getName 获取bt信息下载名称
     * @return bt信息下载名称
     */
    QString getName();

    /**
     * @brief updateSelectedInfo 更新文件列表选中后的信息（Delegate内部调用）
     */
    void updateSelectedInfo();

    /**
     * @brief exec
     * @return
     */
    int exec() override;

    /**
     * @brief getUrlInfo 获取bt信息
     * @param opt bt基本信息
     * @param infoName 文件名字
     * @param infoHash 文件hash值
     */
    void getBtInfo(QMap<QString, QVariant> &opt, QString &infoName, QString &infoHash);

    enum DataRole {
        // Ischecked = 1,
        fileName = 0,
        type = 2,
        size = 3
    };

public slots:
    /**
     * @brief 确定按钮
     */
    bool onBtnOK();

private slots:
    /**
     * @brief 全选按钮
     */
    void onAllCheck();
    /**
     * @brief 视频按钮
     */
    void onVideoCheck();
    /**
     * @brief 视频按钮
     */
    void onAudioCheck();
    /**
     * @brief 视频按钮
     */
    void onPictureCheck();
    /**
     * @brief 视频按钮
     */
    void onOtherCheck();
    /**
     * @brief 选择下载路径
     */
    void onFilechoosed(const QString &);
    /**
     * @brief 随dtk主题变化
     */
    void onPaletteTypeChanged(DGuiApplicationHelper::ColorType type);

//    /**
//     * @brief 随dtk主题变化
//     * @param index 对应索引
//     */
//    void Sort(int index);

//    /**
//     * @brief 随dtk主题变化
//     * @param DataRole 所在行
//     * @param bool
//     */
//    void setTableData(BtInfoDialog::DataRole, bool ret);

private:
    void initUI(); //初始化UI
    bool isVideo(QString ext); //判断扩展名是否是常见视频格式
    bool isAudio(QString ext); //判断扩展名是否是常见音频格式
    bool isPicture(QString ext); //判断扩展名是否是常见图片格式
    QString getFileEditText(QString text);
    void setOkBtnStatus(int count); //根据选择文件设置确认按钮状态

    void sortByFileName(bool ret);
    void sortByType(bool ret);
    void sortBySize(bool ret);

protected:
    /**
     * @brief mainwidow关闭事件
     * @param event 事件类型
     */
    void closeEvent(QCloseEvent *event) override;

private:
    QString m_torrentFile; //bt文件路径
    QString m_defaultDownloadDir; //默热下载文件路径
    QStandardItemModel *m_model; //tableview中的模型，数据交流
    BtInfoDelegate *m_delegate; //tableview中选中表格item
    QList<Aria2cBtFileInfo> m_listBtInfo; //bt文件列表

    DTitlebar *m_titleBar; //标题栏
    BtInfoTableView *m_tableView; //列表
    DWidget *m_widget; //包裹view
    DPushButton *m_btnOK; //Download按钮
    DLabel *m_labelTitle; //标题
    DLabel *m_labelSelectedFileNum; //选中文件数
    DLabel *m_labelFileSize; //总大小标签
    DLabel *m_labelCapacityFree; //下载路径所在分区剩余磁盘容量
    DLabel *m_folderIcon; //文件icon
    DLabel *m_labelInfoName; //下载信息名称
    DFileChooserEdit *m_editDir; //选择下载路径窗口

    DCheckBox *m_checkAll; //文件类型全选
    DCheckBox *m_checkVideo; //视频文件类型
    DCheckBox *m_checkAudio; //音频文件类型
    DCheckBox *m_checkPicture; //图片文件类型
    DCheckBox *m_checkOther; //其他文件类型
    Aria2cBtInfo m_ariaInfo; //当前bt文件信息;
};

#endif // BTINFODIALOG_H

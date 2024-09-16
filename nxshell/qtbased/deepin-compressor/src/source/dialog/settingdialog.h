// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#endif // SETTINGDIALOG_H

#include <DSettingsDialog>
#include <DSettings>
#include <DSettingsOption>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#define AUTO_DELETE_NEVER "Never" //解压后不删除源压缩文件 QT_TR_NOOP("")
#define AUTO_DELETE_ASK "Ask for confirmation" //解压后询问是否删除源压缩文件
#define AUTO_DELETE_ALWAYS "Always" //解压后删除源压缩文件

class SettingDialog: public DSettingsDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog() override;

    /**
     * @brief getDefaultExtractPath 获取默认解压目录
     * @return
     */
    QString getDefaultExtractPath();

    /**
     * @brief isAutoCreatDir    是否创建新的目录进行解压
     * @return      true表示创建，false表示不创建
     */
    bool isAutoCreatDir();

    /**
     * @brief isAutoOpen    是否自动打开,到对应的文件目录
     * @return      true表示自动打开，false表示不打开
     */
    bool isAutoOpen();

    /**
     * @brief isAutoDeleteFile 压缩成功是否自动删除文件
     * @return true表示自动删除 false表示不自动删除
     */
    bool isAutoDeleteFile();

    /**
     * @brief isAutoDeleteArchive 解压成功是否自动删除压缩包
     * @return 表示自动删除 false表示不自动删除
     */
    QString isAutoDeleteArchive();

    /**
     * @brief isAssociatedType  是否是被关联压缩文件类型
     * @param mime  压缩文件的mimetype
     * @return
     */
    bool isAssociatedType(QString mime);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief createSettingButton   创建选择按钮
     */
    void createSettingButton();

    /**
     * @brief createPathBox    创建默认解压位置选项
     */
    void createPathBox();

    /**
     * @brief createDeleteBox   创建删除选项
     */
    void createDeleteBox();

    void writeConfbf();

signals:
    /**
     * @brief sigResetPath    点击恢复默认按钮，恢复默认解压路径设置的信号
     */
    void sigResetPath();

    /**
     * @brief sigResetDeleteArchive   点击恢复默认按钮，恢复默认解压后删除压缩文件设置的信号
     */
    void sigResetDeleteArchive();

private slots:
    /**
     * @brief settingsChanged   DSetting value发生改变
     * @param key   发生改变的key
     * @param value   key所对应改变后的值
     */
    void slotSettingsChanged(const QString &key, const QVariant &value);

    /**
     * @brief slotClickSelectAllButton   点击全选按钮
     */
    void slotClickSelectAllButton();

    /**
     * @brief slotClickCancelSelectAllButton    点击取消全选按钮
     */
    void slotClickCancelSelectAllButton();

    /**
     * @brief slotClickRecommendedButton    点击推荐选择按钮
     */
    void slotClickRecommendedButton();

private:
    DSettings *m_settings;          //
//    QStringList m_associtionList;   //

    DSettingsOption *m_customButtonOption;   // 按钮选项
    DSettingsOption *m_extractPathOption;     // 默认解压路径选项
    DSettingsOption *m_deleteArchiveOption;    // 解压后删除压缩文件选项

    QString m_curpath;
    int m_index_last;
    QString m_autoDeleteArchive;    // 解压后删除压缩文件方式
};

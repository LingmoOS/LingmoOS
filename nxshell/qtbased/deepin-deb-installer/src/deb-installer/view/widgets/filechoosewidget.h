// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILECHOOSEWIDGET_H
#define FILECHOOSEWIDGET_H

#include <DLabel>
#include <DPushButton>

#include <QSettings>
#include <QWidget>

class ChooseFileButton;

DWIDGET_USE_NAMESPACE

class FileChooseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileChooseWidget(QWidget *parent = nullptr);

signals:
    /**
     * @brief packagesSelected 选择文件信号
     * @param files 通过文件选择窗口选择的文件
     */
    void packagesSelected(const QStringList &files) const;

protected:
    /**
       @brief 展示控件时设置默认焦点
     */
    void showEvent(QShowEvent *e);

private slots:
    /**
     * @brief chooseFiles 打开文件选择窗口，选择文件后将文件保存
     */
    void chooseFiles();
    /**
     * @brief themeChanged 更换主题时，修改UI
     */
    void themeChanged();

private:
    ChooseFileButton *m_chooseFileBtn = nullptr;  // 文件选择按钮

    DLabel *split_line = nullptr;   // 分割线
    DLabel *m_dndTips = nullptr;    // 拖入提示语
    DLabel *m_iconImage = nullptr;  // 图标

    QSettings m_settings;  // 保存上次打开的文件路径
};

#endif  // FILECHOOSEWIDGET_H

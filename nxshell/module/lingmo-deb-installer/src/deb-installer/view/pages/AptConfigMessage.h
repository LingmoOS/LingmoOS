// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APTCONFIGMESSAGE_H
#define APTCONFIGMESSAGE_H

#include "view/widgets/installprocessinfoview.h"

#include <DLabel>
#include <DLineEdit>
#include <DMainWindow>
#include <DIconButton>
#include <DSuggestButton>

#include <QWidget>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

/**
 * @brief The AptConfigMessage class
 * 配置包安装界面
 */
class AptConfigMessage : public DMainWindow
{
    Q_OBJECT

public:
    explicit AptConfigMessage(QWidget *parent = nullptr);
    ~AptConfigMessage() override;

public:
    InstallProcessInfoView *m_textEdit = nullptr;

public:
    static AptConfigMessage *getInstance()
    {
        if (aptConfig == nullptr) {
            aptConfig = new AptConfigMessage;
        }
        return aptConfig;
    }

public:
    /**
     * @brief appendTextEdit 获取安装进程返回的信息
     * @param str 安装进程的详细信息
     */
    void appendTextEdit(QString processInfo);

    /**
     * @brief clearTexts 清除输入框和信息框中的内容
     */
    void clearTexts();

public slots:
    /**
     * @brief dealInput 处理筛选输入的内容
     */
    void dealInput();

signals:
    /**
     * @brief AptConfigInputStr 将输入的序号或者选项传输给命令
     */
    void AptConfigInputStr(QString);

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    DLineEdit *m_inputEdit = nullptr;        // 输入框
    DSuggestButton *m_pushbutton = nullptr;  // 确定输入按钮
    DLabel *m_pQuestionLabel = nullptr;      // 提示信息
    static AptConfigMessage *aptConfig;

private:
    /**
     * @brief initUI 初始化UI界面的格局
     */
    void initUI();

    /**
     * @brief initTitlebar 修改标题栏，去除标题栏的各种按钮
     */
    void initTitlebar();

    /**
     * @brief initControl 初始化各种控件
     */
    void initControl();

    /**
     * @brief initTabOrder 初始化tab切换焦点的顺序
     */
    void initTabOrder();

    /**
     * @brief initAccessibleName 初始化AccessibleName
     */
    void initAccessibleName();
};

#endif  // APTCONFIGMESSAGE_H

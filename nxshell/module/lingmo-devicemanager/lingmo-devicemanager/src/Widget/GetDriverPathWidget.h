// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GETDRIVERLSTDIALOG_H
#define GETDRIVERLSTDIALOG_H

#include <DCheckBox>
#include <DLabel>
#include <DPushButton>
#include <DWidget>

#include <QObject>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class UrlChooserEdit;
class GetDriverPathWidget : public DWidget
{
    Q_OBJECT
public:
    explicit GetDriverPathWidget(QWidget *parent = nullptr);
    ~GetDriverPathWidget() override;

public:
    /**
     * @brief path 返回用户选择的驱动文件所在目录
     * @return 返回该目录
     */
    QString path();

    /**
     * @brief includeSubdir 返回是否包含子目录中的驱动文件
     * @return
     */
    bool includeSubdir();

    /**
     * @brief updateTipLabelText 更新tipLabel
     * @param text
     */
    void updateTipLabelText(const QString &text);

    /**
     * @brief clearTipLabelText
     */
    void clearTipLabelText();

private:
    /**
     * @brief init 初始化界面
     */
    void init();

signals:
    /**
     * @brief signalNotLocalFolder
     */
    void signalNotLocalFolder(bool isLocal);
private slots:
    /**
     * @brief onUpdateTheme 更新主题
     */
    void onUpdateTheme();
private:
    UrlChooserEdit *mp_fileChooseEdit;
    DCheckBox      *mp_includeCheckBox;
    DLabel         *mp_tipLabel;
    DLabel         *mp_titleLabel;
};
#endif // GETDRIVERLSTDIALOG_H

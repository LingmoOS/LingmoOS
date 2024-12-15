// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class CustomCommandLinkButton;
class QVBoxLayout;
class QSettings;
class QShortcut;

// 首页
class HomePage : public DWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);
    ~HomePage() override;


private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

protected:
    /**
     * @brief dragEnterEvent    拖拽进入
     */
    void dragEnterEvent(QDragEnterEvent *) override;

    /**
     * @brief dragMoveEvent     拖拽移动
     */
    void dragMoveEvent(QDragMoveEvent *) override;

    /**
     * @brief dropEvent 拖拽放下
     */
    void dropEvent(QDropEvent *) override;

signals:
    /**
     * @brief signalFileChoose  选择文件信号
     */
    void signalFileChoose();

    /**
     * @brief signalDragFiles   拖拽添加文件信号
     * @param listFiles 拖拽文件
     */
    void signalDragFiles(const QStringList &listFiles);

private slots:
    /**
     * @brief slotThemeChanged  系统主题变化
     */
    void slotThemeChanged();

private:
    QVBoxLayout *m_pLayout;     // 界面布局
    QPixmap m_pixmap;           // 界面图标
    DLabel *m_pIconLbl;        // 图标
    DLabel *m_pTipLbl;        // 提示语
    DLabel *m_pSplitLbl;       // 分割线
    CustomCommandLinkButton *m_pChooseBtn;     // 选择文件按钮
    QSettings *m_pSettings;     // 配置信息
};

#endif

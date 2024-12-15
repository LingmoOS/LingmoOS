// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AITOOLBAR_H
#define AITOOLBAR_H

#include "transbutton.h"
#include "omitbutton.h"

#include <DWidget>
#include <DCommandLinkButton>
#include <DMenu>
#include <DLabel>

#include <QHBoxLayout>
#include <QAction>

namespace GrandSearch {
class AiToolBarInner;
class AiToolBar : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    /**
     * @brief 检查是否安装了UOS AI
     * @return
     */
    static bool checkUosAiInstalled();

    /**
     * @brief 显示提醒弹窗
     * @param name AI功能名
     */
    static void showWarningDialog(QString name);

    explicit AiToolBar(QWidget *parent = nullptr);
    ~AiToolBar();
    void setLeftSpace(int left) { m_left = left; m_mainLayout->setContentsMargins(m_left, m_top, m_right, 0); }
    void setRightSpace(int right) { m_right = right; m_mainLayout->setContentsMargins(m_left, m_top, m_right, 0); }
    void setTopSpace(int top) { m_top = top; m_mainLayout->setContentsMargins(m_left, m_top, m_right, 0); }
    void setFilePath(QString filePath);

private:
    void initUi();

private:
    AiToolBarInner *m_inner = nullptr;
    int m_left  = 0;
    int m_right = 0;
    int m_top   = 0;
    QHBoxLayout *m_mainLayout = nullptr;
};

class SpaceWidget;
class AiToolBarInner : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit AiToolBarInner(QWidget *parent = nullptr);
    void setFilePath(QString filePath) { m_filePath = filePath; }
    void closeMainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUi();
    void initConnect();
    void onOpenUosAi();
    void onSummary();
    void onTranslation();
    void onExtension();
    void onKnowledge();
    // AI功能按钮的布局调整，显示不完时启用省略号菜单
    void adjustBts();

    /**
     * @brief 显示提醒弹窗
     * @param name AI功能名
     */
    void showWarningDialog(QString name);

private slots:
    void onBtClicked();
    void onMenuTriggered(QAction *action);

private:
    Dtk::Widget::DCommandLinkButton *m_summaryBt     = nullptr;
    Dtk::Widget::DCommandLinkButton *m_translationBt = nullptr;
    Dtk::Widget::DCommandLinkButton *m_extensionBt   = nullptr;
    Dtk::Widget::DCommandLinkButton *m_knowledgeBt   = nullptr;
    QList<Dtk::Widget::DWidget *> m_btList;
    Dtk::Widget::DWidget *m_summarySpace             = nullptr;
    Dtk::Widget::DWidget *m_translationSpace         = nullptr;
    Dtk::Widget::DWidget *m_extensionSpace           = nullptr;
    Dtk::Widget::DWidget *m_knowledgeSpace           = nullptr;
    QList<Dtk::Widget::DWidget *> m_spaceList;
    TransButton *m_iconBt = nullptr;
    OmitButton *m_omitBt = nullptr;

    Dtk::Widget::DMenu *m_menu = nullptr;
    QAction *m_summaryAction     = nullptr;
    QAction *m_translationAction = nullptr;
    QAction *m_extensionAction   = nullptr;
    QAction *m_knowledgeAction   = nullptr;
    QList<QAction *> m_actionList;

    QString m_filePath;
};
}

#endif // AITOOLBAR_H

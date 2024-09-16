// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INFOCONTROLBUTTON_H
#define INFOCONTROLBUTTON_H

#include <DLabel>

#include <QVBoxLayout>
#include <QAbstractButton>

DWIDGET_USE_NAMESPACE
#define THEME_DARK 2   //"dark"
#define THEME_LIGHT 1  //"light"

class InfoCommandLinkButton;
class InfoControlButton : public QWidget
{
    Q_OBJECT

public:
    explicit InfoControlButton(const QString &expandTips, const QString &shrinkTips, QWidget *parent = nullptr);

    /**
     * @brief setShrinkTips 设置收缩的提示语
     * @param text  提示语
     */
    void setShrinkTips(const QString text);

    /**
     * @brief setExpandTips 设置展开的提示语
     * @param text  提示语
     */
    void setExpandTips(const QString text);

public:
    /**
     * @brief linkButton 返回当前使用的CommandLinkButton
     * @return 当前使用的CommandLinkButton
     */
    QAbstractButton *controlButton();

signals:
    /**
     * @brief expand    展开的信号
     */
    void expand();

    /**
     * @brief shrink    收缩的信号
     */
    void shrink();

protected:
    /**
     * @brief mouseReleaseEvent     重写鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *) override;

    /**
     * @brief keyPressEvent         重写按键事件 增加对回车、换行、空格的兼容
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:

    /**
     * @brief onMouseRelease        鼠标事件的具体实现
     */
    void onMouseRelease();

    /**
     * @brief themeChanged          主题变化后，图标等的变化
     */
    void themeChanged();

private:
    bool m_expand = false;      // 当前是需要扩展还是收缩的标志位
    QString m_expandTips = "";  // 展开的提示语
    QString m_shrinkTips = "";  // 收缩的提示语

    DLabel *m_arrowIcon = nullptr;         // 展开或收缩的图标
    QVBoxLayout *centralLayout = nullptr;  // 布局

    // DCommandLinkButton for Activity color
    InfoCommandLinkButton *m_tipsText = nullptr;  //
};

#endif  // INFOCONTROLBUTTON_H

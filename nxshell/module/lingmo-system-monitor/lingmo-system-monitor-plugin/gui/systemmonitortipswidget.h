// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMMONITORTIPSWIDGET_H
#define SYSTEMMONITORTIPSWIDGET_H

#include <QFrame>
class SystemMonitorTipsWidget : public QFrame
{
    Q_OBJECT
public:
    explicit SystemMonitorTipsWidget(QWidget *parent = nullptr);

    //!
    //! \brief setSystemMonitorTipsText 设置系统监视器tips标签文字信息
    //! \param strList
    //!
    void setSystemMonitorTipsText(QStringList strList);

Q_SIGNALS:
    void visibleChanged(bool visible);

protected:
    //!
    //! \brief paintEvent 重绘系统监视器插件提示框
    //! \param event
    //!
    void paintEvent(QPaintEvent *event) override;

    //!
    //! \brief event 捕捉字体改变事件
    //! \param event 捕获的事件对象
    //! \return 成功返回true,失败返回false
    //!
    bool event(QEvent *event) override;

private:
    QStringList m_textList; // 获取到的悬浮框标签文字内容列表

    int m_leftWidth = 0;    // 设置字体左侧的宽度
    int m_rightWidth = 0;   // 设置字体右侧的宽度
};

#endif // SYSTEMMONITORTIPSWIDGET_H

// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONITORPLUGINBUTTONWIDGET_H
#define MONITORPLUGINBUTTONWIDGET_H

// smo-plugin

// Qt
#include <QWidget>
#include <QIcon>

class MonitorPluginButtonWidget :public QWidget
{
    Q_OBJECT
public:
    //!
    //! \brief MonitorPluginButtonWidget 构造函数
    //! \param parent
    //!
    explicit MonitorPluginButtonWidget(QWidget *parent = nullptr);

protected:
    //!
    //! \brief paintEvent 重绘事件
    //! \param e
    //!
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    //!
    //! \brief resizeEvent 界面调整大小缩放事件
    //! \param event
    //!
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    //!
    //! \brief mouseMoveEvent 鼠标移动事件
    //! \param event
    //!
    void mouseMoveEvent(QMouseEvent *event) override;
    //!
    //! \brief mousePressEvent 鼠标按下事件
    //! \param event
    //!
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //!
    //! \brief mouseReleaseEvent 鼠标释放事件
    //! \param event
    //!
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    //!
    //! \brief leaveEvent 鼠标移出事件
    //! \param event
    //!
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    //!
private:

    //! \brief loadSvg 加载SVG图标
    //! \param iconName 图标名称
    //! \param fallbackIconName 备用图标名称，没有系统图标，就使用资源文件的图标
    //! \param localPath 图标路径
    //! \param size 图标大小
    //! \param ratio 缩放比例
    //! \return
    //!
    const QPixmap loadSvg(const QString &iconName, const QString &fallbackIconName, const QString &localPath, const int size, const qreal ratio);
    //!
    //! \brief containCursorPos 是否鼠标在界面上
    //! \return
    //!
    bool containCursorPos();

private:
    bool m_hover;   //鼠标悬浮状态
    bool m_pressed; //鼠标按下状态
};

#endif // MONITORPLUGINBUTTONWIDGET_H

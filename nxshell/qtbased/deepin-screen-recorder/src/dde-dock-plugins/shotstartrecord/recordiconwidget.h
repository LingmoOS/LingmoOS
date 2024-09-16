// SPDX-FileCopyrightText: 2021-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QTime>
#include <QIcon>
#include <DWidget>
#include <com_deepin_dde_daemon_dock.h>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE

using DBusDock = com::deepin::dde::daemon::Dock;

class RecordIconWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RecordIconWidget(QWidget *parent = nullptr);
    ~RecordIconWidget() override;
    bool enabled();

    const QString itemContextMenu();
    void invokedMenuItem(const QString &menuId);
    QString getSysShortcuts(const QString &type);
    QString getDefaultValue(const QString &type);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    const QPixmap loadSvg(const QString &fileName, const QSize &size) const;

private:
    bool m_hover = false;       // 鼠标是否悬浮
    bool m_pressed = false;     // 鼠标是否按下
    QIcon m_icon;
    QPixmap *m_blgPixmap;
    QBoxLayout *centralLayout;
};

#endif // TIMEWIDGET_H

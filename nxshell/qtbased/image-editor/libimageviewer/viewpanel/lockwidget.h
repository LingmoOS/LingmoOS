// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCKWIDGET_H
#define LOCKWIDGET_H

#include <DLabel>

#include <QLabel>
#include <QMouseEvent>

#include "widgets/themewidget.h"

class QGestureEvent;
class QPinchGesture;
class QSwipeGesture;
class QPanGesture;

DWIDGET_USE_NAMESPACE

class LockWidget : public ThemeWidget
{
    Q_OBJECT
public:
    LockWidget(const QString &darkFile, const QString &lightFile,
               QWidget *parent = nullptr);
    ~LockWidget() override;
signals:
    void nextRequested();
    void previousRequested();
    void showfullScreen();

    void sigMouseMove();
public slots:
    void setContentText(const QString &text);
    void handleGestureEvent(QGestureEvent *gesture);
protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private slots:
    void pinchTriggered(QPinchGesture *gesture);
private:
    void onThemeChanged(DGuiApplicationHelper::ColorType theme);
//    DLabel *m_thumbnailLabel{nullptr};
    QPixmap m_logo;
    QString m_picString;
    bool m_theme;
    DLabel *m_bgLabel{nullptr};
    DLabel *m_lockTips{nullptr};
    int m_startx = 0;
    int m_maxTouchPoints = 0;

    //单击时间
    qint64 m_clickTime{0};
};
#endif // LOCKWIDGET_H

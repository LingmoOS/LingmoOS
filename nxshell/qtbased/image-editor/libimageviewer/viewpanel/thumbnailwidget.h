// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>

#include "widgets/themewidget.h"

#include <DLabel>

class QGestureEvent;
class QPinchGesture;
class QSwipeGesture;
class QPanGesture;

DWIDGET_USE_NAMESPACE
typedef DLabel QLbtoDLabel;

class ThumbnailWidget : public ThemeWidget
{
    Q_OBJECT
public:
    ThumbnailWidget(const QString &darkFile, const QString
                    &lightFile, QWidget *parent = nullptr);
    ~ThumbnailWidget() override;

    enum DisplayType {
        DamageType = 0,
        CannotReadType,
        TypeCount
    };

signals:
    void mouseHoverMoved();
    void nextRequested();
    void previousRequested();

    void sigMouseMove();
    void showfullScreen();
#ifdef LITE_DIV
    void openImageInDialog();
#endif

public slots:
    void handleGestureEvent(QGestureEvent *gesture);
    void setThumbnailImageAndText(const QPixmap thumbnail, DisplayType type);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private slots:
    void pinchTriggered(QPinchGesture *gesture);
private:
    void onThemeChanged(DGuiApplicationHelper::ColorType theme);

    bool m_isDefaultThumbnail = false;
    QLbtoDLabel *m_thumbnailLabel;
    QPixmap m_logo;
#ifndef LITE_DIV
    QLabel *m_tips;
#else
    DLabel *m_tips;
#endif
    QPixmap m_defaultImage;
    QColor m_inBorderColor;
    QString m_picString;
    bool m_theme;
    bool m_usb = false;
    int m_startx = 0;
    int m_maxTouchPoints = 0;
};

#endif // THUMBNAILWIDGET_H

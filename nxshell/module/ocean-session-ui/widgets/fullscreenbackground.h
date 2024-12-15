// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FULLSCREENBACKGROUND_H
#define FULLSCREENBACKGROUND_H

#include <QWidget>
#include <QPointer>
#include <QTimer>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QVariantAnimation>

class FullscreenBackground : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool contentVisible READ contentVisible WRITE setContentVisible NOTIFY contentVisibleChanged)

public:
    explicit FullscreenBackground(QWidget *parent = nullptr);

    bool contentVisible() const;

public slots:
    void updateBackground(const QPixmap &background);
    void updateBackground(const QString &file);
    void setScreen(QScreen *screen);
    void setContentVisible(bool contentVisible);

signals:
    void contentVisibleChanged(bool contentVisible);

protected:
    void setContent(QWidget * const w);
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *event) override;

private:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    const QPixmap pixmapHandle(const QPixmap &pixmap);

private:
    void updateScreen(QScreen *screen);
    void updateGeometry();
    using QWidget::setGeometry;
    using QWidget::resize;
    using QWidget::move;

    QString m_bgPath;
    QPixmap m_background;
    QPixmap m_fakeBackground;
    QPixmap m_backgroundCache;
    QPixmap m_fakeBackgroundCache;
    QPointer<QWidget> m_content;
    QVariantAnimation *m_fadeOutAni;
    QScreen *m_screen = nullptr;
};

#endif // FULLSCREENBACKGROUND_H

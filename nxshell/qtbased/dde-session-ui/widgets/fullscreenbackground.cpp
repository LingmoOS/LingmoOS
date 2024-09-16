// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fullscreenbackground.h"

#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QPainter>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>
#include <QKeyEvent>
#include <QCryptographicHash>
#include <QWindow>

FullscreenBackground::FullscreenBackground(QWidget *parent)
    : QWidget(parent)
    , m_fadeOutAni(new QVariantAnimation(this))
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);

    m_fadeOutAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_fadeOutAni->setDuration(1000);
    m_fadeOutAni->setStartValue(1.0f);
    m_fadeOutAni->setEndValue(0.0f);

    connect(m_fadeOutAni, &QVariantAnimation::valueChanged, this, static_cast<void (FullscreenBackground::*)()>(&FullscreenBackground::update));
}

bool FullscreenBackground::contentVisible() const
{
    return m_content && m_content->isVisible();
}

void FullscreenBackground::updateBackground(const QPixmap &background)
{
    // show old background fade out
    m_fakeBackground = m_background;
    m_background = background;

    m_backgroundCache = pixmapHandle(m_background);
    m_fakeBackgroundCache = pixmapHandle(m_fakeBackground);

    m_fadeOutAni->start();
}

void FullscreenBackground::updateBackground(const QString &file)
{
    const QUrl url(file);
    if (url.isLocalFile())
        return updateBackground(url.path());

    if (m_bgPath == file)
        return;

    if (QFile::exists(file)) {
        m_bgPath = file;
    }
    else {
        m_bgPath = "/usr/share/wallpapers/deepin/desktop.jpg";
        if (!QFile::exists(m_bgPath)) {
            m_bgPath = "/usr/share/backgrounds/default_background.jpg";
        }
    }

    Q_ASSERT(QFileInfo(m_bgPath).isFile());

    updateBackground(QPixmap(m_bgPath));
}

void FullscreenBackground::setScreen(QScreen *screen)
{
    updateScreen(screen);
}

void FullscreenBackground::setContentVisible(bool contentVisible)
{
    if (this->contentVisible() == contentVisible)
        return;

    if (!m_content)
        return;

    if (!isVisible() && !contentVisible)
        return;

    m_content->setVisible(contentVisible);

    emit contentVisibleChanged(contentVisible);
}

void FullscreenBackground::setContent(QWidget * const w)
{
    Q_ASSERT(m_content.isNull());

    m_content = w;
    m_content->setParent(this);
    m_content->raise();
    m_content->move(0, 0);
}

void FullscreenBackground::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    const float current_ani_value = m_fadeOutAni->currentValue().toFloat();

    const QRect trueRect(QPoint(0, 0), QSize(size() * devicePixelRatioF()));

    if (!m_background.isNull()) {
        // tr is need redraw rect, sourceRect need correct upper left corner
        painter.drawPixmap(trueRect,
                           m_backgroundCache,
                           QRect(trueRect.topLeft(), trueRect.size() * m_backgroundCache.devicePixelRatioF()));
    }

    if (!m_fakeBackground.isNull()) {
        // draw background
        painter.setOpacity(current_ani_value);
        painter.drawPixmap(trueRect,
                           m_fakeBackgroundCache,
                           QRect(trueRect.topLeft(), trueRect.size() * m_fakeBackgroundCache.devicePixelRatioF()));
        painter.setOpacity(1);
    }
}

void FullscreenBackground::enterEvent(QEvent *event)
{
    m_content->show();
    emit contentVisibleChanged(true);

    return QWidget::enterEvent(event);
}

void FullscreenBackground::leaveEvent(QEvent *event)
{
    return QWidget::leaveEvent(event);
}

void FullscreenBackground::resizeEvent(QResizeEvent *event)
{
    m_content->resize(size());

    m_backgroundCache = pixmapHandle(m_background);
    m_fakeBackgroundCache = pixmapHandle(m_fakeBackground);

    return QWidget::resizeEvent(event);
}

void FullscreenBackground::keyPressEvent(QKeyEvent *e)
{
    QWidget::keyPressEvent(e);

    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
#endif
    default:;
    }
}

void FullscreenBackground::showEvent(QShowEvent *event)
{
    if (QWindow *w = windowHandle()) {
        if (m_screen) {
            if (w->screen() != m_screen) {
                w->setScreen(m_screen);
            }

            // 更新窗口位置和大小
            updateGeometry();
        } else {
            updateScreen(w->screen());
        }

        connect(w, &QWindow::screenChanged, this, &FullscreenBackground::updateScreen);
    }

    return QWidget::showEvent(event);
}

const QPixmap FullscreenBackground::pixmapHandle(const QPixmap &pixmap)
{
    const QSize trueSize { size() * devicePixelRatioF() };
    QPixmap pix = pixmap.scaled(trueSize,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);

    pix = pix.copy(QRect((pix.width() - trueSize.width()) / 2,
                         (pix.height() - trueSize.height()) / 2,
                         trueSize.width(),
                         trueSize.height()));

    // draw pix to widget, so pix need set pixel ratio from qwidget devicepixelratioF
    pix.setDevicePixelRatio(devicePixelRatioF());

    return pix;
}

void FullscreenBackground::updateScreen(QScreen *screen)
{
    if (screen == m_screen)
        return;

    if (m_screen) {
        disconnect(m_screen, &QScreen::geometryChanged, this, &FullscreenBackground::updateGeometry);
    }

    if (screen) {
        connect(screen, &QScreen::geometryChanged, this, &FullscreenBackground::updateGeometry);
    }

    m_screen = screen;

    if (m_screen)
        updateGeometry();
}

void FullscreenBackground::updateGeometry()
{
    setGeometry(m_screen->geometry());
}

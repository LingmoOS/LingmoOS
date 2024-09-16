// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOXFRAME_H
#define BOXFRAME_H

#include <QLabel>
#include <QPixmapCache>

class QPixmap;
class BackgroundManager;
class QScreen;

class BoxFrame : public QLabel
{
    Q_OBJECT

public:
    explicit BoxFrame(QWidget* parent = nullptr);

    void setBackground(const QString &url);
    void setBlurBackground(const QString &url);

signals:
    void backgroundImageChanged(const QPixmap & img);

protected:
    void scaledBackground();
    void scaledBlurBackground();
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;

private:
    virtual const QScreen * currentScreen();

private:
    QString m_lastUrl;
    QString m_lastBlurUrl;
    QPixmap m_pixmap;
    QString m_defaultBg;
    QPixmapCache::Key m_cacheNormalKey;
    QPixmapCache::Key m_cacheBlurKey;
    BackgroundManager *m_bgManager;
    bool m_useSolidBackground;
};

#endif // BOXFRAME_H

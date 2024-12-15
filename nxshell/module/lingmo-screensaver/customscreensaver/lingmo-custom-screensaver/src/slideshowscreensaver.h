// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDESHOWSCREENSAVER_H
#define SLIDESHOWSCREENSAVER_H

#include <QAbstractNativeEventFilter>
#include <QWidget>
#include <QMap>

class SlideshowScreenSaver : public QWidget, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit SlideshowScreenSaver(bool subWindow = false, QWidget *parent = nullptr);
    ~SlideshowScreenSaver() override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void init();
private slots:
    void onUpdateImage();

protected:
    void paintEvent(QPaintEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    QSize mapFromHandle(const QSize &handleSize);

private:
    void showDefaultBlack(QPaintEvent *event);
    void randomImageIndex();
    void loadSlideshowImages();
    void initPixMap();
    void nextValidPath();
    void filterInvalidFile(const QString &path);
    void scaledPixmap();

private:
    QScopedPointer<QPixmap> m_pixmap;
    QScopedPointer<QPixmap> m_scaledPixmap;
    QScopedPointer<QTimer> m_timer;

    QString m_path;   // 图片路径
    QString m_invaildPath;   // 无效图片路径提示
    bool m_shuffle { false };   // 随机轮播
    int m_intervalTime { 0 };   // 轮播间隔
    int m_currentIndex { 0 };
    QStringList m_imagefiles;
    bool m_subWindow { false };
    QMap<int, QString> m_playOrder;
    QString m_lastImage;
};

#endif   // SLIDESHOWSCREENSAVER_H

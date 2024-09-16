// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QFuture>
#include <QWaitCondition>
#include <QThread>

namespace dfm_wallpapersetting {

class PixmapProducer : public QThread
{
    Q_OBJECT
public:
    struct PixmapInfo
    {
        QString path;
        QSize size;
        qreal ratio;
    };
    explicit PixmapProducer(QObject *parent);
    void append(const PixmapInfo &);
    void stop();
    void launch();
protected:
    void run() override;
protected:
    volatile bool running = false;
    QList<PixmapInfo> infos;
    QWaitCondition cond;
    QMutex condMtx;
};

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();
    void updateImage();
    void setImage(const QString &img);
    void setBackground(const QColor &color);
    void setBoder(const QColor &color);
protected:
    QSize imageSize() const;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private slots:
    void setPixmap(QPixmap pix);
protected:
    QString imgPath;
    QPixmap pixmap;
    QColor bkgColor;
    QColor bdColor;
private:
    PixmapProducer *worker = nullptr;
    QSize curSize;
};

}
#endif // PREVIEWWIDGET_H

// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <DLabel>

#include <QWidget>

namespace GrandSearch {
namespace video_preview {

class ThumbnailLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailLabel(QWidget* parent = nullptr);
    void setPixmap(const QPixmap &pixmap);
private:
    QPixmap m_pixmap;
    QLabel *m_picLabel = nullptr;
};

class VideoView : public QWidget
{
    Q_OBJECT
public:
    explicit VideoView(QWidget *parent = nullptr);
    void initUI();
    void setTitle(const QString &title);
    void setThumbnail(const QPixmap &pixmap);
    static QSize maxThumbnailSize();
signals:

public slots:
protected:
    Dtk::Widget::DLabel *m_title = nullptr;
    ThumbnailLabel *m_picFrame= nullptr;
};

}}

#endif // VIDEOVIEW_H

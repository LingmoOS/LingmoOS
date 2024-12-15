// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <DWidget>
#include <QLabel>
#include <QByteArray>
#include <QMovie>
#include <QVBoxLayout>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE

namespace GrandSearch {
namespace image_preview {

class ImageView : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = nullptr);
    ~ImageView() Q_DECL_OVERRIDE;

    QSize sourceSize();
    bool stopPreview();

    void loadImage(const QString &file, const QString &type);
private:
    void initUI();
    void initConnect();

    bool canPreview();

    QPixmap getRoundPixmap(const QPixmap &pixmap);

private slots:
    void onMovieFrameChanged(int frameNumber);
    void showErrorPage();

private:
    QVBoxLayout *m_mainLayout = nullptr;

    QString m_imageFile;                // 资源文件
    QByteArray m_formats;               // 格式
    QImage m_image;                     // 图片
    QMovie *m_movie = nullptr;          // gif图片
    bool m_isMovie = false;             // 是否是gif图片

    QSize m_sourceSize;                 // 资源尺寸
    QLabel *m_imageLabel;               // 显示图片
    Dtk::Widget::DLabel *m_titleLabel;  // 显示名称
};

}}

#endif // IMAGEVIEW_H

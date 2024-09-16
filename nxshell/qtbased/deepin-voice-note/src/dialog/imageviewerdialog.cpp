// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialog/imageviewerdialog.h"

#include <DLog>
#include <DGuiApplicationHelper>
#include <QApplication>
#include <QDesktopWidget>
#include <QShortcut>
#include <QImageReader>
#include <QPainterPath>

static const int kBorderSize = 12;
static const int kCloseBtnSize = 48;
static const int kImgLabelWidth = 380; //图片控件临界宽度
static const int kImgLabelHeight = 240; //图片控件临界高度
static const int kRadius = 18; //圆角大小

ImageViewerDialog::ImageViewerDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setObjectName("ImageViewer");
    this->initUI();

    connect(m_closeButton, &Dtk::Widget::DIconButton::clicked, this, &ImageViewerDialog::close);
}

ImageViewerDialog::~ImageViewerDialog()
{
}

/**
 * @brief ImageViewer::open
 * @param filepath
 * @note 根据给定的图片文件路径，打开图片并全屏显示
 */
void ImageViewerDialog::open(const QString &filepath)
{
    QImage image;
    QImageReader reader(filepath);
    reader.setDecideFormatFromContent(true);
    if (!reader.canRead() || !reader.read(&image)) {
        return;
    }

    //获取图片显示的最大大小（显示屏大小的80%）
    const QRect screenRect = qApp->desktop()->screenGeometry(QCursor::pos());
    int maxWidth = int(screenRect.width() * 0.8);
    int maxHeight = int(screenRect.height() * 0.8);

    //当图片过大时缩放图片
    if (image.width() > maxWidth || image.height() > maxHeight) {
        //设置图片保存纵横比平滑模式自适应控件大小
        image = image.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
    }

    //设置控件占据整个屏幕大小
    this->move(screenRect.topLeft());
    this->resize(screenRect.size());
    this->showFullScreen();

    m_imgWidth = image.width();
    m_imgHeight = image.height();
    //加载图片，图片居中显示
    m_imgLabel->setPixmap(QPixmap::fromImage(image));
    m_imgLabel->setAlignment(Qt::AlignCenter);
    m_imgLabel->resize(image.width(), image.height());
    //将图片控件移至居中位置
    m_imgLabel->move(int((screenRect.width() - m_imgLabel->width()) / 2.0), int((screenRect.height() - m_imgLabel->height()) / 2.0));

    //关闭按钮移到图片右上角
    const QPoint topRightPoint = m_imgLabel->geometry().topRight();
    m_closeButton->move(topRightPoint.x() - kCloseBtnSize / 2,
                        topRightPoint.y() - kCloseBtnSize / 2);
    m_closeButton->show();
    m_closeButton->raise();
}

/**
 * @brief ImageViewer::initUI
 * 界面初始化
 */
void ImageViewerDialog::initUI()
{
    m_imgLabel = new DLabel(this);
    m_imgLabel->setObjectName("ImageLabel");
    m_imgLabel->setMinimumSize(kImgLabelWidth + kRadius / 2, kImgLabelHeight + kRadius / 2);

    //右上角关闭按钮初始化
    m_closeButton = new DDialogCloseButton(this);
    m_closeButton->setFlat(true);
    m_closeButton->setObjectName("CloseButton");
    m_closeButton->raise();
    m_closeButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_closeButton->setIconSize(QSize(36, 36));
    m_closeButton->setFixedSize(45, 45);

    this->setContentsMargins(kBorderSize, kBorderSize, kBorderSize, kBorderSize);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint
                         | Qt::Dialog | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setModal(true);
}

/**
 * @brief ImageViewer::mousePressEvent
 * @param event
 * 重写鼠标点击事件，打开图片后点击窗口任意位置，都隐藏此界面
 */
void ImageViewerDialog::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    this->hide();
}

/**
 * @brief ImageViewer::paintEvent
 * @param event
 * 画图事件，重绘整个屏幕大小
 */
void ImageViewerDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(0, 0, this->width(), this->height(), QColor(0, 0, 0, 77));

    //图片大小大于等于临界值时不绘制背景，防止出现白边
    if (m_imgWidth < kImgLabelWidth || m_imgHeight < kImgLabelHeight) {
        //获取背景色
        QColor color = DGuiApplicationHelper::instance()->applicationPalette().background().color();
        //抗齿距
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(m_imgLabel->geometry(), kRadius, kRadius);
        //设置填充背景
        painter.fillPath(path, color);
    }
}

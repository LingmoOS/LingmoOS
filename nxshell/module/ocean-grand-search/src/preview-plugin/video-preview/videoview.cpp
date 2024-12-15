// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreview_global.h"
#include "videoview.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QVBoxLayout>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>

#define CONTENT_WIDTH 310
#define CONTENT_HEIGHT 110

DWIDGET_USE_NAMESPACE
GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::video_preview;

VideoView::VideoView(QWidget *parent) : QWidget(parent)
{

}

void VideoView::initUI()
{
    // 高150
    setFixedHeight(150);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setSpacing(0);
    // 左边距35，view外有10的上边距，内部只需22-10=12的上边距
    // 右边距35,view外下边距已有15,内部为0
    lay->setContentsMargins(35, 12, 35, 0);

    //现实缩略图
    m_picFrame = new ThumbnailLabel(this);
    m_picFrame->setFixedSize(CONTENT_WIDTH, CONTENT_HEIGHT);
    lay->addWidget(m_picFrame);

    //显示文件名
    m_title = new DLabel(this);
    m_title->setFixedWidth(CONTENT_WIDTH);
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setElideMode(Qt::ElideMiddle);
    m_title->setMargin(0);

    QFont titleFont = m_title->font();
    titleFont.setWeight(QFont::Medium);
    titleFont = DFontSizeManager::instance()->get(DFontSizeManager::T5, titleFont);
    m_title->setFont(titleFont);

    QColor textColor(0, 0, 0, int(255 * 0.9));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, int(255 * 0.9));
    QPalette pa = m_title->palette();
    pa.setColor(QPalette::WindowText, textColor);
    m_title->setPalette(pa);

    lay->addWidget(m_title);
}

void VideoView::setTitle(const QString &title)
{
    if (m_title) {
        m_title->setText(title);
        QFontMetrics fontMetrics(m_title->font());
        int textWidth = fontMetrics.size(Qt::TextSingleLine, title).width();
        if (textWidth >= m_title->width())
            m_title->setToolTip(title);
        else
            m_title->setToolTip("");
    }
}

void VideoView::setThumbnail(const QPixmap &pixmap)
{
    if (m_picFrame)
        m_picFrame->setPixmap(pixmap);
}

QSize VideoView::maxThumbnailSize()
{
    return QSize(CONTENT_WIDTH, CONTENT_HEIGHT);
}

ThumbnailLabel::ThumbnailLabel(QWidget *parent) : QWidget(parent)
{
    //图片，水平居中
    m_picLabel = new QLabel(this);
    m_picLabel->setMargin(0);
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->addWidget(m_picLabel);
    lay->setMargin(0);
}

void ThumbnailLabel::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_picLabel->setFixedSize(pixmap.size());
    m_picLabel->setPixmap(pixmap);
}


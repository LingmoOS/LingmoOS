// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreview_global.h"
#include "imagepreviewplugin.h"
#include "imageview.h"
#include "global/commontools.h"

#include <QFileInfo>
#include <QDateTime>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::image_preview;

ImagePreviewPlugin::ImagePreviewPlugin(QObject *parent)
    : QObject (parent)
    , PreviewPlugin()
{

}

ImagePreviewPlugin::~ImagePreviewPlugin()
{
    if (m_imageView) {
        m_imageView->deleteLater();
        m_imageView = nullptr;
    }
}

void ImagePreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
}

bool ImagePreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    const QString type = item.value(PREVIEW_ITEMINFO_TYPE);

    if (!m_imageView)
        m_imageView = new ImageView();

    m_imageView->loadImage(path, type);

    // 尺寸
    auto dimension = m_imageView->sourceSize();
    QString dimensionStr("--");
    if (dimension.isValid())
        dimensionStr = QString("%1*%2").arg(dimension.width()).arg(dimension.height());

    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Dimensions:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(dimensionStr));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    QFileInfo fileInfo(path);

    // 类型
    auto suffix = fileInfo.suffix();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Type:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(suffix.isEmpty() ? "--" : suffix.toUpper()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 大小
    auto size = fileInfo.size();
    auto fileSize = qMakePair<QString, QString>("Size:", CommonTools::formatFileSize(size));

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Size:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(CommonTools::formatFileSize(size)));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 位置
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Location:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.absoluteFilePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 修改时间
    auto time = fileInfo.lastModified();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Time modified:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(time.toString(CommonTools::dateTimeFormat())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    m_item = item;
    return true;
}

ItemInfo ImagePreviewPlugin::item() const
{
    return m_item;
}

QWidget *ImagePreviewPlugin::contentWidget() const
{
    return m_imageView;
}

bool ImagePreviewPlugin::stopPreview()
{
    if (m_imageView)
        m_imageView->stopPreview();
    return true;
}

DetailInfoList ImagePreviewPlugin::getAttributeDetailInfo() const
{
    return m_detailInfos;
}

QWidget *ImagePreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool ImagePreviewPlugin::showToolBar() const
{
    return true;
}
